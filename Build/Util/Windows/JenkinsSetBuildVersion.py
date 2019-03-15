#!python
#
# Script to update BUILD number in RGA/Utils/Include/rgaVersionInfo.h
#
# Usage
#   export WORKSPACE=<jenkins-workspace-root>
#   export BUILD_NUMBER=<number>
#   python $WORKSPACE/RGA/Build/Utils/JenkinsSetBuildVersion.py -b <build_number>
#
import os
import argparse

# Get full path to script to support run from anywhere
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# handle command line arguments
parser = argparse.ArgumentParser('Update BUILD string in rgaVersionInfo.h')
parser.add_argument('-b', '--build', action='store', required=True, help='version BUILD_NUMBER value')
updateArgs = parser.parse_args()

# initialize file for search
rgaVersionFile = os.path.normpath(os.path.join(os.environ['WORKSPACE'], 'RGA', 'Utils', 'Include', 'rgaVersionInfo.h'))
rgaVersionData = open(rgaVersionFile, 'r')

# replace BUILD version string in version info file and write information back to file
newData = []
for line in rgaVersionData:
    if 'define RGA_VERSION_BUILD ' in line:
        newline = line.replace("0", updateArgs.build)
        newData.append(newline)
    else:
        newData.append(line)

rgaVersionData.close()
rgaVersionData = open(rgaVersionFile, 'w')
rgaVersionData.writelines(newData)
rgaVersionData.close()
