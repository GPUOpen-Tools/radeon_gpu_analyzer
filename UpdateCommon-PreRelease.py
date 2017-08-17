#! /usr/bin/python
#
# Script to get the "pre-release" version of Common repos.

import os
import subprocess
import shutil
import errno
import stat

def RemoveErrorHandler(func, path, exc):
    if func in (os.rmdir, os.remove) and exc[1].errno == errno.EACCES:
        os.chmod(path, stat.S_IRWXU|stat.S_IRWXG|stat.S_IRWXO)
        func(path)
    else:
        raise

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

gitRoot = "ssh://git.amd.com:29418/DevTools/ec/"

# Define a set of dependencies that exist as separate git projects.
gitMapping = {
#   Remote repo name                                 Local path                              Branch
    "common-src-DeviceInfo"                     : ["../Common/Src/DeviceInfo",             "amd-master"]
}

# for each dependency - remove it if it exists and clone from git repo.
for key in gitMapping:
    # convert path to OS specific format
    # add script directory to path
    localPath = gitMapping[key][0]
    gitBranch = gitMapping[key][1]
    tmppath = os.path.join(scriptRoot, localPath)
    # clean up path, collapsing and ../ and converting / to \ for Windows
    path = os.path.normpath(tmppath)

    if os.path.isdir(path):
        # directory exists - remove it
        print("\n " + path + " exists, removing...")
        shutil.rmtree(path, ignore_errors=False, onerror=RemoveErrorHandler)

    # clone
    print("\n" + path + ": Using 'git clone' to get latest")
    p = subprocess.Popen(["git", "clone", "-b", gitBranch, gitRoot+key, path])
    p.wait();
