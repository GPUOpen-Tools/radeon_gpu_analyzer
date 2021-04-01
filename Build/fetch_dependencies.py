#! /usr/bin/python
#
# Simple script to update a set of common directories that are needed as dependencies of the current project
# Usage:
#   FetchDependencies.py [latest]
#
# If "latest" is specified, the latest commit will be checked out.
# Otherwise, the repos will be updated to the commit specified in the "git_mapping" table.
# If the required commit in the "git_mapping" is None, the repo will be updated to the latest commit.

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
if ( sys.platform.startswith("win32")):
    SHELLARG = True

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
script_root = os.path.dirname(os.path.realpath(__file__))

# Assume workspace root is two folders up from script_root (RGA/Build)
workspace = os.path.abspath(os.path.normpath(os.path.join(script_root, "../..")))
rga_root = os.path.abspath(os.path.normpath(os.path.join(script_root, "..")))
rga_internal_root = os.path.abspath(os.path.normpath(os.path.join(script_root, "../../RGA-Internal")))

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
if git_url == None:
    print("Error: Unable to determine origin for RGA git project")
    exit(1)
elif "github" not in git_url_string:
    sys.path.insert(0, os.path.join(rga_internal_root, "build"))
    from artifactory_helper import ArtifactoryHelper
    from artifactory_helper import artifactory_root
    from artifactory_helper import artifactory_server

# URL to root of radeon_gpu_analyzer releases on github.com.
github_release_root = "https://github.com/GPUOpen-Tools/radeon_gpu_analyzer/releases/download/"

def parse_arguments():
    parser = argparse.ArgumentParser(description="A script that updates the build enviroment")
    parser.add_argument('--latest', action='store_true', default=False, help='Use latest version on the default branch')
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
    # convert targetPath to OS specific format
    tmppath = os.path.join(script_root, value)

    # clean up path, collapsing any ../ and converting / to \ for Windows
    targetPath = os.path.normpath(tmppath)

    # Create target folder if necessary.
    if False == os.path.isdir(targetPath):
        os.makedirs(targetPath)
    zip_file_name = key.split('/')[-1].split('#')[0].split('?')[0]
    zipPath = os.path.join(targetPath, zip_file_name)
    if False == os.path.isfile(zipPath):
        print("\nDownloading " + key + " into " + zipPath)
        if isPython3OrAbove:
            urllib.request.urlretrieve(key, zipPath)
        else:
            urllib.urlretrieve(key, zipPath)
        if os.path.splitext(zipPath)[1] == ".zip":
            zipfile.ZipFile(zipPath).extractall(targetPath)
            os.remove(zipPath)
        elif os.path.splitext(zipPath)[1] == ".gz":
            tarfile.open(zipPath).extractall(targetPath)
            os.remove(zipPath)


def getVersion():
    rga_version_file = os.path.normpath(os.path.join(rga_root, 'Utils/Include/rgaVersionInfo.h'))
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
    # path is artifactory server relative path to zip file.
    # value is <rga-relative-target-path>.
    zip_file_name = file_path.split('/')[-1].split('#')[0].split('?')[0]
    target_path = os.path.normpath(os.path.join(rga_root, value))
    artifactory_download = ArtifactoryHelper(artifactory_server)
    artifactory_path = artifactory_server + file_path
    artifactory_download.DownloadFile(artifactory_path)
    if os.path.splitext(zip_file_name)[1] == ".zip":
        zipfile.ZipFile(zip_file_name).extractall(target_path)
        if (get_os() == "Linux"):
            make_executable(target_path)
        os.remove(zip_file_name)
    else:
        os.rename(zip_file_name, os.path.join(target_path, zip_file_name))


def fetch_git_map(arguments, git_branch, git_root):
    for key in git_mapping:
        # Target path, relative to workspace
        path = git_mapping[key][0]
        source = git_root + key

        reqdCommit = git_mapping[key][1]
        # reqdCommit may be "None" - or user may override commit via command line. In this case, use tip of tree
        if( reqdCommit is None):
            reqdCommit = git_branch

        print("\nChecking out commit: " + reqdCommit + " for " + key)

        os.chdir(workspace)
        if os.path.isdir(path):
            # directory exists - get latest from git using pull
            print("Directory " + path + " exists. \n\tUsing 'git fetch' to get latest from " + source)
            sys.stdout.flush()
            try:
                subprocess.check_call(["git", "-C", path, "fetch", "origin"], shell=SHELLARG)
            except subprocess.CalledProcessError as e:
                print ("'git fetch' failed with return code: %d\n" % e.returncode)
            try:
                subprocess.check_call(["git", "-C", path, "checkout", reqdCommit], shell=SHELLARG)
            except subprocess.CalledProcessError as e:
                print ("'git checkout' failed with return code: %d\n" % e.returncode)
            sys.stderr.flush()
            sys.stdout.flush()
        else:
            # directory doesn't exist - clone from git
            print("Directory " + path + " does not exist. \n\tUsing 'git clone' to get latest from " + source)
            sys.stdout.flush()
            try:
                subprocess.check_call(["git", "clone", source, path], shell=SHELLARG)
                subprocess.check_call(["git", "-C", path, "checkout", reqdCommit], shell=SHELLARG)
            except subprocess.CalledProcessError as e:
                print ("'git clone' failed with return code: %d\n" % e.returncode)
                sys.exit(1)
            sys.stderr.flush()
            sys.stdout.flush()


