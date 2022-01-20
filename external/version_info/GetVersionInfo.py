#!python
#
# get CodeXL version from Common\Src\VersionInfo\VersionInfo.h
#
# Usage:
#    python <workspace>\Common\Src\VersionInfo\GetVersionInfo.py --major|minor
#      -M,--major     Return major version number
#      -m,--minor     Return minor version number
#
import os
import argparse

# Get full path to script to support run from anywhere
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# handle command line arguments
parser = argparse.ArgumentParser(description='Get CodeXL version information')
parser.add_argument('-M', '--major', action='store_true', default=False, help='Return value of MAJOR version string')
parser.add_argument('-m', '--minor', action='store_true', default=False, help='Return value of MINOR version string')
versionArgs = parser.parse_args()

# initialize file for search
CXLVersionFile = os.path.normpath(os.path.join(scriptRoot, 'VersionInfo.h'))
CXLVersionData = file(CXLVersionFile)

# get major and minor version string
for line in CXLVersionData:
    if 'CODEXL_MAJOR_AND_MINOR_VERSION  ' in line:
        cxlVersion = (line.split()[2])
        break

# split the string on ","
major = cxlVersion.split(',')[0]
minor = cxlVersion.split(',')[1]

# print requested value
if versionArgs.major:
    print major
if versionArgs.minor:
    print minor
