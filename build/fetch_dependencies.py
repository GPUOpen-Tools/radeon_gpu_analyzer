#! /usr/bin/python
#
# Simple script to update a set of common directories that are needed as dependencies of the current project
# Usage:
#   fetch_dependencies.py [--update]
#
# If "--update" is specified, the latest commit will be checked out.
# Otherwise, the repos will be updated to the commit specified in the "git_mapping" table.
# If the required commit in the "git_mapping" is None, the repo will be updated to the latest commit.
#

import argparse
import os
import platform
import shutil
import stat
import subprocess
import sys
import tarfile
import zipfile

# prevent generation of .pyc file
sys.dont_write_bytecode = True

# Check for the python 3.x name and import it as the 2.x name
try:
    import urllib.request as urllib
# if that failed, then try the 2.x name
except ImportError:
    import urllib

# Use "shell=True" in POpen and subprocess functions on Windows to initialize the PATH environment.
SHELLARG = False
if sys.platform.startswith("win32"):
    SHELLARG = True

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
script_root = os.path.dirname(os.path.realpath(__file__))

# also store the basename of the file
script_name = os.path.basename(__file__)

# Assume workspace root is two folders up from script_root (RGA/Build)
WORKSPACE = os.path.abspath(os.path.normpath(os.path.join(script_root, "../..")))
RGA_ROOT = os.path.abspath(os.path.normpath(os.path.join(script_root, "..")))

# Print a message to the console with appropriate pre-amble
def log_print(message):
    print ("\n" + script_name + ": " + message)
    sys.stdout.flush()

# add script root to support import of URL and git maps
sys.path.append(script_root)
from dependency_map import git_mapping
from dependency_map import github_mapping
from dependency_map import github_root
from dependency_map import url_mapping_win
from dependency_map import url_mapping_linux

# Calculate the root of the git server - all git and zip file dependencies should be retrieved from the same server.
GIT_URL = subprocess.check_output(["git", "-C", script_root, "remote", "get-url", "origin"], shell=SHELLARG)
AMD_GITHUB_URL = (str(GIT_URL).lstrip("b'"))
if GIT_URL is None:
    print("\nERROR: Unable to determine origin for RGA git project\n")
    exit(1)

# Used for development using alternate github servers.
GIT_ROOT_INTERNAL = "git@github.amd.com:Developer-Solutions/"

def parse_arguments():
    parser = argparse.ArgumentParser(description="A script that updates the build enviroment")
    parser.add_argument('-u', '--update', action='store_true', default=False,
                        help='Use latest version on the default branch')
    parser.add_argument('-f', '--force', action='store_true', default=False,
                        help='Force re-cloning a repo when the origin URL does not match the dependency_map URL.  Only useful with --update')
    return (parser.parse_args())


def get_os():
    # detect the OS
    MACHINE_OS = ""
    if "windows" in platform.system().lower():
        MACHINE_OS = "Windows"
    elif "cygwin" in platform.system().lower():
        MACHINE_OS = "Windows"
    elif "linux" in platform.system().lower():
        MACHINE_OS = "Linux"
    else:
        log_print("INFO:Operating system not recognized correctly")
        sys.exit(1)
    return MACHINE_OS


def make_executable(file_path):
    # chmod +x the Ubuntu binaries.
    cur_dir = os.getcwd()
    os.chdir(file_path)
    bin_files = subprocess.getoutput('find . -type f -print0 | xargs -0 -n 10 file -i | grep "application/x-executable"')
    if len(bin_files) > 0:
        bin_files_list = bin_files.split('\n')
        for bin_file in bin_files_list:
            file_name = bin_file.split(':')
            target_bin_file = file_name[0]
            os.chmod(target_bin_file, stat.S_IRWXU|stat.S_IRWXG|stat.S_IROTH|stat.S_IXOTH)
    os.chdir(cur_dir)


