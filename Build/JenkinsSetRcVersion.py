#!python
#
# Script to update RadeonGPUAnalyzerGUI.rc with appropriate version values
#    MAJOR and MINOR gotten using JenkinsGetVersion.py
#    BUILD value using BUILD_NUMBER environment variable
#    REVISION is not changed.
#
# Usage
#   export WORKSPACE=<jenkins-workspace-root>
#   export BUILD_NUMBER=<number>
#   python $WORKSPACE/RGA/Utils/JenkinsUpdateRcVersion.py -M <major> -m <minor> -b <build_number> [-r <revision>]
#
import os
import argparse

# Get full path to script to support run from anywhere
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# handle command line arguments
parser = argparse.ArgumentParser('Update version string in RadeonGPUAnalyzerGUI.rc')
parser.add_argument('-M', '--major', action='store', required=True, help='version MAJOR value')
parser.add_argument('-m', '--minor', action='store', required=True, help='version MINOR value')
parser.add_argument('-b', '--build', action='store', required=True, help='version BUILD_NUMBER value')
parser.add_argument('-r', '--revision', action='store', default="0", help='version REVISION value')
updateArgs = parser.parse_args()

# initialize file for search
rgaVersionFile = os.path.normpath(os.path.join(os.environ['WORKSPACE'], 'RGA', 'Utils', 'res', 'RadeonGPUAnalyzerGUI.rc'))
rgaVersionData = open(rgaVersionFile, 'r')

# replace version string in data and write back out to file
newData = []
for line in rgaVersionData:
    if 'define RADEON_GPU_ANALYZER_MAJOR_VERSION ' in line:
        newline = line.replace("0", updateArgs.major)
        newData.append(newline)
    else:
        if 'define RADEON_GPU_ANALYZER_MINOR_VERSION ' in line:
          newline = line.replace("0", updateArgs.minor)
          newData.append(newline)
        else:
            if 'define RADEON_GPU_ANALYZER_BUILD_NUMBER ' in line:
                newline = line.replace("0", updateArgs.build)
                newData.append(newline)
            else:
                if 'define RADEON_GPU_ANALYZER_REVISION_NUMBER ' in line:
                    newline = line.replace("0", updateArgs.revision)
                    newData.append(newline)
                else:
                    newData.append(line)

rgaVersionData.close()
rgaVersionData = open(rgaVersionFile, 'w')
rgaVersionData.writelines(newData)
rgaVersionData.close()
