
if [ "$1" = "-internal" ]; then
  INTERNAL=true
fi

# Copy the Vulkan backend.
cp ../../Core/Vulkan/rev_1_0_0/Release/lnx64/* ../../output/bin/
chmod +x ../../output/bin/amdspv
chmod +x ../../output/bin/spvgen.so

# Copy the OpenGL backend.
cp ../../Core/OpenGL/VirtualContext/Release/lnx64/VirtualContext ../../output/bin/
chmod +x ../../output/bin/VirtualContext

# Copy the ROCm Compiler.
if [ ! -d "../../output/bin/ROCm" ]; then
  mkdir ../../output/bin/ROCm
fi
cp -rf ../../Core/ROCm/OpenCL/linux ../../output/bin/ROCm/OpenCL
cp ../../Core/ROCm/OpenCL/additional-targets ../../output/bin/ROCm/OpenCL/

# Copy the static analysis backend.
if [ "$INTERNAL" = true ]; then
  cp ../../../RGA-Internal/Core/ShaderAnalysis/Linux/x64/shae-internal ../../output/bin/
  chmod +x ../../output/bin/shae-internal
  if [ -e ../../output/bin/shae ]; then
    rm ../../output/bin/shae
  fi
  if [ -e ../../output/bin/rga-bin ]; then
    rm ../../output/bin/rga-bin
  fi
else
  cp ../../Core/ShaderAnalysis/Linux/x64/shae ../../output/bin/
  chmod +x ../../output/bin/shae
  if [ -e ../../output/bin/shae-internal ]; then
    rm ../../output/bin/shae-internal
  fi
  if [ -e ../../output/bin/rga-bin-internal ]; then
    rm ../../output/bin/rga-bin-internal
  fi
fi

# Copy the launch script.
cp ./rga ../../output/bin/
chmod +x ../../output/bin/rga

# Copy license files.
cp ../../License.txt ../../output/bin/
cp ../../RGAThirdPartyLicenses.txt ../../output/bin/

