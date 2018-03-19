#! /usr/bin/python
#
# Simple script to update a set of common directories that are needed as dependencies of the current project
# Usage:
#   FetchDependencies.py [latest]
#
# If "latest" is specified, the latest commit will be checked out.
# Otherwise, the repos will be updated to the commit specified in the "gitMapping" table.
# If the required commit in the "gitMapping" is None, the repo will be updated to the latest commit.

import os
import subprocess
import sys

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# To allow for future updates where we may have cloned the project somewhere other than github, store the root of the repo
# in a variable. In future, we can automatically calculate this based on the git config
gitRoot = "https://github.com/GPUOpen-Tools/"

# Define a set of dependencies that exist as separate git projects.
gitMapping = {
 # Lib.
    "common-lib-AMD-ACL.git"              : ["../../Common/Lib/AMD/ACL",           None], 
    "common-lib-amd-ADL.git"              : ["../../Common/Lib/AMD/ADL",           None],
    "common-lib-amd-APPSDK-3.0.git"       : ["../../Common/Lib/AMD/APPSDK",        None],
    "common-lib-AMD-CAL-8.95.git"         : ["../../Common/Lib/AMD/CAL",           None],
    "common-lib-amd-ags-4.0.0.git"        : ["../../Common/Lib/AMD/ags",           None],
    "common-lib-ext-Boost-1.59.git"       : ["../../Common/Lib/Ext/Boost",         None],
    "common-lib-ext-tinyxml-2.6.2.git"    : ["../../Common/Lib/Ext/tinyxml",       None],
    "common-lib-ext-tinyxml2-5.0.1"       : ["../../Common/Lib/Ext/tinyxml2",      None],
    "common-lib-ext-utf8cpp.git"          : ["../../Common/Lib/Ext/utf8cpp",       None],
    "common-lib-ext-WindowsKits.git"      : ["../../Common/Lib/Ext/Windows-Kits",  None],
    "common-lib-ext-zlib-1.2.8.git"       : ["../../Common/Lib/Ext/zlib",          None],
    "common-lib-ext-yaml-cpp"             : ["../../Common/Lib/Ext/yaml-cpp",      None],
 # Src.
    "common-src-ACLModuleManager.git"     : ["../../Common/Src/ACLModuleManager",  None],
    "common-src-ADLUtil.git"              : ["../../Common/Src/ADLUtil",           None],
    "common-src-AMDTBaseTools.git"        : ["../../Common/Src/AMDTBaseTools",     None],
    "common-src-AMDTOSWrappers.git"       : ["../../Common/Src/AMDTOSWrappers",    None],
    "common-src-AMDTMutex.git"            : ["../../Common/Src/AMDTMutex",         None],
    "common-src-CElf.git"                 : ["../../Common/Src/CElf",              None],
    "common-src-DeviceInfo.git"           : ["../../Common/Src/DeviceInfo",        None],
    "common-src-DynamicLibraryModule.git" : ["../../Common/Src/DynamicLibraryModule", None],
    "common-src-ShaderUtils.git"          : ["../../Common/Src/ShaderUtils",       None],
    "common-src-TSingleton.git"           : ["../../Common/Src/TSingleton",        None],
    "common-src-VersionInfo.git"          : ["../../Common/Src/VersionInfo",       None],
    "common-src-Vsprops.git"              : ["../../Common/Src/Vsprops",           None],
    "common-src-Miniz.git"                : ["../../Common/Src/Miniz",             None],
    "common-src-Misc.git"                 : ["../../Common/Src/Misc",              None],
    "common-dk-Installer"                 : ["../../Common/DK/Installer",          None],
 # QtCommon
    "QtCommon"                            : ["../../QtCommon",                     None]
}

# for each dependency - test if it has already been fetched - if not, then fetch it, otherwise update it to top of tree

SHELLTYPE = os.environ.get('SHELL')
# SHELL only set for Cygwin or Linux, use "shell=False" in Popen calls
SHELLARG = False
if ( SHELLTYPE == None ):
    # on Windows, set "shell=True"
    SHELLARG = True
gitCmd = ["git", "--version"]
sys.stdout.flush()
sys.stderr.flush()
gitOutput = subprocess.check_output(gitCmd, shell=SHELLARG)
sys.stdout.flush()
sys.stderr.flush()
print("%s"%gitOutput)

for key in gitMapping:
    # convert path to OS specific format
    # add script directory to path
    tmppath = os.path.join(scriptRoot, gitMapping[key][0])
    # clean up path, collapsing any ../ and converting / to \ for Windows
    path = os.path.normpath(tmppath)
    source = gitRoot + key
    # required commit (may be "None")
    reqdCommit = gitMapping[key][1]

    if os.path.isdir(path):
        # directory exists - get latest from git
        print("\nDirectory %s exists, using 'git pull' to get latest from %s"%(path, source))
        sys.stdout.flush()
        sys.stderr.flush()
        p = subprocess.Popen((["git","pull"]), cwd=path, shell=SHELLARG)
        p.wait()
        sys.stdout.flush()
        sys.stderr.flush()
        if(p.returncode != 0):
            print("git pull failed with %d"%p.returncode)
        else:
            # Checkout to required commit if it's not None
            if((len(sys.argv) == 1 or sys.argv[1] is not "latest") and reqdCommit is not None):
                print("\nChecking out required commit: %s"%reqdCommit)
                sys.stdout.flush()
                sys.stderr.flush()
                p = subprocess.Popen((["git","checkout", reqdCommit]), cwd=path, shell=SHELLARG)
                p.wait()
                sys.stdout.flush()
                sys.stderr.flush()
                if(p.returncode != 0):
                    print("git checkput failed with %d"%p.returncode)
    else:
        # directory doesn't exist - clone from git
        print("\nDirectory %s does not exist, using 'git clone' to get latest from %s"%(path, source))
        sys.stdout.flush()
        sys.stderr.flush()
        p = subprocess.Popen((["git","clone",source,path]), shell=SHELLARG)
        p.wait()
        sys.stdout.flush()
        sys.stderr.flush()
        if(p.returncode != 0):
            print("git clone failed with %d"%p.returncode)
        else:
            # Checkout to required commit if it's not None
            if((len(sys.argv) == 1 or sys.argv[1] is not "latest") and reqdCommit is not None):
                print("\nChecking out required commit: %s"%reqdCommit)
                sys.stdout.flush()
                sys.stderr.flush()
                p = subprocess.Popen((["git","checkout", reqdCommit]), cwd=path, shell=SHELLARG)
                p.wait()
                sys.stdout.flush()
                sys.stderr.flush()
                if(p.returncode != 0):
                    print("git checkput failed with %d"%p.returncode)
