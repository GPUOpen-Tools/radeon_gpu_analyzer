#!bash
set -x
set -e
export MAJOR=`python RGA/Build/Util/Windows/JenkinsGetVersion.py --major`
export MINOR=`python RGA/Build/Util/Windows/JenkinsGetVersion.py --minor`
export UPDATE=`python RGA/Build/Util/Windows/JenkinsGetVersion.py --update`
export VERSION=$MAJOR.$MINOR.$UPDATE.$BUILD_NUMBER
mkdir -p RGA/Output/rga-$VERSION
mkdir -p RGA/Output/rga-$VERSION/layer

# make sure clang is symbolic link to clang-7
cd RGA/Output/bin/LC/OpenCL/bin
rm -f clang
ln -s clang-7 clang
cd ../../../../../..

cp -rf RGA/Output/bin/* RGA/Output/rga-$VERSION
mkdir -p RGA/Output/rga-$VERSION/Vulkan/amdvlk/
cp -f amdvlk64.so RGA/Output/rga-$VERSION/Vulkan/amdvlk/

cp RGA/Installer/Linux/rga_layer_* RGA/Output/rga-$VERSION/layer
cp VKLayerRGA-*/* RGA/Output/rga-$VERSION/layer
chmod +x RGA/Output/rga-$VERSION/layer/*

cp RGA/EULA.txt RGA/Output/rga-$VERSION/

cd RGA/Documentation/build
mkdir -p $WORKSPACE/RGA/Output/rga-$VERSION/Documentation/
cp -R html $WORKSPACE/RGA/Output/rga-$VERSION/Documentation/
cd $WORKSPACE

cd RGA/Output
tar -zcvf ../../rga-$VERSION.tgz ./rga-$VERSION --exclude='.[^/]*' --exclude=*.tgz --exclude=*.rar --exclude=*.zip --exclude=*.lib --exclude=*.exe --exclude=*.dll --exclude=*.pdb
