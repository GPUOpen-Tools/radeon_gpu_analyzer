#! /usr/bin/python
#
# Simple script to update a set of common directories that are needed as dependencies of the current project
# Usage:
#   fetch_dependencies.py [--latest] [--binary-path <path-to-rga_dependencies-zip-file>]
#
# If "--latest" is specified, the latest commit will be checked out.
# Otherwise, the repos will be updated to the commit specified in the "git_mapping" table.
# If the required commit in the "git_mapping" is None, the repo will be updated to the latest commit.
#
# If "--binary-path" is specified, the script will use the "rga_dependencies_<version>.zip" file
# specified by the argument, for example "/home/someuser/Downloads/rga_dependencies_2.5.2.zip",
# instead of downloading rga_dependencies_<version>.zip from Artifactory.
#

import argparse
import os
import platform
import stat
import subprocess
import sys
import tarfile
import urllib
import zipfile

isPython3OrAbove = None
if sys.version_info[0] >= 3:
    isPython3OrAbove = True

if isPython3OrAbove:
    import urllib.request

SHELLARG = False
if sys.platform.startswith("win32"):
    SHELLARG = True

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
script_root = os.path.dirname(os.path.realpath(__file__))

# Assume workspace root is two folders up from script_root (RGA/Build)
workspace = os.path.abspath(os.path.normpath(os.path.join(script_root, "../..")))
rga_root = os.path.abspath(os.path.normpath(os.path.join(script_root, "..")))

# add path to project build directory to python dependency path
sys.path.insert(0, script_root)
from dependency_map import git_mapping
from dependency_map import github_mapping
from dependency_map import github_root
from dependency_map import download_mapping_windows
from dependency_map import download_mapping_linux
from dependency_map import zip_files

# Calculate the root of the git server - all git and zip file dependencies should be retrieved from the same server.
git_url = subprocess.check_output(["git", "-C", script_root, "remote", "get-url", "origin"], shell=SHELLARG)
git_url_string = (str(git_url).lstrip("b'"))
if git_url is None:
    print("Error: Unable to determine origin for RGA git project")
    exit(1)

# URL to root of radeon_gpu_analyzer releases on github.com.
github_release_root = "https://github.com/GPUOpen-Tools/radeon_gpu_analyzer/releases/download/"


def parse_arguments():
    parser = argparse.ArgumentParser(description="A script that updates the build enviroment")
    parser.add_argument('--latest', action='store_true', default=False, help='Use latest version on the default branch')
    parser.add_argument('--binary-path', action='store', default=None, help='Use rga_dependencies zip file at specified path')
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
        print("Operating system not recognized correctly")
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


def downloadandunzip(key, value):
    # convert target_path to OS specific format
    tmppath = os.path.join(script_root, value)

    # clean up path, collapsing any ../ and converting / to \ for Windows
    target_path = os.path.normpath(tmppath)

    # Create target folder if necessary.
    if not os.path.isdir(target_path):
        os.makedirs(target_path)
    zip_file_name = key.split('/')[-1].split('#')[0].split('?')[0]
    zip_path = os.path.join(target_path, zip_file_name)
    if not os.path.isfile(zip_path):
        print("\nDownloading " + key + " into " + zip_path)
        if isPython3OrAbove:
            urllib.request.urlretrieve(key, zip_path)
        else:
            urllib.urlretrieve(key, zip_path)
        if os.path.splitext(zip_path)[1] == ".zip":
            try:
                zipfile.ZipFile(zip_path).extractall(target_path)
                os.remove(zip_path)
            except (RuntimeError, ValueError):
                print("Unable to expand package %s.\n"%key)
                sys.exit(1)
        elif os.path.splitext(zip_path)[1] == ".gz":
            try:
                tarfile.open(zip_path).extractall(target_path)
                os.remove(zip_path)
            except (RuntimeError, ValueError):
                print("Unable to expand package %s.\n"%key)
                sys.exit(1)


def getVersion():
    rga_version_file = os.path.normpath(os.path.join(rga_root, 'source/common/rga_version_info.h'))
    rga_version_data = None
    if os.path.exists(rga_version_file):
        rga_version_data = open(rga_version_file)
    else:
        print("ERROR: Unable to open file: %s"%rga_version_file)
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


