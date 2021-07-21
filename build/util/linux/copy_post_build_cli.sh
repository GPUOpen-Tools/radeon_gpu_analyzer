#!bash
OUTPUT_DIR=$1
set -e
if [ "$2" = "-internal" ]; then
  INTERNAL=true
fi

set -x

X64_DIR=$OUTPUT_DIR/utils

# Get the build version string
export MAJOR=`python3 ../../../build/util/get_version.py --major`
export MINOR=`python3 ../../../build/util/get_version.py --minor`

# Copy the Vulkan offline backend.
if [ ! -d "$X64_DIR" ]; then
  mkdir -p $X64_DIR
fi
cp ../../../external/vulkan_offline/linux/* $X64_DIR/
chmod +x $X64_DIR/amdspv
chmod +x $X64_DIR/spvgen.so

# Copy the OpenGL backend.
cp ../../../external/opengl/VirtualContext/linux/VirtualContext $X64_DIR/
chmod +x $X64_DIR/VirtualContext

# Copy the LC Compiler.
if [ ! -d "$X64_DIR/LC" ]; then
  mkdir -p $X64_DIR/LC
fi
mkdir -p $X64_DIR/LC/OpenCL/lib/clang/13.0.0/include
cp -rf ../../../external/lc/opencl/linux/* $X64_DIR/LC/OpenCL
rm -f $X64_DIR/LC/OpenCL/include/opencl-c-base.h
cp -f ../../../external/lc/opencl/additional-targets $X64_DIR/LC/OpenCL/
cp -f ../../../external/lc/opencl/linux/include/opencl-c-base.h $X64_DIR/LC/OpenCL/lib/clang/13.0.0/include/

# Copy the LC disassembler.
if [ ! -d "$X64_DIR/LC/Disassembler" ]; then
  mkdir $X64_DIR/LC/Disassembler
fi
cp ../../../external/lc/disassembler/linux/amdgpu-dis $X64_DIR/LC/Disassembler

# make sure LC/OpenCL/bin/clang is link to clang-13
CURDIR=`pwd`
rm -f $X64_DIR/LC/OpenCL/bin/clang
cd $X64_DIR/LC/OpenCL/bin/
ln -s clang-13 clang
ln -s lld ld.lld
ln -s llvm-readobj llvm-readelf
cd $CURDIR

chmod +x $X64_DIR/LC/OpenCL/bin/lld $X64_DIR/LC/OpenCL/bin/clang-13 $X64_DIR/LC/OpenCL/bin/llvm-objdump $X64_DIR/LC/OpenCL/bin/llvm-readobj
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
  cp ../../../source/utils/shader_analysis/linux/x64/shae $X64_DIR/
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
cp ../../../external/vulkan/tools/linux/bin/* "$X64_DIR/Vulkan/"
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
cp ../../../documentation/RGA_RELEASE_NOTES.txt $OUTPUT_DIR/
