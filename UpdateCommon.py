#! /usr/bin/python
#
# Simple script to update a set of common directories that are needed as dependencies of the current project

import os
import subprocess

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# To allow for future updates where we may have cloned the project somewhere other than github, store the root of the repo
# in a variable. In future, we can automatically calculate this based on the git config
gitRoot = "https://github.com/GPUOpen-Tools/"

# Define a set of dependencies that exist as separate git projects.
gitMapping = {
 # Lib.
	"common-lib-amd-ACL.git"              : "../Common/Lib/AMD/ACL",
	"common-lib-amd-ADL.git"              : "../Common/Lib/AMD/ADL",
	"common-lib-amd-APPSDK-3.0.git"       : "../Common/Lib/AMD/APPSDK",
	"common-lib-AMD-CAL-8.95.git"         : "../Common/Lib/AMD/CAL",
	"common-lib-amd-ags-4.0.0.git"	      : "../Common/Lib/AMD/ags",
	"common-lib-ext-Boost-1.59.git"       : "../Common/Lib/Ext/Boost",
	"common-lib-ext-tinyxml-2.6.2.git"    : "../Common/Lib/Ext/tinyxml",
	"common-lib-ext-utf8cpp.git"          : "../Common/Lib/Ext/utf8cpp",
    "common-lib-ext-WindowsKits.git"      : "../Common/Lib/Ext/Windows-Kits",
	"common-lib-ext-zlib-1.2.8.git"	      : "../Common/Lib/Ext/zlib",
# Src.
	"common-src-ACLModuleManager.git"     : "../Common/Src/ACLModuleManager",
    "common-src-ADLUtil.git"              : "../Common/Src/ADLUtil",
    "common-src-AMDTBaseTools.git"        : "../Common/Src/AMDTBaseTools",
	"common-src-AMDTOSAPIWrappers.git"    : "../Common/Src/AMDTOSAPIWrappers",
	"common-src-AMDTOSWrappers.git"       : "../Common/Src/AMDTOSWrappers",
    "common-src-AMDTMutex.git"            : "../Common/Src/AMDTMutex",
	"common-src-CElf.git"       		  : "../Common/Src/CElf",
	"common-src-DeviceInfo.git"           : "../Common/Src/DeviceInfo",
    "common-src-DynamicLibraryModule.git" : "../Common/Src/DynamicLibraryModule",
    "common-src-SCons.git"      		  : "../Common/Src/SCons",
	"common-src-ShaderUtils.git"      	  : "../Common/Src/ShaderUtils",
	"common-src-SCons.git"      		  : "../Common/Src/SCons",
    "common-src-TSingleton.git"           : "../Common/Src/TSingleton",
	"common-src-VersionInfo.git"	      : "../Common/Src/VersionInfo",
    "common-src-Vsprops.git"              : "../Common/Src/Vsprops",
	"common-src-Miniz.git"                : "../Common/Src/Miniz",
	"common-src-Misc.git"                 : "../Common/Src/Misc",
}

# for each dependency - test if it has already been fetched - if not, then fetch it, otherwise update it to top of tree
for key in gitMapping:
    # convert path to OS specific format
    # add script directory to path
    tmppath = os.path.join(scriptRoot, gitMapping[key])  
    # clean up path, collapsing and ../ and converting / to \ for Windows
    path = os.path.normpath(tmppath)                    

    if os.path.isdir(path):
        # directory exists - get latest from git
        print("\nDirectory " + path + " exists, using 'git pull' to get latest")
        p = subprocess.Popen(["git","pull"], cwd=path)
        p.wait();
    else:
        # directory doesn't exist - clone from git
        print("\nDirectory " + path + " does not exist, using 'git clone' to get latest")
        p = subprocess.Popen(["git","clone",gitRoot+key,path])
        p.wait();
        
