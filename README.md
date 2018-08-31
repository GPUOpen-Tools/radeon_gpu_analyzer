# RGA (Radeon GPU Analyzer) #

Radeon GPU Analyzer is an offline compiler and a performance analysis tool for DirectX shaders, OpenGL shaders,
Vulkan shaders and OpenCL kernels. Using this product, you can compile source code for a variety of AMD GPUs and APUs,
independent from the GPU/APU that is physically installed on your system, and generate AMD ISA disassembly, intermediate language disassembly, 
performance statistics and static analysis reports for each target platform.

This product can be used to produce the following output:
* Build errors and warnings
* AMD ISA, AMDIL and HSAIL disassembly
* DX ASM for D3D shaders
* Compiled binaries
* Performance statistics
* Live register analysis (see http://gpuopen.com/live-vgpr-analysis-radeon-gpu-analyzer/ for more info)
* Control-Flow graphs

The RGA package contains both a GUI app and a command-line exectuable.

The supported platforms by the **command-line tool** are:
* D3D11
* OpenCL - AMD's LLVM-based Lightning Compiler for ROCm (-s rocm-cl)
* OpenCL - Legacy compiler (-s cl)
* OpenGL
* Vulkan - GLSL input
* Vulkan - SPIR-V binary input
* Vulkan - SPIR-V textual input
* AMD IL

The supported platforms by the **GUI app** are:
* OpenCL - AMD's LLVM-based Lightning Compiler for ROCm 

## System Requirements ##

* Windows: 7, 8.1 or 10, 64-bit. Visual Studio 2015 or later.
* Linux: Ubuntu 14.04, Red Hat 6.4 or later. Build with gcc 4.7.2 or later.

The Radeon Software Crimson Edition or AMD Catalyst release must be installed to run this tool.

## Build Instructions ##

### Building on Windows ###
As a preliminary step, make sure that you have the following installed on your system:
* CMake
* Python 2.7 (or above)
* Qt (in case that you are interested in building the GUI app; you can build the command line executable without Qt)

cd to the Build sub-folder, and run:
   
prebuild.bat --build release (or: debug)

Running the prebuild script will fetch all the dependencies, and generate the solution file for VS 2015. 
After successfully running the preuild script, open RGA.sln from Build\CMake\VS2015, and build:
* RadeonGPUAnalyzerCLI project for the command line exectuable
* RadeonGPUAnalyzerGUI project for the GUI app

Some useful options of the prebuild script:
* --cli-only: only build the command line tool (do not build the GUI app)
* --build <configuration>: by default, the solution files would be generated for Debug configuration. Add --build Release to generate the solution files for Release configuration
* --qt <path>: full path to the folder from where you would like the Qt binaries to be retrieved
* --no-fetch: do not attempt to update the third party repositories
* --vs <VS version>: generate the solution files for a Visual Studio version that is different than 2015. For example, to target VS 2017, add --vs 2017 to the command.
   
If you are intending to analyze DirectX shaders using RGA, copy the x64 version of Microsoft's D3D compiler to a subdirectory
named "x64" under the RGA executable's directory (for example, D3DCompiler_47.dll).

-=-
   
If for some reason you do not want to use the prebuild.bat script, you can also manually fetch the dependencies and generate the solution and project files:
   
Start by running the fetch_dependencies.py script to fetch the solution's dependencies.
   
To generate the files for an x64 build, use:
   
  cmake.exe -G "Visual Studio 14 2015 Win64"  –DCMAKE_BUILD_TYPE=Release (or: Debug) <full path to the RGA repo directory>
   
If you are intending to analyze DirectX shaders using RGA, copy the x64 version of Microsoft's D3D compiler to a subdirectory
named "x64" under the RGA executable's directory (for example, D3DCompiler_47.dll).

  
### Building on Ubuntu ###
* One time setup:
  * sudo apt-get install libboost-all-dev
  * sudo apt-get install gcc-multilib g++-multilib
  * sudo apt-get install libglu1-mesa-dev mesa-common-dev libgtk2.0-dev
  * sudo apt-get install zlib1g-dev libx11-dev:i386
  * sudo apt install cmake
  * Install python 2.7 (or above)
  * To build the GUI app, you should also have Qt installed
  
* Build: 

   cd to the Build sub-folder, and run:
   
   prebuild.sh --build release (or: debug)
   
   This will fetch all the dependencies, and generate the makefiles.

   Then, cd to the auto-generated subfolder Build/CMake/linux and run make.

   -=-
   
   If for some reason you do not want to use the prebuild.sh script, you can also manually fetch the dependencies and generate the makefiles:
   
  * run: fetch_dependencies.py
  * run: cmake –DCMAKE_BUILD_TYPE=Release (or: Debug) <full or relative path to the RGA repo directory>
    
	It is recommended to create a directory to hold all build files, and launch cmake from that directory.
	
	For example:
	* cd to the RGA repo directory
	* mkdir _build
	* cd _build
	* cmake –DCMAKE_BUILD_TYPE=Release ../
  * run: make

#### Building on CENTOS 6.X ####
Install gcc 4.7.2
* sudo wget http://people.centos.org/tru/devtools-1.1/devtools-1.1.repo -P /etc/yum.repos.d
* sudo sh -c 'echo "enabled=1" >> /etc/yum.repos.d/devtools-1.1.repo'
* sudo yum install devtoolset-1.1
* wget http://people.centos.org/tru/devtools-1.1/6/i386/RPMS/devtoolset-1.1-libstdc++-devel-4.7.2-5.el6.i686.rpm
* sudo yum install devtoolset-1.1-libstdc++-devel-4.7.2-5.el6.i686.rpm
* sudo ln -s /opt/centos/devtoolset-1.1/root/usr/bin/* /usr/local/bin/
* hash -r
* gcc --version (verify that version 4.7.2 is displayed)

Install cmake
* sudo yum install cmake

Install python 2.7 (or above)

Install zlib
* yum install zlib-devel

Install glibc
* yum -y install glibc-devel.i686 glibc-devel
 
Building RGA

cd to the Build sub-folder, and run:
   
* prebuild.sh --build release (or: debug)
  
  This will fetch all the dependencies, and generate the makefiles.

* cd to the auto-generated subfolder Build/CMake/linux and run make.

   -=-
   
   If for some reason you do not want to use the prebuild.sh script, you can also manually fetch the dependencies and generate the makefiles:
   
  * run: fetch_dependencies.py
  * run: cmake –DCMAKE_BUILD_TYPE=Release (or: Debug) <full or relative path to the RGA repo directory>
    
    It is recommended to create a directory to hold all build files, and launch cmake from that directory.

    For example:
    * cd to the RGA repo directory
    * mkdir _build
    * cd _build
    * cmake –DCMAKE_BUILD_TYPE=Release ../
  * run: make
  
## Running ##
### GUI App ###
Run the RadeonGPUAnalyzerGUI exectuable. The app provides a quickstart guide and a help manual under Help. 

### Command Line Executable ###

Run the rga executable.

* Usage: 
  * General: rga -h
  * DirectX: rga -s hlsl -h
        
    Note: RGA's HLSL mode requires Microsoft's D3D Compiler DLL in runtime. If you copy the relevant D3D Compiler DLL to the x64 
	subdirectory under the executable's directory, RGA will use that DLL in runtime. The default D3D compiler that RGA public releases ship with
	is d3dcompiler_47.dll.
  * OpenGL:  rga -s opengl -h
  * ROCm OpenCL:  rga -s rocm-cl -h
  * Legacy OpenCL:  rga -s cl -h
  * Vulkan - glsl:  rga -s vulkan -h
  * Vulkan - SPIR-V binary input:  rga -s vulkan-spv -h
  * Vulkan - SPIRV-V textual input:  rga -s vulkan-spv-txt -h
  * AMD IL: rga -s amdil -h

## Support ##
For support, please visit the RGA repository github page: https://github.com/GPUOpen-Tools/RGA

## License ##
Radeon GPU Analyzer is licensed under the MIT license. See the License.txt file for complete license information.

## Copyright information ##
Please see RGAThirdPartyLicenses.txt for copyright and third party license information.