def download_url_dependencies(key, value, update=False, retry_count = 10):
    # convert target_path to OS specific format
    tmp_path = os.path.join(script_root, value)

    # clean up path, collapsing any ../ and converting / to \ for Windows
    target_path = os.path.normpath(tmp_path)

    # Create target folder if necessary.
    if not os.path.isdir(target_path):
        os.makedirs(target_path)
    zip_file_name = key.split('/')[-1].split('#')[0].split('?')[0]
    zip_path = os.path.join(target_path, zip_file_name)

    if not os.path.isfile(zip_path):
        # File doesn't exist, print message and download
        log_print("INFO: Downloading %s into %s"%(key,zip_path))
        try:
            urllib.urlretrieve(key, zip_path)
        except urllib.ContentTooShortError:
            os.remove(zip_path)
            if retry_count > 0:
                log_print("WARNING: URL content too short.  Retrying.  Retries remaining: %d"%retry_count)
                download_url_dependencies(key, value, update, retry_count - 1)
            return;
        # Unpack the downloaded file.
        if os.path.splitext(zip_path)[1] == ".zip":
            try:
                zipfile.ZipFile(zip_path).extractall(target_path)
                os.remove(zip_path)
            except (RuntimeError, ValueError):
                log_print("Unable to expand package %s.\n"%key)
                sys.exit(1)
        elif os.path.splitext(zip_path)[1] == ".gz":
            try:
                tarfile.open(zip_path).extractall(target_path)
                os.remove(zip_path)
            except (RuntimeError, ValueError):
                print("Unable to expand package %s.\n"%key)
                sys.exit(1)


def getVersion():
    rga_version_file = os.path.normpath(os.path.join(RGA_ROOT, 'source/common/rga_version_info.h'))
    rga_version_data = None
    if os.path.exists(rga_version_file):
        rga_version_data = open(rga_version_file)
    else:
        log_print("ERROR: Unable to open file: %s"%rga_version_file)
        sys.exit(1)

    # Get major, minor, and update version strings.
    major = None
    minor = None
    update = None
    for line in rga_version_data:
        if 'define RGA_VERSION_MAJOR  ' in line:
            major = (line.split()[2])
        if 'define RGA_VERSION_MINOR ' in line:
            minor = (line.split()[2])
        if 'define RGA_VERSION_UPDATE ' in line:
            update = (line.split()[2])
    if update == "0":
        return major + "." + minor
    else:
        return major + "." + minor + "." + update


def do_fetch(repo_target, repo_branch, repo_source, force):
    status = True

    # Get git repo origin URL.
    try:
        repo_url_tmp = subprocess.check_output(["git", "-C", repo_target, "remote", "get-url", "origin"], shell=SHELLARG)
        repo_url = repo_url_tmp.decode().strip()
        log_print("INFO: repo url: %s\nINFO: map url: %s"%(repo_url, repo_source))
    except subprocess.CalledProcessError as e:
        log_print("ERROR: Error trying to query for repo origin. Return code: %d\n"%e.returncode)
        status = False
    sys.stderr.flush()
    sys.stdout.flush()

    if repo_url == repo_source:
        # Current workspace cloned from a matching source.
        log_print("INFO: Directory " + repo_target + " exists. \n\tUsing 'git fetch' to get the latest revision.")
    else:
        # Current workspace cloned from an alternate source.
        log_print("WARNING: Directory %s exists, but was cloned from a different URL:"%repo_target)
        log_print("WARNING: Current origin URL: %s"%repo_url)
        log_print("WARNING: Dependency map URL: %s"%repo_source)
        log_print("WARNING: Using 'git pull' to update %s from %s"%(repo_target, repo_url))

    sys.stdout.flush()

    # Update repo_target database with fetch
    if status is True:
        try:
            subprocess.check_call(["git", "-C", repo_target, "fetch", "origin"], shell=SHELLARG)
        except subprocess.CalledProcessError as e:
            log_print ("ERROR: 'git fetch' failed with return code: %d\n"%e.returncode)
            status = False
        sys.stderr.flush()
        sys.stdout.flush()

    if status is True:
        if repo_url == repo_source:
            try:
                subprocess.check_call(["git", "-C", repo_target, "checkout", repo_branch], shell=SHELLARG)
            except subprocess.CalledProcessError as e:
                log_print ("ERROR: 'git checkout' failed with return code: %d\n"%e.returncode)
                status = False
        else:
            if not force is True:
                log_print("INFO: Using git pull to update the workspace")
                try:
                    subprocess.check_call(["git", "-C", repo_target, "pull"], shell=SHELLARG)
                except subprocess.CalledProcessError as e:
                    log_print("ERROR: 'git pull' failed with return code: %d\n"%e.returncode)
                    status = False
            else:
                # --update --force will delete the current repo_target folder and
                # reclone the repo.
                shutil.rmtree(repo_target)
                status = do_clone(repo_source, repo_target, repo_branch)

    sys.stderr.flush()
    sys.stdout.flush()

    return status


