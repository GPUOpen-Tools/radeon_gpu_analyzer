REM Create RGA zip packages
@echo on
REM Prep for zip file creation
REM WORKSPACE and BUILD_NUMBER set by Jenkins
REM this scripts assumes that AMDVLK64.DLL has been downloaded into the correct location:
REM RGA/Output/bin/Release/x64/Vulkan/bin/amdvlk/
REM Usage: call RGA\Build\Util\Windows\jenkins_rga_pkg.bat [automation]
REM

REM check for optional arguments "automation" and "debug"
set BUILD_TYPE=Release
if "%1" == "debug" set BUILD_TYPE=Debug
if "%2" == "debug" set BUILD_TYPE=Debug

set OUTPUT_PATH=%WORKSPACE%\RGA\Output\%BUILD_TYPE%
set COPY_TEST_SCRIPT=No
if "%1" == "automation" (
    set OUTPUT_PATH=%WORKSPACE%\RGA\Output_Test\%BUILD_TYPE%
    set COPY_TEST_SCRIPT=Yes
)
if "%2" == "automation" (
    set OUTPUT_PATH=%WORKSPACE%\RGA\Output_Test\%BUILD_TYPE%
    set COPY_TEST_SCRIPT=Yes
)

set RGAPATH=%WORKSPACE%\RGA
set BUILDPATH=%RGAPATH%\BuildOutput
set ZIPDIRRGA=%BUILDPATH%
if not exist %ZIPDIRRGA% mkdir %ZIPDIRRGA%
if not exist %ZIPDIRRGA%\bin mkdir %ZIPDIRRGA%\bin

