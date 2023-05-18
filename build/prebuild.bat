@echo off
:: prebuild.bat --vs 2019 --qt <path_to_qt>

SETLOCAL enabledelayedexpansion

rem Print help message
if "%1"=="-h" goto :print_help
if "%1"=="-help" goto :print_help
if "%1"=="--h" goto :print_help
if "%1"=="--help" goto :print_help
if "%1"=="/?" goto :print_help

goto :start

:print_help
echo:
echo This script generates Visual Studio project and solution files for RGA on Windows.
echo:
echo Usage:  prebuild.bat ^[options^]
echo:
echo Options:
echo    --no-fetch           Do not call fetch_dependencies.py script before running cmake. The default is "false".
echo    --cmake              Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.
echo    --vs                 Microsoft Visual Studio version. Currently supported values are: "2015", "2017", "2019", and "2022".
echo    --qt                 Path to Qt5 root folder. The default is empty (cmake will look for Qt5 package istalled on the system).
echo    --cli-only           Build RGA command-line tool only (do not build GUI). The default is "false".
echo    --gui-only           Build GUI only (do not build RGA command-line tool). The default is "false".
echo    --no-vulkan          Build RGA without live Vulkan mode support. If this option is used, the Vulkan SDK is not required. The default is "false".
echo                         This option is only valid in conjunction with --cli-only.
echo    --vk-include         Path to the Vulkan SDK include folder.
echo    --vk-lib             Path to the Vulkan SDK library folder.
echo    --cppcheck           Add "-DCMAKE_CXX_CPPCHECK" to cmake command
echo    --clean              Delete the cmake build and output folders
echo:
echo Examples:
echo    prebuild.bat
echo    prebuild.bat --vs 2017 --qt C:\Qt\5.7\msvc2015_64
echo    prebuild.bat --vs 2015 --cli-only
echo    prebuild.bat --no-vulkan
goto :exit

:start
set SCRIPT_DIR=%~dp0
set CURRENT_DIR=%CD%

rem Default values
set CMAKE_PATH=cmake
set VS_VER=2022
set QT_ROOT=
set VK_INCLUDE=
set VK_LIB=
set CPPCHECK=
set NO_FETCH=FALSE

:begin
if [%1]==[] goto :start_cmake
if "%1"=="--cmake" goto :set_cmake
if "%1"=="--vs" goto :set_vs
if "%1"=="--qt" goto :set_qt
if "%1"=="--cli-only" goto :set_cli_only
if "%1"=="--gui-only" goto :set_gui_only
if "%1"=="--no-vulkan" goto :set_no_vulkan
if "%1"=="--vk-include" goto :set_vk_include
if "%1"=="--vk-lib" goto :set_vk_lib
if "%1"=="--no-fetch" goto :set_no_fetch
if "%1"=="--automation" goto :set_automation
if "%1"=="--internal" goto :set_internal
if "%1"=="--verbose" goto :set_verbose
if "%1"=="--cppcheck" goto :set_cppcheck
if "%1"=="--clean" goto :run_clean
goto :bad_arg

:set_cmake
set CMAKE_PATH=%2
goto :shift_2args

:set_vs
set VS_VER=%2
goto :shift_2args

:set_qt
set QT_ROOT=%2
goto :shift_2args

:set_vk_include
set VK_INCLUDE=%2
goto :shift_2args

:set_vk_lib
set VK_LIB=%2
goto :shift_2args

:set_cli_only
set CLI_ONLY=-DBUILD_CLI_ONLY^=ON
goto :shift_arg

:set_gui_only
set GUI_ONLY=-DBUILD_GUI_ONLY^=ON
goto :shift_arg

:set_no_vulkan
set NO_VULKAN=-DRGA_ENABLE_VULKAN^=OFF
goto :shift_arg

:set_no_fetch
set NO_FETCH=TRUE
goto :shift_arg

:set_automation
set AUTOMATION=-DGUI_AUTOMATION^=ON
set TEST_DIR_SUFFIX=_test
goto :shift_arg