def fetch_github_map(arguments, git_branch):
    for key in github_mapping:
        # Target path, relative to workspace
        path = github_mapping[key][0]
        source = github_root + key

        reqdCommit = github_mapping[key][1]

        # reqdCommit may be "None" - or user may override commit via command line. In this case, use tip of tree
        if(reqdCommit is None):
            reqdCommit = git_branch

        print("\nChecking out commit: " + reqdCommit + " for " + key)

        os.chdir(workspace)
        if os.path.isdir(path):
            # directory exists - get latest from git using pull
            print("Directory " + path + " exists. \n\tUsing 'git fetch' to get latest from " + source)
            sys.stdout.flush()
            try:
                subprocess.check_call(["git", "-C", path, "fetch", "origin"], shell=SHELLARG)
            except subprocess.CalledProcessError as e:
                print ("'git fetch' failed with return code: %d\n" % e.returncode)
            try:
                subprocess.check_call(["git", "-C", path, "checkout", reqdCommit], shell=SHELLARG)
            except subprocess.CalledProcessError as e:
                print ("'git checkout' failed with return code: %d\n" % e.returncode)
            sys.stderr.flush()
            sys.stdout.flush()
        else:
            # directory doesn't exist - clone from git
            print("Directory " + path + " does not exist. \n\tUsing 'git clone' to get latest from " + source)
            sys.stdout.flush()
            try:
                subprocess.check_call(["git", "clone", source, path], shell=SHELLARG)
                subprocess.check_call(["git", "-C", path, "checkout", reqdCommit], shell=SHELLARG)
            except subprocess.CalledProcessError as e:
                print ("'git clone' failed with return code: %d\n" % e.returncode)
                sys.exit(1)
            sys.stderr.flush()
            sys.stdout.flush()


def do_fetch_dependencies(arguments):
    # When running this script on Windows (and not under cygwin), we need to set the shell=True argument to Popen and similar calls
    # Without this option, Jenkins builds fail to find the correct version of git
    SHELLARG = False
    if (sys.platform.startswith("win32")):
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
    if "github" in git_url_string:
        git_branch = "master"
        # temporary for testing

    print("\nFetching dependencies from: " + git_root + " - using branch: " + git_branch)

    # Define a set of dependencies that exist as separate git projects. The parameters are:
    # "git repo name"  : ["Directory for clone relative to this script",  "branch or commit to checkout (or None for top of tree)"

    # The following section contains OS-specific dependencies that are downloaded and placed in the specified target directory.
    # for each dependency - test if it has already been fetched - if not, then fetch it, otherwise update it to top of tree
    fetch_git_map(arguments, git_branch, git_root)
    fetch_github_map(arguments, git_branch)

    # detect the OS
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

    # If one of the binaries exists, assume they all do.
    if os.path.isfile(os.path.normpath(os.path.join(rga_root, "Core/LC/OpenCL/win64/bin/clang.exe"))):
        print("\nBinaries already exist\n")
        return
    else:
        # Download and extract additional zip files if necessary.
        if "github" in git_url_string:
            version = getVersion()
            rga_dependencies_zip_file = github_release_root + version + "/" + "rga_dependencies.zip"
            downloadandunzip(rga_dependencies_zip_file, workspace)
            for key in zip_files:
                zip_file_path = os.path.join(workspace, key)
                target_path = os.path.normpath(os.path.join(rga_root, zip_files[key]))
                if os.path.splitext(zip_file_path)[1] == ".zip":
                    zipfile.ZipFile(zip_file_path).extractall(target_path)
                    # extractall doesn't retain execute permissions on Linux binaries.
                    if (machine_os == "Linux"):
                        make_executable(target_path)
                    os.remove(zip_file_path)
                else:
                    # Support file dxcompiler.dll.
                    target_path = os.path.join(target_path, key)
                    os.rename(zip_file_path, target_path)
        else:
            for key in zip_files:
                artifactory_path = artifactory_root + key
                artifactoryDownload(artifactory_path, zip_files[key])


if __name__ == '__main__':
    # fetch_dependencies.py executed as a script
    fetch_args = parse_arguments()
    do_fetch_dependencies(fetch_args)
