#!/bin/bash

# Print help message
if [[ "$1" == "-h" ]] || [[ "$1" == "-help" ]] || [[ "$1" == "--h" ]] || [[ "$1" == "--help" ]]; then
    echo ""
    echo "This script generates Makefiles for RGA on Linux."
    echo ""
    echo "Usage:  prebuild.sh [options]"
    echo ""
    echo "Options:"
    echo "   --no-fetch           Do not call fetch_dependencies.py script before running cmake. The default is \"false\"."
    echo "   --cmake              Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.\n"
    echo "   --build              The build type: \"release\" or \"debug\". The default is \"debug\"."
    echo "   --qt                 Path to Qt5 root folder. The default is empty (cmake will look for Qt5 package istalled on the system)."
    echo "   --cli-only           Build RGA command-line tool only (do not build RGA GUI). The default is \"false\"."
    echo "   --gui-only           Build GUI only (do not build RGA command-line tool). The default is \"false\"."
    echo "   --no-vulkan          Build RGA without support for live Vulkan mode. If this option is used, Vulkan SDK is not required. The default is \"false\"."
    echo "   --vulkan-sdk         Path to the Vulkan SDK. The Vulkan SDK is required to build the RGA vulkan backend."
    echo "   --vk-include         Path to the Vulkan SDK include folder."
    echo "   --vk-lib             Path to the Vulkan SDK library folder."
    echo "   --cppcheck           Add CMAKE_CXX_CPPCHECK preprocess option to cmake."
    echo "   --clean              Delete cmake files and build output folders."
    echo ""
    echo "Examples:"
    echo "   prebuild.sh"
    echo "   prebuild.sh --build release"
    echo "   prebuild.sh --qt C:\Qt\5.7\msvc2015_64"
    echo "   prebuild.sh --build release --cli-only"
    echo "   prebuild.sh --no-vulkan"
    echo ""
    exit 0
fi

# Defaults
CMAKE_PATH="cmake"
BUILD_TYPE="Release"
CMAKE_QT=
QT_ROOT=
VULKAN_INC_DIR=
VULKAN_LIB_DIR=
CMAKE_VK_INC=
CMAKE_VK_LIB=
CLI_ONLY=
GUI_ONLY=
NO_VULKAN=
TEST_DIR_SUFFIX=""
CMAKE_CPPCHECK=

CURRENT_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")
OUTPUT_LINUX_DIR="$SCRIPT_DIR/linux"

# Parse command line arguments
args=("$@")
for ((i=0; i<$#; i++)); do
    arg=${args[i]}
    if [ "$arg" == "--build" ]; then
        ((i++))
        BUILD_TYPE=${args[i]}
        if [ "$BUILD_TYPE" == "release" ]; then
            BUILD_TYPE="Release"
        fi
        if [ "$BUILD_TYPE" == "debug" ]; then
            BUILD_TYPE="Debug"
        fi
    elif [ "$arg" == "--cmake" ]; then
        ((i++))
        CMAKE_PATH=${args[i]}
    elif [ "$arg" == "--qt" ]; then
        ((i++))
        QT_ROOT=${args[i]}
    elif [ "$arg" == "--vk-include" ]; then
        ((i++))
        VULKAN_INC_DIR=${args[i]}
    elif [ "$arg" == "--vk-lib" ]; then
        ((i++))
        VULKAN_LIB_DIR=${args[i]}
    elif [ "$arg" == "--cli-only" ]; then
        CLI_ONLY="-DBUILD_CLI_ONLY=ON"
    elif [ "$arg" == "--gui-only" ]; then
        GUI_ONLY="-DBUILD_GUI_ONLY=ON"
    elif [ "$arg" == "--no-vulkan" ]; then
        NO_VULKAN="-DRGA_ENABLE_VULKAN=OFF"
    elif [ "$arg" == "--no-fetch" ]; then
        NO_UPDATE="TRUE"
    elif [ "$arg" == "--automation" ]; then
        AUTOMATION="-DGUI_AUTOMATION=ON"
        TEST_DIR_SUFFIX="_test"
    elif [ "$arg" == "--cppcheck" ]; then
        CMAKE_CPPCHECK="-DCMAKE_CXX_CPPCHECK=cppcheck"
    elif [ "$arg" == "--clean" ]; then
        if [ -d "$OUTPUT_LINUX_DIR" ]; then
            echo "INFO: Deleting $OUTPUT_LINUX_DIR"
            rm -rf "$OUTPUT_LINUX_DIR"
        fi
        if [ -d "$SCRIPT_DIR/../build_output" ]; then
            echo "INFO: Deleting $SCRIPT_DIR/../build_output"
            rm -rf "$SCRIPT_DIR/../build_output"
        fi
        if [ -d "$SCRIPT_DIR/../output" ]; then
            echo "INFO: Deleting $SCRIPT_DIR/../output"
            rm -rf "$SCRIPT_DIR/../output"
        fi
        if [ -d "$SCRIPT_DIR/../output_test" ]; then
            echo "INFO: Deleting $SCRIPT_DIR/../output_test"
            rm -rf "$SCRIPT_DIR/../output_test"
        fi
        exit 0
    else
        echo "Unexpected argument: $arg. Aborting...";
        exit 1
    fi
done

OUTPUT_DIR="$OUTPUT_LINUX_DIR/make$TEST_DIR_SUFFIX"

if [ "$NO_VULKAN" == "-DRGA_ENABLE_VULKAN=OFF" ]; then
    if [ ! "$CLI_ONLY" == "-DBUILD_CLI_ONLY=ON" ]; then
        echo "ERROR: Invalid syntax: must use --cli-only with --no-vulkan"
        exit 1
    fi
fi

if [ -n "$QT_ROOT" ]; then
    CMAKE_QT="-DQT_PACKAGE_ROOT=$QT_ROOT -DNO_DEFAULT_QT=ON"
fi

if [ -n "$VULKAN_INC_DIR" ]; then
    CMAKE_VK_INC="-DVULKAN_SDK_INC_DIR=$VULKAN_INC_DIR"
fi

if [ -n "$VULKAN_LIB_DIR" ]; then
    CMAKE_VK_LIB="-DVULKAN_SDK_LIB_DIR=$VULKAN_LIB_DIR"
fi

# Create output folder
if [ ! -d "$OUTPUT_LINUX_DIR" ]; then
    mkdir $OUTPUT_LINUX_DIR
fi
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir $OUTPUT_DIR
fi

# Update Common
if [ "$NO_UPDATE" != "TRUE" ]; then
    echo ""
    echo "Updating Common..."
    python $SCRIPT_DIR/fetch_dependencies.py

    if [ $? -ne 0 ]; then
        echo "Error: encountered an error while fetching dependencies. Aborting..."
        exit 1
    fi
fi

# Launch cmake
echo ""
echo "Running cmake to generate Makefiles..."
cd $OUTPUT_DIR
$CMAKE_PATH -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_QT $CMAKE_VK_INC $CMAKE_VK_LIB $CLI_ONLY $GUI_ONLY $NO_VULKAN $AUTOMATION $CMAKE_CPPCHECK ../../..
cd $CURRENT_DIR
echo "Done."