:set_internal
set AMD_INTERNAL=-DAMD_INTERNAL^=ON
set INTERNAL=TRUE
goto :shift_arg

:set_verbose
@echo on
goto :shift_arg

:set_cppcheck
set CPPCHECK=-DCMAKE_CXX_CPPCHECK="C:\Program Files\Cppcheck\cppcheck.exe"
goto :shift_arg

:run_clean
if exist %SCRIPT_DIR%windows (
    echo INFO: Deleting %SCRIPT_DIR%\windows folder
    rmdir /s /q %SCRIPT_DIR%\windows
)
if exist %SCRIPT_DIR%..\output (
    echo INFO: Deleting %SCRIPT_DIR%..\output folder
    rmdir /s /q %SCRIPT_DIR%..\output
)
if exist %SCRIPT_DIR%..\output_test (
    echo INFO: Deleting %SCRIPT_DIR%..\output_test folder
    rmdir /s /q %SCRIPT_DIR%..\output_test
)
exit /b 0

:shift_2args
rem Shift to the next pair of arguments
shift
:shift_arg
shift
goto :begin

:bad_arg
echo Error: Unexpected argument: %1%. Aborting...
exit /b 1

:start_cmake
if not "!NO_VULKAN!"=="" (
    if "%CLI_ONLY%"=="" (
        echo ERROR: Invalid Syntax: must use --cli-only with --no-vulkan
        exit /b 1
    )
)

set CMAKE_VSARCH=
if "%VS_VER%"=="2015" (
    set CMAKE_VS="Visual Studio 14 2015 Win64"
) else (
    if "%VS_VER%"=="2017" (
        set CMAKE_VS="Visual Studio 15 2017 Win64"
    ) else (
        if "%VS_VER%"=="2019" (
            set CMAKE_VS="Visual Studio 16 2019"
            set CMAKE_VSARCH=-A x64
        ) else (
            if "%VS_VER%"=="2022" (
                set CMAKE_VS="Visual Studio 17 2022"
                set CMAKE_VSARCH=-A x64
            ) else (
                echo Error: Unknown VisualStudio version provided. Aborting...
                exit /b 1
            )
        )
    )
)

if not [%QT_ROOT%]==[] (
    set CMAKE_QT=-DQT_PACKAGE_ROOT^=%QT_ROOT% -DNO_DEFAULT_QT=ON
)

if not [%VK_INCLUDE%]==[] (
    set CMAKE_VK_INCLUDE=-DVULKAN_SDK_INC_DIR^=%VK_INCLUDE%
)

if not [%VK_LIB%]==[] (
    set CMAKE_VK_LIB=-DVULKAN_SDK_LIB_DIR^=%VK_LIB%
)

rem Create an output folder
set VS_FOLDER=vs%VS_VER%
set OUTPUT_FOLDER=%SCRIPT_DIR%windows\%VS_FOLDER%%TEST_DIR_SUFFIX%
if not exist %OUTPUT_FOLDER% (
    mkdir %OUTPUT_FOLDER%
)

rem clone or download dependencies
if "!NO_FETCH!" == "FALSE" (
    echo Updating Common...
    call python %SCRIPT_DIR%\fetch_dependencies.py
    if not !ERRORLEVEL!==0 (
        echo Error: encountered an error while fetching dependencies. Aborting...
        exit /b 1
    )
)

rem Invoke cmake with required arguments.
echo:
echo Running cmake to generate a VisualStudio solution...
cd %OUTPUT_FOLDER%
%CMAKE_PATH% -G %CMAKE_VS% %CMAKE_VSARCH% !CMAKE_QT! !CMAKE_VK_INCLUDE! !CMAKE_VK_LIB! %CLI_ONLY% %GUI_ONLY% %NO_VULKAN% %AUTOMATION% %AMD_INTERNAL% %CPPCHECK% ..\..\..
if not !ERRORLEVEL!==0 (
    echo "ERROR: cmake failed. Aborting..."
    exit /b 1
)
cd %CURRENT_DIR%
echo Done.

