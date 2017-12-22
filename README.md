# RGA (Radeon GPU Analyzer) #

RGA is an offline compiler and a performance analysis tool for DirectX shaders, OpenGL shaders,
Vulkan shaders and OpenCL kernels. Using this product, you can compile source code for a variety of AMD GPUs and APUs,
independent from the GPU/APU that is physically installed on your system, and generate AMD ISA disassembly, intermediate language disassembly, 
performance statistics and static analysis reports for each target platform.

This product can be used to produce the following output:
* Build errors and warnings
* AMD ISA, AMDIL and HSAIL disassembly
* DX ASM for D3D shaders
* Compiled binaries
* Performance statistics
* Live register analysis (for more info about this feature, please see http://gpuopen.com/live-vgpr-analysis-radeon-gpu-analyzer/)
* Control-Flow graphs

The supported platforms are:
* D3D11
* OpenCL - ROCm Lightning Compiler (-s rocm-cl)
* OpenCL - Legacy compiler (-s cl)
* OpenGL
* Vulkan - GLSL input
* Vulkan - SPIR-V binary input
* Vulkan - SPIR-V textual input
* AMD IL

## System Requirements ##

* Windows: 7, 8.1 or 10, 64-bit. Visual Studio 2015.
* Linux: Ubuntu 14.04, Red Hat 6.4 or later. Build with gcc 4.7.2 or later.

The Radeon Software Crimson Edition or AMD Catalyst release must be installed to run this tool.

## Build Instructions ##

### Building on Windows ###
There are two ways to build RGA on Windows:


1. Using CMake. This is the recommended way to build RGA, and soon to be the only way.

   Cd to the Build sub-folder, and run:
   
   prebuild.bat --build release (or: debug)
   
   This will fetch all the dependencies, and generate the solution file for VS 2015. To target VS 2017, add --vs 2017 to the command.

   Open RGA.sln from Build\CMake\VS2015, and build the RadeonGPUAnalyzerCLI project.
   
   If you are intending to analyze DirectX shaders using RGA, copy the x64 version of Microsoft's D3D compiler to a subdirectory
   named "x64" under the RGA executable's directory (for example, D3DCompiler_47.dll).

   -=-
   
   If for some reason you do not want to use the prebuild.bat script, you can also manually fetch the dependencies and generate the solution and project files:
   
   Start by running the UpdateCommon.py script to fetch the dependencies.
   
   Then, for an x86 build, use:

   cmake.exe –DCMAKE_BUILD_TYPE=Release (or: Debug) <full path to the RGA repo directory>
   
   To generate the files for an x64 build, use:
   
   cmake.exe -G "Visual Studio 14 2015 Win64"  –DCMAKE_BUILD_TYPE=Release (or: Debug) <full path to the RGA repo directory>
   
   If you are intending to analyze DirectX shaders using RGA, copy the x64 version of Microsoft's D3D compiler to a subdirectory
   named "x64" under the RGA executable's directory (for example, D3DCompiler_47.dll).

   
2. Using the RadeonGPUAnalyzer.sln file that can be found under Build\VS2015. 
   Remember to run the UpdateCommon.py script, which fetches the solution's dependencies before using the solution file,.
   Heads-up: please note that this is not the recommended way to build RGA, and that this solution file is about to be removed in future updates of the repository.
  

### Building on Ubuntu ###
* One time setup:
  * sudo apt-get install libboost-all-dev
  * sudo apt-get install gcc-multilib g++-multilib
  * sudo apt-get install libglu1-mesa-dev mesa-common-dev libgtk2.0-dev
  * sudo apt-get install zlib1g-dev libx11-dev:i386
* Build: 
   Cd to the Build sub-folder, and run:
   
   prebuild.bat --build release (or: debug)
   
   This will fetch all the dependencies, and generate the makefiles.

   Then cd to the auto-generated subfolder Build/CMake/linux and run make.

   -=-
   
   If for some reason you do not want to use the prebuild.bat script, you can also manually fetch the dependencies and generate the makefiles:
   
  * run: UpdateCommon.py
  * run: cmake –DCMAKE_BUILD_TYPE=Release (or: Debug) <full or relative path to the RGA repo directory>
    
	It is recommended to create a directory to hold all build files, and launch cmake from that directory.
	
	For example:
	* cd to the RGA repo directory
	* mkdir _build
	* cd _build
	* cmake –DCMAKE_BUILD_TYPE=Release ../
  * run: make

#### Building on CENTOS 6.X ####
Install compiler 4.7.2
* sudo wget http://people.centos.org/tru/devtools-1.1/devtools-1.1.repo -P /etc/yum.repos.d
* sudo sh -c 'echo "enabled=1" >> /etc/yum.repos.d/devtools-1.1.repo'
* sudo yum install devtoolset-1.1
* wget http://people.centos.org/tru/devtools-1.1/6/i386/RPMS/devtoolset-1.1-libstdc++-devel-4.7.2-5.el6.i686.rpm
* sudo yum install devtoolset-1.1-libstdc++-devel-4.7.2-5.el6.i686.rpm
* sudo ln -s /opt/centos/devtoolset-1.1/root/usr/bin/* /usr/local/bin/
* hash -r
* gcc --version (verify that version 4.7.2 is displayed)
Install zlib
* yum install zlib-devel

Install glibc
* yum -y install glibc-devel.i686 glibc-devel
 
Building RGA
Cd to the Build sub-folder, and run:
   
   prebuild.bat --build release (or: debug)
   
   This will fetch all the dependencies, and generate the makefiles.

   Then, cd to the auto-generated subfolder Build/CMake/linux and run make.

   -=-
   
   If for some reason you do not want to use the prebuild.bat script, you can also manually fetch the dependencies and generate the makefiles:
   
  * run: UpdateCommon.py
  * run: cmake –DCMAKE_BUILD_TYPE=Release (or: Debug) <full or relative path to the RGA repo directory>
    
	It is recommended to create a directory to hold all build files, and launch cmake from that directory.
	
	For example:
	* cd to the RGA repo directory
	* mkdir _build
	* cd _build
	* cmake –DCMAKE_BUILD_TYPE=Release ../
  * run: make
## Running ##
Run the rga executable.

* Usage: 
  * General: rga -h
  * DirectX: rga -s hlsl -h
        
    Note: RGA's HLSL mode requires Microsoft's D3D Compiler DLL in runtime. If you copy the relevant D3D Compiler DLL to the relevant (x64 or x86) 
	subdirectory under the executable's directory, RGA will use that DLL in runtime. The default D3D compiler that RGA public releases ship with
	is d3dcompiler_47.dll.
  * OpenGL:  rga -s opengl -h
  * OpenCL:  rga -s cl -h
  * Vulkan - glsl:  rga -s vulkan -h
  * Vulkan - SPIR-V binary input:  rga -s vulkan-spv -h
  * Vulkan - SPIRV-V textual input:  rga -s vulkan-spv-txt -h
  * AMD IL: rga -s amdil -h

## Support ##
For support, please visit the RGA repository github page: https://github.com/GPUOpen-Tools/RGA

## License ##
Radeon GPU Analyzer is licensed under the MIT license. See LICENSE file for full license information.

## Copyright information ##

**Boost**

Copyright Beman Dawes, 2003.
    
**TinyXML**

TinyXML is released under the zlib license
Files: *
Copyright: 2000-2007, Lee Thomason, 2002-2004, Yves Berquin 
Files: tinystr.*
Copyright: 2000-2007, Lee Thomason, 2002-2004, Yves Berquin, 2005, Tyge Lovset

**TinyXML-2**

TinyXML-2 is released under the zlib license:

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
This notice may not be removed or altered from any source distribution.

**glew**

The OpenGL Extension Wrangler Library
Copyright (C) 2002-2007, Milan Ikits <milan ikits@ieee org>
Copyright (C) 2002-2007, Marcelo E. Magallon <mmagallo@debian org>
Copyright (C) 2002, Lev Povalahev
All rights reserved.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
    
**OpenCL**

Copyright (c) 2008-2015 The Khronos Group Inc.

**RSA Data Security, Inc.**

Copyright (C) 1990, RSA Data Security, Inc. All rights reserved.
License to copy and use this software is granted provided that
it is identified as the "RSA Data Security, Inc. MD5 Message
Digest Algorithm" in all material mentioning or referencing this 
software or this function.
License is also granted to make and use derivative works
provided that such works are identified as "derived from the RSA
Data Security, Inc. MD5 Message Digest Algorithm" in all
material mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning
either the merchantability of this software or the suitability
of this software for any particular purpose.  It is provided "as
is" without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 
**Glslang**
Copyright (C) 2002-2005 3Dlabs Inc. Ltd.
Copyright (C) 2012-2013 LunarG, Inc.

**yaml-cpp**
Copyright (c) 2008-2015 Jesse Beder.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

**spdlog**
The MIT License (MIT)

Copyright (c) 2016 Gabi Melman.                                       

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

**LLVM**
University of Illinois/NCSA
Open Source License

Copyright (c) 2003-2017 University of Illinois at Urbana-Champaign.
All rights reserved.

Developed by:

    LLVM Team

    University of Illinois at Urbana-Champaign

    http://llvm.org

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal with
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimers.

    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimers in the
      documentation and/or other materials provided with the distribution.

    * Neither the names of the LLVM Team, University of Illinois at
      Urbana-Champaign, nor the names of its contributors may be used to
      endorse or promote products derived from this Software without specific
      prior written permission.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
SOFTWARE.
