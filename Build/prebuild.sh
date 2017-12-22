#!/bin/bash

# Print help message
if [[ "$1" == "-h" ]] || [[ "$1" == "-help" ]] || [[ "$1" == "--h" ]] || [[ "$1" == "--help" ]]; then
    echo ""
    echo "This script generates Makefiles for RGA on Linux."
    echo ""
    echo "Usage:  gen_sln.bat [--cmake <cmake_path>] [--build <build_type>]"
    echo ""
    echo "Options:"
    echo "   --cmake        Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.\n"
    echo "   --build        The build type: \"release\" or \"debug\". The default is \"debug\"."
    echo "   --no-update    Do not call UpdateCommon.py script before running cmake."
    echo ""
    echo "Examples:"
    echo "   gen_sln.bat"
    echo "   gen_sln.bat --build release"
    echo ""
    exit 0
fi

# Defaults
CMAKE_PATH="cmake"
BUILD_TYPE="Debug"

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
    elif [ "$arg" == "--no-update" ]; then
        NO_UPDATE="TRUE"
    else
        echo "Unexpected argument: $arg. Aborting...";
        exit 1
    fi
done

CURRENT_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")
OUTPUT_CMAKE_DIR="$SCRIPT_DIR/CMake"
OUTPUT_LINUX_DIR="$SCRIPT_DIR/CMake/linux"

# Create output folder
if [ ! -d "$OUTPUT_CMAKE_DIR" ]; then
    mkdir $OUTPUT_CMAKE_DIR
fi
if [ ! -d "$OUTPUT_LINUX_DIR" ]; then
    mkdir $OUTPUT_LINUX_DIR
fi

# Call UpdateCommon.py
if [ "$NO_UPDATE" != "TRUE" ]; then
    echo ""
    echo "Updating Common..."
    python $SCRIPT_DIR/../UpdateCommon.py
    if [ -f $SCRIPT_DIR/../UpdateCommon_PreRelease.py ]; then
        python $SCRIPT_DIR/../UpdateCommon_PreRelease.py
    fi
fi

# Launch cmake
echo ""
echo "Running cmake to generate Makefiles..."
cd $OUTPUT_LINUX_DIR
$CMAKE_PATH -DCMAKE_BUILD_TYPE=$BUILD_TYPE ../../..
cd $CURRENT_DIR
echo "Done."
