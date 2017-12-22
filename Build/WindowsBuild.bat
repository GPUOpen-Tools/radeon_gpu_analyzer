REM RGA
REM "RGA_ROOT" is passed in to this build script from Jenkins
REM "SOLUTION_PATH" is passed in to this build script from Jenkins
REM "ARCH" is passed in to this build script from Jenkins (must be "x64" or "x86")
REM "ARCHIVE_NAME" is passed in to this build script from Jenkins
REM
REM Usage:
REM   cd RGA_ROOT\RGA\Build
REM   call WindowsBuild.bat RGA_ROOT <solution-path> <x86|x64> <archive-name>
REM
@echo on

set RGA_ROOT=%1
set SOLUTION_PATH=%2
set ARCH=%3
set ARCHIVE_NAME=%4
echo RGA_ROOT %RGA_ROOT% %SOLUTION_PATH% %ARCH% %ARCHIVE_NAME%

set CONFIG=Release
set RGAPATH="%RGA_ROOT%\RGA"
set BUILDPATH=%RGAPATH%\BuildOutput
set OUTPUT_PATH=%RGAPATH%\output

REM initialize zip file target for CodeXL bits
echo "Initialize folders for zip package"
set ZIPDIRRGA=%BUILDPATH%
MKDIR %ZIPDIRRGA%
MKDIR %ZIPDIRRGA%\bin

REM build release 
call "%VS140COMNTOOLS%\vsvars32.bat"
cd %RGAPATH%\Build
echo "Clean the build"
call JenkinsMSBuildClean.bat %SOLUTION_PATH% %ARCH% %CONFIG%
if not %ERRORLEVEL%==0 (
    echo CLEAN FAILED!
    exit 1
)
echo "Build Release"
call JenkinsMSBuildRelease.bat %SOLUTION_PATH% %ARCH%
if not %ERRORLEVEL%==0 (
    echo BUILD FAILED!
    exit 1
)
echo "Build Complete"

cd %WORKSPACE%
REM Copy release binaries to the zip file dir
XCopy "%OUTPUT_PATH%\%ARCH%\bin\%CONFIG%\rga.exe" "%ZIPDIRRGA%\bin\rga.exe*"
XCopy "%OUTPUT_PATH%\%ARCH%\bin\%CONFIG%\%ARCH%\VirtualContext.exe" "%ZIPDIRRGA%\bin\%ARCH%\VirtualContext.exe*"
XCopy "%OUTPUT_PATH%\%ARCH%\bin\%CONFIG%\%ARCH%\amdspv.exe" "%ZIPDIRRGA%\bin\%ARCH%\amdspv.exe*"
XCopy "%OUTPUT_PATH%\%ARCH%\bin\%CONFIG%\%ARCH%\spvgen.dll" "%ZIPDIRRGA%\bin\%ARCH%\spvgen.dll*"
XCopy "%OUTPUT_PATH%\%ARCH%\bin\%CONFIG%\%ARCH%\shae.exe" "%ZIPDIRRGA%\bin\%ARCH%\shae.exe*"
XCopy "%OUTPUT_PATH%\%ARCH%\bin\%CONFIG%\%ARCH%\RGADX11.exe" "%ZIPDIRRGA%\bin\%ARCH%\RGADX11.exe*"
XCopy /e "%OUTPUT_PATH%\%ARCH%\bin\%CONFIG%\%ARCH%\ROCm" "%ZIPDIRRGA%\bin\%ARCH%\ROCm\"

REM Copy the system d3d compiler
XCopy "C:\Program Files (x86)\Windows Kits\10\bin\%ARCH%\d3dcompiler_47.dll" "%ZIPDIRRGA%\bin\%ARCH%\d3dcompiler_47.dll*"

REM Copy the VC++ Redistributable package binaries
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\%ARCH%\Microsoft.VC140.CRT\concrt140.dll" "%ZIPDIRRGA%\bin\concrt140.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\%ARCH%\Microsoft.VC140.CRT\msvcp140.dll" "%ZIPDIRRGA%\bin\msvcp140.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\%ARCH%\Microsoft.VC140.CRT\vccorlib140.dll" "%ZIPDIRRGA%\bin\vccorlib140.dll*"
XCopy "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\redist\%ARCH%\Microsoft.VC140.CRT\vcruntime140.dll" "%ZIPDIRRGA%\bin\vcruntime140.dll*"

REM create zip package
cd %ZIPDIRRGA%
powershell compress-archive -DestinationPath %BUILDPATH%\%ARCHIVE_NAME%.%BUILD_NUMBER%.zip -Path .\bin