def do_clone(repo_git_path, repo_target, repo_branch):
    status = True
    log_print("INFO: Directory %s does not exist. \n\tUsing 'git clone' to get latest from %s"%(repo_target, repo_git_path))
    sys.stdout.flush()
    try:
        log_print("INFO: Cloning...%s\n"%(" ".join(["git", "clone", repo_git_path, repo_target])))
        subprocess.check_call(["git", "clone", repo_git_path, repo_target], shell=SHELLARG)
        log_print("INFO: Checking out branch...\n")
        subprocess.check_call(["git", "-C", repo_target, "checkout", repo_branch], shell=SHELLARG)
    except subprocess.CalledProcessError as e:
        log_print("ERROR: 'git clone' failed with return code: %d\n"%e.returncode)
        status = False
    sys.stderr.flush()
    sys.stdout.flush()

    return status


def fetch_git_map(update, git_branch, force):
    log_print("INFO: Fetching dependencies from: " + GIT_ROOT_INTERNAL)
    for key in git_mapping:
        # Target path, relative to workspace
        path = git_mapping[key][0]
        source = GIT_ROOT_INTERNAL + key

        required_commit = git_mapping[key][1]
        # required_commit may be "None" - or user may override commit via command line. In this case, use tip of tree
        if required_commit is None:
            required_commit = git_branch

        log_print("INFO: Checking out commit: " + required_commit + " for " + key)

        os.chdir(WORKSPACE)
        target_path = os.path.normpath(os.path.join(script_root, path))
        if os.path.isdir(target_path):
            # directory exists - get latest from git using pull
            if update is True:
                status = do_fetch(target_path, required_commit, source, force)
                if not status:
                    sys.exit(1)
        else:
            # directory doesn't exist - clone from git
            status = do_clone(source, target_path, required_commit)
            if not status:
                sys.exit(1)


def fetch_github_map(update, git_branch, force):
    log_print("INFO: Fetching dependencies from: " + github_root)
    for key in github_mapping:
        # Target path, relative to workspace
        path = github_mapping[key][0]
        source = github_root + key

        required_commit = github_mapping[key][1]

        # required_commit may be "None" - or user may override commit via command line. In this case, use tip of tree
        if required_commit is None:
            required_commit = git_branch

        log_print("INFO: Checking out commit: " + required_commit + " for " + key)

        os.chdir(WORKSPACE)
        target_path = os.path.normpath(os.path.join(script_root, path))
        if os.path.isdir(target_path):
            if update is True:
                # directory exists - get latest from git using pull
                status = do_fetch(target_path, required_commit, source, force)
                if not status:
                    sys.exit(1)
        else:
            # directory doesn't exist - clone from git
            status = do_clone(source, target_path, required_commit)
            if not status:
                sys.exit(1)


def do_fetch_dependencies(update, internal, force):
    # When running this script on Windows (and not under cygwin), we need to set the shell=True argument to Popen and similar calls
    # Without this option, Jenkins builds fail to find the correct version of git
    SHELLARG = False
    if sys.platform.startswith("win32"):
        # running on windows under default shell
        SHELLARG = True

    # Print the version of git being used. This also confirms that the script can find git
    try:
        subprocess.call(["git", "--version"], shell=SHELLARG)
    except OSError:
        # likely to be due to inability to find git command
        print("Error calling command: git --version")

    # Strip everything after the last '/' from the URL to retrieve the root
    amd_github_root = (AMD_GITHUB_URL.rsplit('/', 1))[0] + '/'

    # If cloning from github.com - use the master branch as the default branch - otherwise use amd-master
    git_branch = "amd-master"
    if "github.com" in AMD_GITHUB_URL:
        git_branch = "master"

    # The following section contains OS-specific dependencies that are downloaded and placed in the specified target directory.
    # for each dependency - test if it has already been fetched - if not, then fetch it, otherwise update it to top of tree
    # at the users request.
    fetch_git_map(update, git_branch, force)
    fetch_github_map(update, git_branch, force)

    # Capture the operating system.
    machine_os = get_os()

    # reference the correct archive path
    url_mapping = None
    if machine_os == "Linux":
        url_mapping = url_mapping_linux
    else:
        url_mapping = url_mapping_win

    # routine for downloading and unzipping an archive
    # for each archived release, download and unzip the artifacts into the target location
    for key in url_mapping:
        download_url_dependencies(key, url_mapping[key])


if __name__ == '__main__':
    # fetch_dependencies.py executed as a script
    fetch_args = parse_arguments()
    do_fetch_dependencies(fetch_args.update, False, fetch_args.force)
