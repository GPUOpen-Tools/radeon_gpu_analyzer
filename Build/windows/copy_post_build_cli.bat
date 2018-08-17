@echo off

rem Make all variables defined in this script local.
SETLOCAL

set OUTPUT_DIR=%1
set ARCH=x64

rem Parse arguments
rem if (%2 == "-debug" || %3 == "-debug)
IF "%2"=="-debug" goto :set_debug
IF "%3"=="-debug" goto :set_debug
goto :no_debug

:set_debug
set DEBUG=TRUE

:no_debug

IF "%2"=="-x86" goto :set_32bit
IF "%3"=="-x86" goto :set_32bit
goto :cont

:set_32bit
set ARCH=x86

:cont

rem Create the output folders:
IF NOT exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%
IF NOT exist %OUTPUT_DIR%\%ARCH% mkdir %OUTPUT_DIR%\%ARCH%

IF "%ARCH%"=="x86" (
    XCopy /r /d /y "..\..\Core\OpenGL\VirtualContext\Release\win32\VirtualContext.exe" "%OUTPUT_DIR%\%ARCH%\"
) ELSE (
    XCopy /r /d /y "..\..\Core\OpenGL\VirtualContext\Release\win64\VirtualContext.exe" "%OUTPUT_DIR%\%ARCH%\"
)
XCopy /r /d /y "..\..\Core\ShaderAnalysis\Windows\x86\shae.exe" "%OUTPUT_DIR%\%ARCH%\"
XCopy /r /d /y "..\..\Core\DX\DX10\bin\RGADX11.exe" "%OUTPUT_DIR%\%ARCH%\"
XCopy /r /e /d /y "..\..\Core\ROCm\OpenCL\win64" "%OUTPUT_DIR%\%ARCH%\ROCm\OpenCL\"
XCopy /r /d /y "..\..\Core\ROCm\OpenCL\additional-targets" "%OUTPUT_DIR%\%ARCH%\ROCm\OpenCL\"
XCopy /r /d /y "..\..\License.txt" "%OUTPUT_DIR%\"
XCopy /r /d /y "..\..\RGAThirdPartyLicenses.txt" "%OUTPUT_DIR%\"
XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win64\amdspv.exe" "%OUTPUT_DIR%\%ARCH%\"
XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win64\spvgen.dll" "%OUTPUT_DIR%\%ARCH%\"
