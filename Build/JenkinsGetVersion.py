#!python
#
# get RGA version from RGA\Utils\include\rgaVersionInfo.h
#
# Usage:
#    python <workspace>\RGA\Build\JenkinsGetVersion.py -major|minor
#      -major     Return major version number
#      -minor     Return minor version number
#      -revision  Return revision version number
#
import os
import argparse

# Get full path to script to support run from anywhere
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# handle command line arguments
parser = argparse.ArgumentParser(description='Get RGA version information')
parser.add_argument('--major', action='store_true', default=False, help='Return value of MAJOR version string')
parser.add_argument('--minor', action='store_true', default=False, help='Return value of MINOR version string')
parser.add_argument('--revision', action='store_true', default=False, help='Return value of REVISION version string')
versionArgs = parser.parse_args()

# initialize file for search
RGAVersionFile = os.path.normpath(os.path.join(os.environ['WORKSPACE'], 'RGA/Utils/include/rgaVersionInfo.h'))
RGAVersionData = file(RGAVersionFile)

# get major and minor version string
for line in RGAVersionData:
    if 'STR_RGA_VERSION = ' in line:
        rgaVersionString = (line.split()[5])
        break

# strip the quotes and ";"
tmpVersionString = rgaVersionString.strip(';')
rgaVersion=tmpVersionString.strip('"')

major = rgaVersion.split('.')[0]
minor = rgaVersion.split('.')[1]
revision = rgaVersion.split('.')[2]

# print requested value
if versionArgs.major:
    print major
if versionArgs.minor:
    print minor
if versionArgs.revision:
    print revision
