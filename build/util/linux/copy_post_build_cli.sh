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
chmod +x $X64_DIR/amdllpc

# Copy the OpenGL backend.
cp ../../../external/opengl/glc/linux/glc $X64_DIR/
chmod +x $X64_DIR/glc

# Copy the LC Compiler.
if [ ! -d "$X64_DIR/lc" ]; then
  mkdir -p $X64_DIR/lc
fi
mkdir -p $X64_DIR/lc/opencl/lib/clang/16.0.0/include
cp -rf ../../../external/lc/opencl/linux/* $X64_DIR/lc/opencl
rm -f $X64_DIR/lc/opencl/include/opencl-c-base.h
cp -f ../../../external/lc/opencl/additional-targets $X64_DIR/lc/opencl/
cp -f ../../../external/lc/opencl/linux/include/opencl-c-base.h $X64_DIR/lc/opencl/lib/clang/16.0.0/include/

# Copy the LC disassembler.
if [ ! -d "$X64_DIR/lc/disassembler" ]; then
  mkdir $X64_DIR/lc/disassembler
fi
cp ../../../external/lc/disassembler/linux/amdgpu-dis $X64_DIR/lc/disassembler

# make sure lc/opencl/bin/clang is link to clang-16
CURDIR=`pwd`
rm -f $X64_DIR/lc/opencl/bin/clang
cd $X64_DIR/lc/opencl/bin/
# clean up old symlinks
if [ -e clang ]; then
    rm -f clang
fi
if [ -e ld.lld ]; then
    rm -f ld.lld
fi
if [ -e llvm-readelf ]; then
    rm -f llvm-readelf
fi
# create new symlinks
ln -s clang-16 clang
ln -s lld ld.lld
ln -s llvm-readobj llvm-readelf
cd $CURDIR

chmod +x $X64_DIR/lc/opencl/bin/lld $X64_DIR/lc/opencl/bin/clang-16 $X64_DIR/lc/opencl/bin/llvm-objdump $X64_DIR/lc/opencl/bin/llvm-readobj
chmod +x $X64_DIR/lc/opencl/lib/bitcode/*.bc

# Copy the static analysis backend.
if [ "$INTERNAL" = true ]; then
  cp ../../../../rga_internal_utils/core/shader_analysis/linux/x64/shae-internal $X64_DIR/
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
if [ ! -d "$X64_DIR/vulkan" ]; then
  mkdir -p $X64_DIR/vulkan
fi
cp ../../../external/vulkan/tools/linux/bin/* "$X64_DIR/vulkan/"
chmod +x $X64_DIR/vulkan/*spirv*
chmod +x $X64_DIR/vulkan/glslangValidator

# Copy the Radeon Tools Download Assistant.
cp ../../../../Common/Src/update_check_api/rtda/linux/rtda $OUTPUT_DIR/
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

