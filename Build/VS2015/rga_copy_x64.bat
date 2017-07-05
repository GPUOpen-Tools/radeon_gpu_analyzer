@echo off

set OUTPUT_DIR=%1

rem Create the x64 subfolder:
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%

rem Copy core files:
XCopy /r /d /y "..\..\Core\OpenGL\VirtualContext\Release\win64\VirtualContext.exe" "%OUTPUT_DIR%\x64\"
XCopy /r /d /y "..\..\Core\ShaderAnalysis\Windows\x86\shae.exe" "%OUTPUT_DIR%\x64\"

IF "%2"=="-internal" (
    ECHO Copying internal vulkan backend
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win64\amdspv.exe" "%OUTPUT_DIR%\x64\"
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win64\spvgen.dll" "%OUTPUT_DIR%\x64\"
)ELSE  (
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win64\amdspv.exe" "%OUTPUT_DIR%\x64\"
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win64\spvgen.dll" "%OUTPUT_DIR%\x64\"
)
