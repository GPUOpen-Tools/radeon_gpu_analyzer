#!bash
OUTPUT_DIR=$1

if [ "$2" = "-internal" ]; then
  INTERNAL=true
fi

set -x

# Copy the Vulkan offline backend.
cp ../../../Core/VulkanOffline/lnx64/* $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/amdspv
chmod +x $OUTPUT_DIR/spvgen.so

# Copy the OpenGL backend.
cp ../../../Core/OpenGL/VirtualContext/Release/lnx64/VirtualContext $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/VirtualContext

# Copy the LC Compiler.
if [ ! -d "$OUTPUT_DIR/LC" ]; then
  mkdir $OUTPUT_DIR/LC
fi
cp -rf ../../../Core/LC/OpenCL/linux $OUTPUT_DIR/LC/OpenCL
cp -f ../../../Core/LC/OpenCL/additional-targets $OUTPUT_DIR/LC/OpenCL/

# Copy the LC disassembler.
if [ ! -d "$OUTPUT_DIR/LC/Disassembler" ]; then
  mkdir $OUTPUT_DIR/LC/Disassembler
fi
cp ../../../Core/LC/Disassembler/Linux/amdgpu-dis $OUTPUT_DIR/LC/Disassembler

# make sure LC/OpenCL/bin/clang is link to clang-7
CURDIR=`pwd`
rm -f $OUTPUT_DIR/LC/OpenCL/bin/clang
cd $OUTPUT_DIR/LC/OpenCL/bin/
ln -s clang-7 clang
cd $CURDIR
chmod +x $OUTPUT_DIR/LC/OpenCL/bin/l* $OUTPUT_DIR/LC/OpenCL/bin/clang*
chmod +x $OUTPUT_DIR/LC/OpenCL/lib/bitcode/*.bc

# Copy the static analysis backend.
if [ "$INTERNAL" = true ]; then
  cp ../../../../RGA-Internal/Core/ShaderAnalysis/Linux/x64/shae-internal $OUTPUT_DIR/
  chmod +x $OUTPUT_DIR/shae-internal
  if [ -e $OUTPUT_DIR/shae ]; then
    rm bin/shae
  fi
  if [ -e $OUTPUT_DIR/rga-bin ]; then
    rm $OUTPUT_DIR/rga-bin
  fi
else
  cp ../../../Core/ShaderAnalysis/Linux/x64/shae $OUTPUT_DIR/
  chmod +x $OUTPUT_DIR/shae
  if [ -e b$OUTPUT_DIR/shae-internal ]; then
    rm $OUTPUT_DIR/shae-internal
  fi
  if [ -e $OUTPUT_DIR/rga-bin-internal ]; then
    rm $OUTPUT_DIR/rga-bin-internal
  fi
fi

# Copy the Vulkan tools.
if [ ! -d "$OUTPUT_DIR/Vulkan" ]; then
  mkdir $OUTPUT_DIR/Vulkan
fi
cp ../../../Core/Vulkan/tools/Lnx64/bin/* "$OUTPUT_DIR/Vulkan/"
chmod +x $OUTPUT_DIR/Vulkan/*

# Copy the AMDToolsDownloader.
cp ../../../../Commmon/Src/UpdateCheckAPI/AMDToolsDownloader/Linux/AMDToolsDownloader $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/AMDToolsDownloader

# Copy the launch script.
cp ./rga $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/rga

# Copy license files.
cp ../../../License.txt $OUTPUT_DIR/
cp ../../../RGAThirdPartyLicenses.txt $OUTPUT_DIR/