def artifactoryDownload(file_path, value):
    from artifactory_helper import ArtifactoryHelper
    from artifactory_helper import artifactory_server
    # path is artifactory server relative path to zip file.
    # value is <rga-relative-target-path>.
    zip_file_name = file_path.split('/')[-1].split('#')[0].split('?')[0]
    target_path = os.path.normpath(os.path.join(rga_root, value))
    print("Downloading %s into %s"%(zip_file_name, target_path))
    artifactory_download = ArtifactoryHelper(artifactory_server)
    artifactory_path = artifactory_server + file_path
    artifactory_download.DownloadFile(artifactory_path)
    try:
        zipfile.ZipFile(zip_file_name).extractall(target_path)
    except (RuntimeError, ValueError):
        print("Unable to expand package %s.\n"%file_path)
        sys.exit(1)
    if (get_os() == "Linux"):
        make_executable(target_path)
    os.remove(zip_file_name)


def do_fetch(repo_target, repo_branch):
    status = True

    print("Directory " + repo_target + " exists. \n\tUsing 'git fetch' to get the latest revision.")
    sys.stdout.flush()
    try:
        subprocess.check_call(["git", "-C", repo_target, "fetch", "origin"], shell=SHELLARG)
    except subprocess.CalledProcessError as e:
        print ("ERROR: 'git fetch' failed with return code: %d\n"%e.returncode)
        status = False

    if status is True:
        try:
            subprocess.check_call(["git", "-C", repo_target, "checkout", repo_branch], shell=SHELLARG)
        except subprocess.CalledProcessError as e:
            print ("ERROR: 'git checkout' failed with return code: %d\n"%e.returncode)
            status = False
        sys.stderr.flush()
        sys.stdout.flush()

    return status


def do_clone(repo_git_path, repo_target, repo_branch):
    status = True

    print("Directory %s does not exist. \n\tUsing 'git clone' to get latest from %s"%(repo_target, repo_git_path))
    sys.stdout.flush()
    try:
        subprocess.check_call(["git", "clone", repo_git_path, repo_target], shell=SHELLARG)
        subprocess.check_call(["git", "-C", repo_target, "checkout", repo_branch], shell=SHELLARG)
    except subprocess.CalledProcessError as e:
        print("ERROR: 'git clone' failed with return code: %d\n"%e.returncode)
        status = False
    sys.stderr.flush()
    sys.stdout.flush()

    return status


def fetch_private_map(git_server, repo_target):
    # Clone repositories in git_private_mapping if they don't exist.
    from dependency_map import git_private_mapping

    # Determine checked out branch of RGA.
    git_output = subprocess.check_output(["git", "-C", script_root, "branch", "--show-current"], shell=SHELLARG)
    git_branch = git_output.decode()
    rga_branch = git_branch.rstrip()

    os.chdir(workspace)
    for key in git_private_mapping:
        if git_private_mapping[key][1] is None:
            git_private_mapping[key][1] = rga_branch
        private_git_path = git_server + key
        private_repo_target = os.path.normpath(os.path.join(repo_target, git_private_mapping[key][0]))

        print("\nChecking out commit: " + git_private_mapping[key][1] + " for " + key)
        if os.path.isdir(private_repo_target):
            # directory exists - get latest from git using pull
            status = do_fetch(private_repo_target, git_private_mapping[key][1])
            if not status:
                sys.exit(1)
        else:
            # directory doesn't exist - clone from git
            status = do_clone(private_git_path, private_repo_target, git_private_mapping[key][1])
            if not status:
                sys.exit(1)


def fetch_git_map(arguments, git_branch, git_root):
    for key in git_mapping:
        # Target path, relative to workspace
        path = git_mapping[key][0]
        source = git_root + key

        required_commit = git_mapping[key][1]
        # required_commit may be "None" - or user may override commit via command line. In this case, use tip of tree
        if required_commit is None:
            required_commit = git_branch

        print("\nChecking out commit: " + required_commit + " for " + key)

        os.chdir(workspace)
        if os.path.isdir(path):
            # directory exists - get latest from git using pull
            status = do_fetch(os.path.normpath(os.path.join(workspace, path)), required_commit)
            if not status:
                sys.exit(1)
        else:
            # directory doesn't exist - clone from git
            status = do_clone(source, os.path.normpath(os.path.join(workspace, path)), required_commit)
            if not status:
                sys.exit(1)