cd %WORKSPACE%
REM Get version information to use in package name.
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --major`) do (
    set MAJOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --minor`) do (
    set MINOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --update`) do (
    set UPDATE=%%F
)
set ARCHIVE_NAME=RGA-%MAJOR%.%MINOR%.%UPDATE%

REM define some variables
REM This script assumes it is being used to generate a Release Candidate package using a Release build config
set ARCH=x64
set BUILD=%BUILD_NUMBER%

REM Copy files from build output or source files into package folder to be used by powershell (compress-archive)
REM or Advanced Installer
cd %WORKSPACE%
REM GUI files
XCopy "%OUTPUT_PATH%\bin\RadeonGPUAnalyzerGUI.exe" "%BUILDPATH%\bin\RadeonGPUAnalyzerGUI.exe*"
XCopy "%OUTPUT_PATH%\bin\Qt5*.dll" "%BUILDPATH%\bin\"
XCopy "%OUTPUT_PATH%\bin\iconengines\*.dll" "%BUILDPATH%\bin\iconengines\"
XCopy "%OUTPUT_PATH%\bin\platforms\*.dll" "%BUILDPATH%\bin\platforms\"
XCopy "%OUTPUT_PATH%\bin\imageformats\*.dll" "%BUILDPATH%\bin\imageformats\"
REM CLI  and core files
XCopy "%OUTPUT_PATH%\bin\rga.exe" "%ZIPDIRRGA%\bin\rga.exe*"
XCopy "%OUTPUT_PATH%\bin\AMDToolsDownloader.exe" "%ZIPDIRRGA%\bin\AMDToolsDownloader.exe*"
XCopy "%OUTPUT_PATH%\bin\x64\VirtualContext.exe" "%ZIPDIRRGA%\bin\x64\VirtualContext.exe*"
XCopy "%OUTPUT_PATH%\bin\x64\amdspv.exe" "%ZIPDIRRGA%\bin\x64\amdspv.exe*"
XCopy "%OUTPUT_PATH%\bin\x64\spvgen.dll" "%ZIPDIRRGA%\bin\x64\spvgen.dll*"
XCopy "%OUTPUT_PATH%\bin\x64\shae.exe" "%ZIPDIRRGA%\bin\x64\shae.exe*"
XCopy "%OUTPUT_PATH%\bin\x64\RGADX11.exe" "%ZIPDIRRGA%\bin\x64\RGADX11.exe*"
XCopy /y /e "%OUTPUT_PATH%\bin\x64\LC" "%ZIPDIRRGA%\bin\x64\LC\"
XCopy /y /e "%OUTPUT_PATH%\bin\x64\Vulkan" "%ZIPDIRRGA%\bin\x64\Vulkan\"
XCopy /y /e "%OUTPUT_PATH%\bin\x64\DX12" "%ZIPDIRRGA%\bin\x64\DX12\"
XCopy /i /e "%OUTPUT_PATH%\bin\Documentation" "%ZIPDIRRGA%\bin\Documentation"
XCopy "%RGAPATH%\License.txt" "%ZIPDIRRGA%\bin\"
XCopy "%RGAPATH%\RGAThirdPartyLicenses.txt" "%ZIPDIRRGA%\bin\"
XCopy "%RGAPATH%\EULA.txt" "%ZIPDIRRGA%\bin\"
XCopy "C:\Program Files (x86)\Windows Kits\10\bin\x64\d3dcompiler_47.dll" "%ZIPDIRRGA%\bin\x64\d3dcompiler_47.dll*"

REM Copy the Vulkan RGA Layer files
for /f "tokens=*" %%i in ('dir /b /d VKLayerRGA*') do set VKLAYERDIR=%%i
if not exist %ZIPDIRRGA%\bin\layer mkdir %ZIPDIRRGA%\bin\layer
XCopy "%WORKSPACE%\%VKLAYERDIR%\*.dll" "%ZIPDIRRGA%\bin\layer\"
XCopy "%WORKSPACE%\%VKLAYERDIR%\*.json" "%ZIPDIRRGA%\bin\layer\"
XCopy "%WORKSPACE%\RGA\Utils\Vulkan\Windows\RGALayer\RGALayerLauncher\bin\%BUILD_TYPE%\*.exe" "%ZIPDIRRGA%\bin"

REM Copy the VC++ Redistributable package binaries
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x64\Microsoft.VC140.CRT\concrt140.dll" "%ZIPDIRRGA%\bin\concrt140.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x64\Microsoft.VC140.CRT\msvcp140.dll" "%ZIPDIRRGA%\bin\msvcp140.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x64\Microsoft.VC140.CRT\vccorlib140.dll" "%ZIPDIRRGA%\bin\vccorlib140.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\x64\Microsoft.VC140.CRT\vcruntime140.dll" "%ZIPDIRRGA%\bin\vcruntime140.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x64\Microsoft.VC120.CRT\msvcp120.dll" "%ZIPDIRRGA%\bin\x64\Vulkan\bin\msvcp120.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x64\Microsoft.VC120.CRT\msvcr120.dll" "%ZIPDIRRGA%\bin\x64\Vulkan\bin\msvcr120.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\redist\x64\Microsoft.VC120.CRT\vccorlib120.dll" "%ZIPDIRRGA%\bin\x64\Vulkan\bin\vccorlib120.dll*"

REM Copy the test files if the user specified "automation"
if "%COPY_TEST_SCRIPT%" == "Yes" (
    XCopy /i /e "%WORKSPACE%\RGA-Internal\Tests\data" "%OUTPUT_PATH%\bin\data"
    XCopy /i /e "%WORKSPACE%\RGA-Internal\Tests\data" "%ZIPDIRRGA%\bin\data"
    XCopy "%WORKSPACE%\RGA-Internal\Tests-GUI\run.py" "%ZIPDIRRGA%\bin\"
    XCopy "%WORKSPACE%\RGA-Internal\Tests-GUI\run.py" "%OUTPUT_PATH%\bin\"
)

REM Create RGA zip file
cd %ZIPDIRRGA%\bin
powershell compress-archive -Path * -DestinationPath %BUILDPATH%\%ARCHIVE_NAME%.%BUILD%.zip
