REM batch script for use by Jenkins build jobs.
REM Usage:
REM  call jenkins_build_rga.bat <vs-version> [automation] [nofetch]
REM
REM Arguments:
REM  <vs-version>  REQUIRED - Specifies the version of Visual Studio to use.  Currently supported versions: 2015 and 2017
REM  automation    Optional - If supplied, sets build flags to build the automated GUI tests.
REM  nofetch       Optional - If supplied, script with skip unzipping and cloning dependencies
@echo on

REM First argument must be the VS version to use.
set VS_VERSION=%1
if "x%VS_VERSION%x" == "xx" (
    echo ERROR: Must specify a Visual Studio version: 2015 or 2017
    exit /b 1
)
if not "%VS_VERSION%" == "2017" (
    if not "%VS_VERSION%" == "2015" (
        echo ERROR: Unsupported Visual Studio version %VS_VERSION%
        echo USAGE: "call jenkins_build_rga.bat <vs-version> [automation] [nofetch]"
        exit /b 1
    )
)

REM Default is to unzip and clone dependencies.
set FETCH_DEPENDENCIES="Yes"

REM Default paths and flags
set OUTPUT_PATH=%WORKSPACE%\RGA\Output
set VS_FOLDER_NAME=VS%VS_VERSION%
set PREBUILD_AUTOMATION=""

REM Check to see if automated GUI test build requested.
if "%2" == "automation" (
    set OUTPUT_PATH=%WORKSPACE%\RGA\Output_Test
    set VS_FOLDER_NAME=VS%VS_VERSION%_Test
    set PREBUILD_AUTOMATION=--automation
)
if "%3" == "automation" (
    set OUTPUT_PATH=%WORKSPACE%\RGA\Output_Test
    set VS_FOLDER_NAME=VS%VS_VERSION%_Test
    set PREBUILD_AUTOMATION=--automation
)

REM Check to see if nofetch requested.
if "%2" == "nofetch" (
    set FETCH_DEPENDENCIES="No"
)
if "%3" == "nofetch" (
    set FETCH_DEPENDENCIES="No"
)

REM extract Vulkan RGA Layer files
cd %WORKSPACE%
if %FETCH_DEPENDENCIES% == "Yes" (
    powershell expand-archive -Path VKLayerRGA-2.1.*.zip -Destination %WORKSPACE%\
    del /f VKLayerRGA*.zip
)

REM PATH setup
set PATH=C:\Python36;C:\Python36\Scripts;C:\Python36\Tools\Scripts;C:\OpenJDK\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;C:\WINDOWS\System32\WindowsPowerShell\v1.0\;"C:\Program Files\doxygen\bin";C:\Strawberry\c\bin;C:\Strawberry\perl\site\bin;C:\Strawberry\perl\bin;"C:\Program Files (x86)\Windows Kits\10\Windows Performance Toolkit\";"C:\Program Files\CMake\bin";"C:\Program Files\Git\bin"
if "%VS_VERSION%" == "2015" (
    REM VS 2015 paths
    set PATH=%PATH%;"C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\IDE";"C:\Program Files (x86)\MSBuild\14.0\Bin";"C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin";"C:\Program Files\Git\bin";
)
else if "%VS_VERSION%" == "2017" (
    REM VS 2017 paths
    set PATH=%PATH%;"C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\Common7\IDE";"C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin";"C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Tools\MSVC\14.16.27023\bin";
)

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
python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsSetRcVersion.py -M %MAJOR% -m %MINOR% -b %BUILD_NUMBER% -r %UPDATE%
python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsSetBuildVersion.py -b %BUILD_NUMBER%

REM clone dependencies without the commit-msg hook
cd %WORKSPACE%\RGA\Build
if %FETCH_DEPENDENCIES% == "Yes" (
    python FetchDependencies.py --no-hooks
)

REM run the prebuild steps
if %PREBUILD_AUTOMATION% == "" (
    call prebuild.bat --no-fetch --vs %VS_VERSION% --qt C:\Qt\Qt5.9.2\5.9.2\msvc%VS_VERSION%_64 --vk-include C:\VulkanSDK\1.1.97.0\Include --vk-lib C:\VulkanSDK\1.1.97.0\Lib
) else (
    call prebuild.bat --no-fetch --vs %VS_VERSION% --qt C:\Qt\Qt5.9.2\5.9.2\msvc%VS_VERSION%_64 --vk-include C:\VulkanSDK\1.1.97.0\Include --vk-lib C:\VulkanSDK\1.1.97.0\Lib %PREBUILD_AUTOMATION%
)
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
set ARCH=x64
set BUILD=%BUILD_NUMBER%

REM build RGA CLI and GUI
cd %WORKSPACE%\RGA\Build\Util\Windows
call .\JenkinsMSBuildRelease_gui.bat %WORKSPACE%\RGA\Build\Windows\%VS_FOLDER_NAME%\RGA.sln x64 2>&1> %WORKSPACE%\RGA_MSBuild.log
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

cd build
XCopy /i /e /y "html" "%OUTPUT_PATH%\Release\bin\Documentation\html"
