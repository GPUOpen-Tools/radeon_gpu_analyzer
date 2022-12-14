@echo off

REM Make all variables defined in this script local.
SETLOCAL
SETLOCAL ENABLEEXTENSIONS

set OUTPUT_DIR=%1

REM Parse arguments
REM if (%2 == "-debug" || %3 == "-debug)
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

REM Get version information to use in package name.
for /F "tokens=* USEBACKQ" %%F in (`python ..\..\..\build\util\get_version.py --major`) do (
    set MAJOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python ..\..\..\build\util\get_version.py --minor`) do (
    set MINOR=%%F
)

REM Create the output folders
IF NOT exist %OUTPUT_DIR%\ mkdir %OUTPUT_DIR%\
IF NOT exist %OUTPUT_DIR%\utils\ mkdir %OUTPUT_DIR%\utils\
IF NOT exist %OUTPUT_DIR%\utils\dx12\ mkdir %OUTPUT_DIR%\utils\dx12\
IF NOT exist %OUTPUT_DIR%\utils\dx12\dxc\ mkdir %OUTPUT_DIR%\utils\dx12\dxc\
IF NOT exist %OUTPUT_DIR%\utils\dx11\ mkdir %OUTPUT_DIR%\utils\dx11\
IF NOT exist %OUTPUT_DIR%\utils\lc\ mkdir %OUTPUT_DIR%\utils\lc\
IF NOT exist %OUTPUT_DIR%\utils\lc\disassembler\ mkdir %OUTPUT_DIR%\utils\lc\disassembler\
IF NOT exist %OUTPUT_DIR%\utils\lc\opencl\ mkdir %OUTPUT_DIR%\utils\lc\opencl\lib\clang\16.0.0\include\
IF NOT exist %OUTPUT_DIR%\utils\vulkan mkdir %OUTPUT_DIR%\utils\vulkan\
echo

XCopy /r /d /y "..\..\..\external\opengl\glc\windows\glc.exe" "%OUTPUT_DIR%\utils\"
XCopy /r /d /y "..\..\..\source\utils\shader_analysis\windows\x64\shae.exe" "%OUTPUT_DIR%\utils\"
XCopy /r /d /y "..\..\..\source\utils\dx11\bin\dx11_adapter.exe" "%OUTPUT_DIR%\utils\"
XCopy /r /e /d /y "..\..\..\external\lc\opencl\windows\*" "%OUTPUT_DIR%\utils\lc\opencl\"
del /f "%OUTPUT_DIR%\utils\lc\opencl\include\opencl-c-base.h"
XCopy /r /d /y "..\..\..\external\lc\opencl\windows\include\opencl-c-base.h" "%OUTPUT_DIR%\utils\lc\opencl\lib\clang\16.0.0\include\"
XCopy /r /d /y "..\..\..\external\lc\opencl\additional-targets" "%OUTPUT_DIR%\utils\lc\opencl\"
XCopy /r /d /y "..\..\..\external\lc\disassembler\windows\amdgpu-dis.exe" "%OUTPUT_DIR%\utils\lc\disassembler\"
XCopy /r /e /d /y "..\..\..\external\vulkan\tools\windows\bin\*.exe" "%OUTPUT_DIR%\utils\vulkan\"
XCopy /r /e /d /y "..\..\..\external\dxc\*" "%OUTPUT_DIR%\utils\dx12\dxc\"
XCopy /r /e /d /y "..\..\..\external\dx12\withdll.exe" "%OUTPUT_DIR%\utils\dx12\"
XCopy /r /e /d /y "..\..\..\external\dx12\umdrepoint.dll" "%OUTPUT_DIR%\utils\dx12\"
XCopy /r /e /d /y "..\..\..\external\dx11\amdxx\amdxx64.dll" "%OUTPUT_DIR%\utils\dx11\"
XCopy /r /d /y "..\..\..\License.txt" "%OUTPUT_DIR%\License.txt*"
XCopy /r /d /y "..\..\..\RGA_NOTICES.txt" "%OUTPUT_DIR%\RGA_NOTICES.txt*"
XCopy /r /d /y "..\..\..\README.md" "%OUTPUT_DIR%\README.md*"
XCopy /r /d /y "..\..\..\documentation\RGA_RELEASE_NOTES.txt" "%OUTPUT_DIR%\RGA_RELEASE_NOTES.txt*"
XCopy /r /d /y "..\..\..\..\Common\Src\update_check_api\rtda\windows\rtda.exe" "%OUTPUT_DIR%\rtda.exe*"

XCopy /r /d /y "..\..\..\external\vulkan_offline\windows\amdspv.exe" "%OUTPUT_DIR%\utils\"
