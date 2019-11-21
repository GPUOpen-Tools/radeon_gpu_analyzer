@echo on
REM extract Vulkan RGA Layer files
cd %WORKSPACE%
powershell expand-archive -Path VKLayerRGA-2.1.*.zip -Destination %WORKSPACE%\
del /f VKLayerRGA*.zip

REM Build RGA
REM make sure cygwin not in path for windows cmake command
set PATH=C:\Python36;C:\Python36\Scripts;C:\Python36\Tools\Scripts;C:\OpenJDK\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;"C:\Program Files\doxygen\bin";C:\Strawberry\c\bin;C:\Strawberry\perl\site\bin;C:\Strawberry\perl\bin;"C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit\";"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE";"C:\Program Files (x86)\MSBuild\14.0\Bin";"C:\Program Files\CMake\bin";"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin";"C:\Program Files\Git\bin";

set RGA_BUILD_DATE=%DATE%

cd %WORKSPACE%
REM update the RadeonGPUAnalyzerGUI.rc file with current version
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --major`) do (
    set MAJOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --minor`) do (
    set MINOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --update`) do (
    set UPDATE=%%F
)
python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsSetRcVersion.py -M %MAJOR% -m %MINOR% -b %BUILD_NUMBER%
python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsSetBuildVersion.py -b %BUILD_NUMBER%

REM clone dependencies without the commit-msg hook
cd %WORKSPACE%\RGA\Build
python FetchDependencies.py --no-hooks

REM run the prebuild steps
call prebuild.bat --no-fetch --vs 2015 --qt C:\Qt\Qt5.9.2\5.9.2\msvc2015_64 --vk-include C:\VulkanSDK\1.1.97.0\Include --vk-lib C:\VulkanSDK\1.1.97.0\Lib --build release
if not %ERRORLEVEL%==0 (
    echo CMake failed to generate solution file!
    exit /b 1
)

@echo on
cd %WORKSPACE%
set RGAPATH=%WORKSPACE%\RGA
set BUILDPATH=%RGAPATH%\BuildOutput
set ZIPDIRRGA=%BUILDPATH%
mkdir %ZIPDIRRGA%
mkdir %ZIPDIRRGA%\bin
set ARCHIVE_NAME=RGA-%MAJOR%.%MINOR%.%UPDATE%
set OUTPUT_PATH=%WORKSPACE%\RGA\Output
set ARCH=x64
set BUILD=%BUILD_NUMBER%

REM build RGA artifacts
cd %WORKSPACE%\RGA\Build\Util\Windows
call .\JenkinsMSBuildRelease_gui.bat %WORKSPACE%\RGA\Build\Windows\VS2015\Release\RGA.sln x64 2>&1> %WORKSPACE%\RGA_MSBuild.log
if not %ERRORLEVEL%==0 (
    echo Build Failed!
    exit /b 1
)
@echo on

REM build RGALayerLauncher
cd %WORKSPACE%\RGA\Utils\Vulkan\Windows\RGALayer\
msbuild /nodeReuse:false /m:4 /t:Build /p:Configuration=Release RGALayerLauncher.sln

REM Build documentation
@echo on
cd %WORKSPACE%\RGA\Documentation

call .\make.bat clean
call .\make.bat html

cd Build
XCopy /i /e /y "html" "..\..\Output\bin\Release\Documentation\html"
