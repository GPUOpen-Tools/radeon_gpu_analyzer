#!python
#
# get RGA version from RGA\Utils\include\rgaVersionInfo.h
#
# Usage:
#    python <workspace>\RGA\Build\JenkinsGetVersion.py -major|minor
#      -major     Return major version number
#      -minor     Return minor version number
#
import os
import argparse

# Get full path to script to support run from anywhere
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# handle command line arguments
parser = argparse.ArgumentParser(description='Get RGA version information')
parser.add_argument('--major', action='store_true', default=False, help='Return value of MAJOR version string')
parser.add_argument('--minor', action='store_true', default=False, help='Return value of MINOR version string')
parser.add_argument('--update', action='store_true', default=False, help='Return the value of UPDATE version string')
versionArgs = parser.parse_args()

# initialize file for search
RGAVersionFile = os.path.normpath(os.path.join(os.environ['WORKSPACE'], 'RGA/Utils/Include/rgaVersionInfo.h'))
RGAVersionData = None
if os.path.exists(RGAVersionFile):
    RGAVersionData = open(RGAVersionFile)
else:
    print("ERROR: Unable to open file: %s"%RGAVersionFile)
    exit(1)

# get major and minor version string
major = None
minor = None
update = None
for line in RGAVersionData:
    if 'define RGA_VERSION_MAJOR  ' in line:
        major = (line.split()[2])
    if 'define RGA_VERSION_MINOR ' in line:
        minor = (line.split()[2])
    if 'define RGA_VERSION_UPDATE ' in line:
        update = (line.split()[2])

if versionArgs.major == True:
    print(major)
if versionArgs.minor == True:
    print(minor)
if versionArgs.update == True:
    print(update)
