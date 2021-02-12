#!bash
OUTPUT_DIR=$1
set -e
if [ "$2" = "-internal" ]; then
  INTERNAL=true
fi

set -x

X64_DIR=$OUTPUT_DIR/utils

# Get the build version string
export MAJOR=`python3 ../../../Build/Util/get_version.py --major`
export MINOR=`python3 ../../../Build/Util/get_version.py --minor`

# Copy the Vulkan offline backend.
if [ ! -d "$X64_DIR" ]; then
  mkdir -p $X64_DIR
fi
cp ../../../Core/VulkanOffline/lnx64/* $X64_DIR/
chmod +x $X64_DIR/amdspv
chmod +x $X64_DIR/spvgen.so

# Copy the OpenGL backend.
cp ../../../Core/OpenGL/VirtualContext/Release/lnx64/VirtualContext $X64_DIR/
chmod +x $X64_DIR/VirtualContext

# Copy the LC Compiler.
if [ ! -d "$X64_DIR/LC" ]; then
  mkdir -p $X64_DIR/LC
fi
cp -rf ../../../Core/LC/OpenCL/linux $X64_DIR/LC/OpenCL
cp -f ../../../Core/LC/OpenCL/additional-targets $X64_DIR/LC/OpenCL/

# Copy the LC disassembler.
if [ ! -d "$X64_DIR/LC/Disassembler" ]; then
  mkdir $X64_DIR/LC/Disassembler
fi
cp ../../../Core/LC/Disassembler/Linux/amdgpu-dis $X64_DIR/LC/Disassembler

# make sure LC/OpenCL/bin/clang is link to clang-7
CURDIR=`pwd`
rm -f $X64_DIR/LC/OpenCL/bin/clang
cd $X64_DIR/LC/OpenCL/bin/
ln -s clang-7 clang
cd $CURDIR
chmod +x $X64_DIR/LC/OpenCL/bin/l* $X64_DIR/LC/OpenCL/bin/clang*
chmod +x $X64_DIR/LC/OpenCL/lib/bitcode/*.bc

# Copy the static analysis backend.
if [ "$INTERNAL" = true ]; then
  cp ../../../../RGA-Internal/core/shader_analysis/linux/x64/shae-internal $X64_DIR/
  chmod +x $X64_DIR/shae-internal
  if [ -e $X64_DIR/shae ]; then
    rm bin/shae
  fi
  if [ -e $OUTPUT_DIR/rga-bin ]; then
    rm $OUTPUT_DIR/rga-bin
  fi
else
  cp ../../../Core/ShaderAnalysis/Linux/x64/shae $X64_DIR/
  chmod +x $X64_DIR/shae
  if [ -e b$X64_DIR/shae-internal ]; then
    rm $X64_DIR/shae-internal
  fi
  if [ -e $OUTPUT_DIR/rga-bin-internal ]; then
    rm $OUTPUT_DIR/rga-bin-internal
  fi
fi

# Copy the Vulkan tools.
if [ ! -d "$X64_DIR/Vulkan" ]; then
  mkdir -p $X64_DIR/Vulkan
fi
cp ../../../Core/Vulkan/tools/Lnx64/bin/* "$X64_DIR/Vulkan/"
chmod +x $X64_DIR/Vulkan/*

# Copy the Radeon Tools Download Assistant.
cp ../../../../Common/Src/UpdateCheckAPI/rtda/linux/rtda $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/rtda

# Copy the launch script.
cp ./rga $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/rga

# Copy license files.
cp ../../../License.txt $OUTPUT_DIR/
cp ../../../RGA_NOTICES.txt $OUTPUT_DIR/

# Copy README.md and Release notes
cp ../../../README.md $OUTPUT_DIR/
cp ../../../Documentation/RGA_RELEASE_NOTES.txt $OUTPUT_DIR/
