#!python
#
# Copyright (c) 2020-2025 Advanced Micro Devices, Inc. All rights reserved.
#
# Get RGA version from RGA/source/common/rga_version_info.h.
#
# Usage:
#    python RGA/Build/Util/get_version.py [--major] [--minor] [--update] [--versionfile <file_path>]
#      --major     Return major version number
#      --minor     Return minor version number
#      --update    Return update version number
#      --versionfile Use <file_path> as the full path name of for rga_version_info.h
#
import os
import argparse
import sys

# prevent generation of .pyc file
sys.dont_write_bytecode = True

# Get full path to script to support run from anywhere.
SCRIPTROOT = os.path.dirname(os.path.realpath(__file__))

# Handle command line arguments.
PARSER = argparse.ArgumentParser(description='Get RGA version information')
PARSER.add_argument('--major', action='store_true', default=False, help='Return value of MAJOR version string')
PARSER.add_argument('--minor', action='store_true', default=False, help='Return value of MINOR version string')
PARSER.add_argument('--update', action='store_true', default=False, help='Return the value of UPDATE version string')
PARSER.add_argument('--versionfile', action='store', default=None, help='Use alternate path for file path')
VERSIONARGS = PARSER.parse_args()

# Initialize file for search.
RGAVERSIONFILE = os.path.normpath(os.path.join(SCRIPTROOT, '../..', 'source/common/rga_version_info.h'))
RGAVERSIONDATA = None
if not VERSIONARGS.versionfile == None:
    RGAVERSIONFILE = os.path.normpath(VERSIONARGS.versionfile)
if os.path.exists(RGAVERSIONFILE):
    RGAVERSIONDATA = open(RGAVERSIONFILE)
else:
    print("ERROR: Unable to open file: %s"%RGAVERSIONFILE)
    sys.exit(1)

# Get major, minor, and update version strings.
MAJOR = None
MINOR = None
UPDATE = None
for line in RGAVERSIONDATA:
    if 'define RGA_VERSION_MAJOR  ' in line:
        MAJOR = (line.split()[2])
    if 'define RGA_VERSION_MINOR ' in line:
        MINOR = (line.split()[2])
    if 'define RGA_VERSION_UPDATE ' in line:
        UPDATE = (line.split()[2])

if VERSIONARGS.major == True:
    print(MAJOR)
if VERSIONARGS.minor == True:
    print(MINOR)
if VERSIONARGS.update == True:
    print(UPDATE)
