@echo off

set CONFIG_NAME=%1
set INTERNAL=%2

rem Create the x86 subfolder:
if not exist ..\..\Build\Output\%CONFIG_NAME%\bin\x86 mkdir ..\..\Build\Output\%CONFIG_NAME%\bin\x86

rem Copy core files:
XCopy /r /d /y "..\..\Core\OpenGL\VirtualContext\Release\win32\VirtualContext.exe" "..\..\Build\Output\%CONFIG_NAME%\bin\x86\"
XCopy /r /d /y "..\..\Core\ShaderAnalysis\Windows\x86\shae.exe" "..\..\Build\Output\%CONFIG_NAME%\bin\x86\"

IF "%INTERNAL%"=="1" (
    ECHO Copying internal vulkan backend
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win32\amdspv.exe" "..\..\Build\Output\%CONFIG_NAME%\bin\x86\"
    XCopy /r /d /y "..\..\..\RGA-Internal\Core\Vulkan\win32\spvgen.dll" "..\..\Build\Output\%CONFIG_NAME%\bin\x86\"
)ELSE (
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win32\amdspv.exe" "..\..\Build\Output\%CONFIG_NAME%\bin\x86\"
    XCopy /r /d /y "..\..\Core\Vulkan\rev_1_0_0\Release\win32\spvgen.dll" "..\..\Build\Output\%CONFIG_NAME%\bin\x86\"
)




