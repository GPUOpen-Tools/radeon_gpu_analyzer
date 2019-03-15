
OUTPUT_DIR=$1

if [ "$2" = "-internal" ]; then
  INTERNAL=true
fi

# Copy the Vulkan offline backend.
cp ../../../Core/VulkanOffline/lnx64/* $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/amdspv
chmod +x $OUTPUT_DIR/spvgen.so

# Copy the OpenGL backend.
cp ../../../Core/OpenGL/VirtualContext/Release/lnx64/VirtualContext $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/VirtualContext

# Copy the ROCm Compiler.
if [ ! -d "$OUTPUT_DIR/ROCm" ]; then
  mkdir $OUTPUT_DIR/ROCm
fi
cp -rf ../../../Core/ROCm/OpenCL/linux $OUTPUT_DIR/ROCm/OpenCL
cp -f ../../../Core/ROCm/OpenCL/additional-targets $OUTPUT_DIR/ROCm/OpenCL/

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

# Copy the AMDToolsDownloader.
cp ../../../../Commmon/Src/UpdateCheckAPI/AMDToolsDownloader/Linux/AMDToolsDownloader $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/AMDToolsDownloader

# Copy the launch script.
cp ./rga $OUTPUT_DIR/
chmod +x $OUTPUT_DIR/rga

# Copy license files.
cp ../../../License.txt $OUTPUT_DIR/
cp ../../../RGAThirdPartyLicenses.txt $OUTPUT_DIR/

