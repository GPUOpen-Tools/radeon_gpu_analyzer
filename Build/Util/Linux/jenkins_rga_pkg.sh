#!/bin/bash
set -x

# check for optional arguments
export BUILD_TYPE=Release
export AUTOMATION=
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
        --debug)
            export BUILD_TYPE=Debug
            shift # past argument
            ;;
        --automation)
            export AUTOMATION=_Test
            shift # past argument
            ;;
        *)
            echo "Invalid argumet $key"
            exit 1
            ;;
    esac
done

# Get the build version string
export MAJOR=`python3 RGA/Build/Util/Windows/JenkinsGetVersion.py --major`
export MINOR=`python3 RGA/Build/Util/Windows/JenkinsGetVersion.py --minor`
export UPDATE=`python3 RGA/Build/Util/Windows/JenkinsGetVersion.py --update`
export VERSION=$MAJOR.$MINOR.$UPDATE.$BUILD_NUMBER

# Create zip package folder
mkdir -p RGA/Output$AUTOMATION/rga-$VERSION
mkdir -p RGA/Output$AUTOMATION/rga-$VERSION/layer

# Script assumes it is creating a package from a BUILD_TYPE build config
# make sure clang is symbolic link to clang-7
cd RGA/Output$AUTOMATION/$BUILD_TYPE/bin/LC/OpenCL/bin
rm -f clang
ln -s clang-7 clang
cd ../../../../../../..

cp -rf RGA/Output$AUTOMATION/$BUILD_TYPE/bin/* RGA/Output$AUTOMATION/rga-$VERSION
rm -f RGA/Output$AUTOMATION/rga-$VERSION/*.json
mkdir -p RGA/Output$AUTOMATION/rga-$VERSION/Vulkan/amdvlk/
cp -f amdvlk64.so RGA/Output$AUTOMATION/rga-$VERSION/Vulkan/amdvlk/

cp RGA/Installer/Linux/rga_layer_* RGA/Output$AUTOMATION/rga-$VERSION/layer
cp VKLayerRGA-*/* RGA/Output$AUTOMATION/rga-$VERSION/layer
chmod +x RGA/Output$AUTOMATION/rga-$VERSION/layer/*

cp RGA/EULA.txt RGA/Output$AUTOMATION/rga-$VERSION/

cd RGA/Documentation/build
mkdir -p $WORKSPACE/RGA/Output$AUTOMATION/rga-$VERSION/Documentation/
cp -R html $WORKSPACE/RGA/Output$AUTOMATION/rga-$VERSION/Documentation/
cd $WORKSPACE

cd RGA/Output$AUTOMATION/
tar -zcvf ../../rga-$VERSION.tgz ./rga-$VERSION --exclude='.[^/]*' --exclude=*.tgz --exclude=*.rar --exclude=*.zip --exclude=*.lib --exclude=*.exe --exclude=*.dll --exclude=*.pdb
