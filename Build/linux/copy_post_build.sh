# Copy the Vulkan backend.
cp ../../Core/Vulkan/rev_1_0_0/Release/lnx64/* ../../output/bin/
chmod +x ../../output/bin/amdspv
chmod +x ../../output/bin/spvgen.so

# Copy the OpenGL backend.
cp ../../Core/OpenGL/VirtualContext/Release/lnx64/VirtualContext ../../output/bin/
chmod +x ../../output/bin/VirtualContext 

# Copy the static analysis backend.
cp ../../Core/ShaderAnalysis/Linux/x64/shae ../../output/bin/
chmod +x ../../output/bin/shae

# Copy the launch script.
cp ./rga ../../output/bin/
chmod +x ../../output/bin/rga

