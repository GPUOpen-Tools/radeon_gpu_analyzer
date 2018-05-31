#!/bin/bash

# Print help message
if [[ "$1" == "-h" ]] || [[ "$1" == "-help" ]] || [[ "$1" == "--h" ]] || [[ "$1" == "--help" ]]; then
    echo ""
    echo "This script generates Makefiles for RGA on Linux."
    echo ""
    echo "Usage:  gen_sln.bat [--cmake <cmake_path>] [--build <build_type>] [--qt <qt5_root>] [--cli-only] [--gui-only]"
    echo ""
    echo "Options:"
    echo "   --cmake        Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.\n"
    echo "   --build        The build type: \"release\" or \"debug\". The default is \"debug\"."
    echo "   --qt           Path to Qt5 root folder. The default is empty (cmake will look for Qt5 package istalled on the system)."
    echo "   --cli-only     Build RGA command-line tool only (do not build RGA GUI)."
    echo "   --no-fetch     Do not call FetchDependencies.py script before running cmake."
    echo ""
    echo "Examples:"
    echo "   gen_sln.bat"
    echo "   gen_sln.bat --build release"
    echo "   gen_sln.bat --qt C:\Qt\5.7\msvc2015_64"
    echo "   gen_sln.bat --build release --cli-only"
    echo ""
    exit 0
fi

# Defaults
CMAKE_PATH="cmake"
BUILD_TYPE="Debug"
CMAKE_QT=
QT_ROOT=
CLI_ONLY=
GUI_ONLY=

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
    elif [ "$arg" == "--cli-only" ]; then
        CLI_ONLY="-DBUILD_CLI_ONLY=ON"
    elif [ "$arg" == "--gui-only" ]; then
        GUI_ONLY="-DBUILD_GUI_ONLY=ON"
    elif [ "$arg" == "--no-fetch" ]; then
        NO_UPDATE="TRUE"
    else
        echo "Unexpected argument: $arg. Aborting...";
        exit 1
    fi
done

if [ -n "$QT_ROOT" ]; then
    CMAKE_QT="-DQT_PACKAGE_ROOT=$QT_ROOT -DNO_DEFAULT_QT=ON"
fi

CURRENT_DIR=$(pwd)
SCRIPT_DIR=$(dirname "$0")
OUTPUT_LINUX_DIR="$SCRIPT_DIR/Linux"
OUTPUT_MAKE_DIR="$OUTPUT_LINUX_DIR/Make"
OUTPUT_DIR="$OUTPUT_MAKE_DIR/$BUILD_TYPE"

# Create output folder
if [ ! -d "$OUTPUT_LINUX_DIR" ]; then
    mkdir $OUTPUT_LINUX_DIR
fi
if [ ! -d "$OUTPUT_MAKE_DIR" ]; then
    mkdir $OUTPUT_MAKE_DIR
fi
if [ ! -d "$OUTPUT_DIR" ]; then
    mkdir $OUTPUT_DIR
fi

# Update Common
if [ "$NO_UPDATE" != "TRUE" ]; then
    echo ""
    echo "Updating Common..."
    python $SCRIPT_DIR/FetchDependencies.py
fi

# Launch cmake
echo ""
echo "Running cmake to generate Makefiles..."
cd $OUTPUT_DIR
$CMAKE_PATH -DCMAKE_BUILD_TYPE=$BUILD_TYPE $CMAKE_QT $CLI_ONLY $GUI_ONLY ../../../..
cd $CURRENT_DIR
echo "Done."
