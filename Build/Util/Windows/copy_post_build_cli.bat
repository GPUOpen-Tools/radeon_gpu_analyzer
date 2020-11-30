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

REM Get version information to use in package name.
for /F "tokens=* USEBACKQ" %%F in (`python ..\..\..\Build\Util\get_version.py --major`) do (
    set MAJOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python ..\..\..\Build\Util\get_version.py --minor`) do (
    set MINOR=%%F
)

rem Create the output folders:
IF NOT exist %OUTPUT_DIR%\ mkdir %OUTPUT_DIR%\
IF NOT exist %OUTPUT_DIR%\utils\ mkdir %OUTPUT_DIR%\utils\
IF NOT exist %OUTPUT_DIR%\utils\DX12\ mkdir %OUTPUT_DIR%\utils\DX12\
IF NOT exist %OUTPUT_DIR%\utils\DX12\DXC\ mkdir %OUTPUT_DIR%\utils\DX12\DXC\
IF NOT exist %OUTPUT_DIR%\utils\LC\ mkdir %OUTPUT_DIR%\utils\LC\
IF NOT exist %OUTPUT_DIR%\utils\LC\Disassembler\ mkdir %OUTPUT_DIR%\utils\LC\Disassembler\
IF NOT exist %OUTPUT_DIR%\utils\LC\OpenCL\ mkdir %OUTPUT_DIR%\utils\LC\OpenCL\
echo

XCopy /r /d /y "..\..\..\Core\OpenGL\VirtualContext\Release\win64\VirtualContext.exe" "%OUTPUT_DIR%\utils\"
XCopy /r /d /y "..\..\..\Core\ShaderAnalysis\Windows\x64\shae.exe" "%OUTPUT_DIR%\utils\"
XCopy /r /d /y "..\..\..\Core\DX\DX10\bin\RGADX11.exe" "%OUTPUT_DIR%\utils\"
XCopy /r /e /d /y "..\..\..\Core\LC\OpenCL\win64\*" "%OUTPUT_DIR%\utils\LC\OpenCL\"
XCopy /r /d /y "..\..\..\Core\LC\OpenCL\additional-targets" "%OUTPUT_DIR%\utils\LC\OpenCL\"
XCopy /r /d /y "..\..\..\Core\LC\Disassembler\Windows\amdgpu-dis.exe" "%OUTPUT_DIR%\utils\LC\Disassembler\"
XCopy /r /e /d /y "..\..\..\Core\Vulkan\tools\Win64\bin" "%OUTPUT_DIR%\utils\Vulkan\"
XCopy /r /e /d /y "..\..\..\Core\DX12\DXC\*" "%OUTPUT_DIR%\utils\DX12\DXC\"
XCopy /r /d /y "..\..\..\License.txt" "%OUTPUT_DIR%\License.txt*"
XCopy /r /d /y "..\..\..\RGAThirdPartyLicenses.txt" "%OUTPUT_DIR%\RGAThirdPartyLicenses.txt*"
XCopy /r /d /y "..\..\..\README.md" "%OUTPUT_DIR%\README.md*"
XCopy /r /d /y "..\..\..\Documentation\releases\%MAJOR%.%MINOR%\RGA_RELEASE_NOTES_v%MAJOR%.%MINOR%.txt" "%OUTPUT_DIR%\RGA_RELEASE_NOTES_v%MAJOR%.%MINOR%.txt*"
XCopy /r /d /y "..\..\..\..\Common\Src\UpdateCheckAPI\rtda\windows\rtda.exe" "%OUTPUT_DIR%\rtda.exe*"

XCopy /r /d /y "..\..\..\Core\VulkanOffline\win64\amdspv.exe" "%OUTPUT_DIR%\utils\"
XCopy /r /d /y "..\..\..\Core\VulkanOffline\win64\spvgen.dll" "%OUTPUT_DIR%\utils\"
