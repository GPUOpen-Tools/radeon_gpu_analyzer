#!bash
# jenkins_build_rga.sh
#  Build RGA artifacts on Linux
#
# Usage: bash ./RGA/Build/Util/Linux/jenkins_build_rga.sh
#
# set script to be verbose and to exit on any command error.
set -x
set -e

# check for optional arguments.
#   Build Release or Debug configuration.
export BUILD_TYPE=Release
#   Build GUI with test automation support.
export AUTOMATION=
export AUTOMATION_SUFFIX=
#   Support running this script twice in the same workspace.
export FETCH_DEPENDENCIES="Yes"
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --debug)
            export BUILD_TYPE=Debug
            shift # past argument
            ;;
        --automation)
            export AUTOMATION=--automation
            export AUTOMATION_SUFFIX=_Test
            shift # past argument
            ;;
        --nofetch)
            export FETCH_DEPENDENCIES="No"
            shift # past argument
            ;;
        *)
            echo "Invalid argument $key"
            exit 1
            ;;
    esac
done

# extract Vulkan RGA Layer files.
if [ $FETCH_DEPENDENCIES == "Yes" ] ; then
    tar -xf VKLayerRGA-*.tgz
    rm -f VKLayerRGA-*.tgz
fi

# set build date, and vulkan and Qt version.
export RGA_BUILD_DATE=$(date +'%m/%d/%Y')
export VULKAN_SDK_VER=1.1.97.0
export QT_VER=5.9.2

# Populate workspace without commit-msg hook.
cd RGA/Build
if [ $FETCH_DEPENDENCIES == "Yes" ] ; then
    python FetchDependencies.py --no-hooks
fi

# Initialize build.
./Prebuild.sh --no-fetch --qt /opt/Qt/Qt$QT_VER/$QT_VER/gcc_64 --build $BUILD_TYPE $AUTOMATION --vk-include /opt/VulkanSDK/$VULKAN_SDK_VER/x86_64/include --vk-lib /opt/VulkanSDK/$VULKAN_SDK_VER/x86_64/lib

# Build RGA.
cd Linux/Make${AUTOMATION_SUFFIX}/
make -j 4

# Build documentation
cd $WORKSPACE/RGA/Documentation
make -j 4 clean
make -j 4 html
