@echo off

set OUTPUT_DIR=%1

rem Create the x86 subfolder:
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

rem Copy core files:
XCopy /r /d /y "..\..\Core\OpenGL\VirtualContext\Release\win32\VirtualContext.exe" "%OUTPUT_DIR%\x86\"
XCopy /r /d /y "..\..\Core\ShaderAnalysis\Windows\x86\shae.exe" "%OUTPUT_DIR%\x86\"
XCopy /r /d /y "..\..\Core\DX\DX10\bin\RGADX11.exe" "%OUTPUT_DIR%\x64\"

IF "%2"=="-internal" (
    ECHO Copying internal vulkan backend
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win32\amdspv.exe" "%OUTPUT_DIR%\x86\"
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win32\spvgen.dll" "%OUTPUT_DIR%\x86\"
)ELSE (
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win32\amdspv.exe" "%OUTPUT_DIR%\x86\"
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win32\spvgen.dll" "%OUTPUT_DIR%\x86\"
)
