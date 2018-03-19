@echo off

rem Make all variables defined in this script local.
SETLOCAL

set OUTPUT_DIR=%1

rem Parse arguments
rem if (%2 == "-debug" || %3 == "-debug)
IF "%2"=="-debug" goto :set_debug
IF "%3"=="-debug" goto :set_debug
goto :no_debug

:set_debug
set DEBUG=TRUE

:no_debug

IF "%2"=="-internal" goto :set_internal
IF "%3"=="-internal" goto :set_internal
goto :no_internal

:set_internal
set INTERNAL=TRUE

:no_internal

rem Create the output folders:
IF NOT exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%
IF NOT exist %OUTPUT_DIR%\x64 mkdir %OUTPUT_DIR%\x64

XCopy /r /d /y "..\..\Core\OpenGL\VirtualContext\Release\win64\VirtualContext.exe" "%OUTPUT_DIR%\x64\"
XCopy /r /d /y "..\..\Core\ShaderAnalysis\Windows\x86\shae.exe" "%OUTPUT_DIR%\x64\"
XCopy /r /d /y "..\..\Core\DX\DX10\bin\RGADX11.exe" "%OUTPUT_DIR%\x64\"
XCopy /r /e /d /y "..\..\Core\ROCm\OpenCL\win64" "%OUTPUT_DIR%\x64\ROCm\OpenCL\"
XCopy /r /d /y "..\..\License.txt" "%OUTPUT_DIR%\"

IF DEFINED INTERNAL (    
    ECHO Copying internal vulkan backend
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win64\amdspv.exe" "%OUTPUT_DIR%\x64\"
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win64\spvgen.dll" "%OUTPUT_DIR%\x64\"
) ELSE (
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win64\amdspv.exe" "%OUTPUT_DIR%\x64\"
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win64\spvgen.dll" "%OUTPUT_DIR%\x64\"
)