def fetch_github_map(arguments, git_branch):
    for key in github_mapping:
        # Target path, relative to workspace
        path = github_mapping[key][0]
        source = github_root + key

        required_commit = github_mapping[key][1]

        # required_commit may be "None" - or user may override commit via command line. In this case, use tip of tree
        if required_commit is None:
            required_commit = git_branch

        print("\nChecking out commit: " + required_commit + " for " + key)

        os.chdir(workspace)
        if os.path.isdir(path):
            # directory exists - get latest from git using pull
            status = do_fetch(os.path.normpath(os.path.join(workspace, path)), required_commit)
            if not status:
                sys.exit(1)
        else:
            # directory doesn't exist - clone from git
            status = do_clone(source, os.path.normpath(os.path.join(workspace, path)), required_commit)
            if not status:
                sys.exit(1)


def do_fetch_dependencies(arguments):
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
    git_root = (git_url_string.rsplit('/', 1))[0] + '/'

    # If cloning from github - use the master branch as the default branch - otherwise use amd-master
    git_branch = "amd-master"
    if "github.com" in git_url_string:
        git_branch = "master"

    print("\nFetching dependencies from: " + git_root + " - using branch: " + git_branch)

    # The following section contains OS-specific dependencies that are downloaded and placed in the specified target directory.
    # for each dependency - test if it has already been fetched - if not, then fetch it, otherwise update it to top of tree
    fetch_git_map(arguments, git_branch, git_root)
    fetch_github_map(arguments, git_branch)

    # Handle git_private_mapping.
    if "github.com" not in git_url_string:
        if "JENKINS_URL" not in os.environ:
            fetch_private_map(git_root, workspace)

    # Capture the operating system.
    machine_os = get_os()

    # reference the correct archive path
    download_mapping = None
    if machine_os == "Linux":
        download_mapping = download_mapping_linux
    else:
        download_mapping = download_mapping_windows

    # routine for downloading and unzipping an archive
    # for each archived release, download and unzip the artifacts into the target location
    for key in download_mapping:
        downloadandunzip(key, download_mapping[key])

    # If one of the large binaries exists, assume they all do.
    if os.path.isfile(os.path.normpath(os.path.join(rga_root, "external/lc/opencl/windows/bin/clang.exe"))):
        print("\nBinaries already exist, skipping rga_dependencies.zip download\n")
        return
    else:
        # Download and extract additional zip files if necessary.
        version = getVersion()
        dependency_zip_file_name = "rga_dependencies_" + version + ".zip"
        if arguments.binary_path is None:
            # Download dependency zip file and unzip.
            if "github" in git_url_string:
                # Download from github.
                rga_dependencies_zip_file = github_release_root + version + "/" + dependency_zip_file_name
                downloadandunzip(rga_dependencies_zip_file, workspace)
            else:
                # Download from Artifactory.
                # Import the artifactory_helper.py from RGA-Internal.
                internal_repo_root = os.path.abspath(os.path.normpath(os.path.join(script_root, "../..")))
                sys.path.insert(0, os.path.normpath(os.path.join(internal_repo_root, "RGA-Internal/build")))
                from artifactory_helper import artifactory_root
                os.chdir(workspace)

                # Download and unzip dependency file from Artifactory.
                artifactory_path = artifactory_root + version +"/" + dependency_zip_file_name
                artifactoryDownload(artifactory_path, workspace)
        else:
            zipfile.ZipFile(arguments.binary_path).extractall(workspace)

        # Copy extracted files to RGA folders.
        for key in zip_files:
            zip_file_path = os.path.join(workspace, key)
            target_path = os.path.normpath(os.path.join(rga_root, zip_files[key]))
            print("Extracting %s files into %s"%(key, target_path))
            zipfile.ZipFile(zip_file_path).extractall(target_path)
            # extractall doesn't retain execute permissions on Linux binaries.
            if machine_os == "Linux":
                make_executable(target_path)
            os.remove(zip_file_path)


if __name__ == '__main__':
    # fetch_dependencies.py executed as a script
    fetch_args = parse_arguments()
    do_fetch_dependencies(fetch_args)
