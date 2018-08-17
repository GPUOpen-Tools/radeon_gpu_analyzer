@echo off
:: gen_sln.bat --build release --vs 2017 --qt <path_to_qt>
SETLOCAL

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
echo Usage:  gen_sln.bat ^[--cmake ^<cmake_path^>^] ^[--build ^<build_type^>^] ^[--vs ^<vs_version^>^] ^[--qt ^<qt5_root^>^] ^[--cli-only^] ^[--gui-only^]
echo:
echo Options:
echo    --cmake        Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.
echo    --build        The build type: "release" or "debug". The default is "debug".
echo    --vs           Microsoft Visual Studio verson. Currently supported values are: "2015", "2017". The default is "2015".
echo    --qt           Path to Qt5 root folder. The default is empty (cmake will look for Qt5 package istalled on the system).
echo    --cli-only     Build RGA command-line tool only (do not build GUI).
echo    --gui-only     Build GUI only (do not build RGA command-line tool).
echo    --no-fetch     Do not call FetchDependencies.py script before running cmake.
echo:
echo Examples:
echo    gen_sln.bat
echo    gen_sln.bat --build release
echo    gen_sln.bat --vs 2017 --qt C:\Qt\5.7\msvc2015_64
echo    gen_sln.bat --vs 2015 --cli-only --build debug
goto :exit

:start
set SCRIPT_DIR=%~dp0
set CURRENT_DIR=%CD%

rem Default values
set CMAKE_PATH=cmake
set BUILD_TYPE=Debug
set VS_VER=2015
set QT_ROOT=

:begin
if [%1]==[] goto :start_cmake
if "%1"=="--cmake" goto :set_cmake
if "%1"=="--build" goto :set_build
if "%1"=="--vs" goto :set_vs
if "%1"=="--qt" goto :set_qt
if "%1"=="--cli-only" goto :set_cli_only
if "%1"=="--gui-only" goto :set_gui_only
if "%1"=="--no-fetch" goto :set_no_update
if "%1"=="--x86" goto :set_32bit
goto :bad_arg

:set_cmake
set CMAKE_PATH=%2
goto :shift_2args
:set_build
set BUILD_TYPE=%2
if "%BUILD_TYPE%"=="release" set BUILD_TYPE=Release
if "%BUILD_TYPE%"=="debug" set BUILD_TYPE=Debug
goto :shift_2args
:set_vs
set VS_VER=%2
goto :shift_2args
:set_qt
set QT_ROOT=%2
goto :shift_2args
:set_cli_only
set CLI_ONLY=-DBUILD_CLI_ONLY^=ON
goto :shift_arg
:set_gui_only
set GUI_ONLY=-DBUILD_GUI_ONLY^=ON
goto :shift_arg
:set_no_update
set NO_UPDATE=TRUE
goto :shift_arg
:set_32bit
set BUILD_32BIT=TRUE
goto :shift_arg

:shift_2args
rem Shift to the next pair of arguments
shift
:shift_arg
shift
goto :begin

:bad_arg
echo Error: Unexpected argument: %1%. Aborting...
exit /b

:start_cmake

if "%VS_VER%"=="2015" (
    if "%BUILD_32BIT%"=="TRUE" (
        set CMAKE_VS="Visual Studio 14 2015"
    ) else (
        set CMAKE_VS="Visual Studio 14 2015 Win64"
    )
) else (
    if "%VS_VER%"=="2017" (
        if "%BUILD_32BIT%"=="TRUE" (
            set CMAKE_VS="Visual Studio 15 2017"
        ) else (
            set CMAKE_VS="Visual Studio 15 2017 Win64"
        )
    ) else (
        echo Error: Unknows VisualStudio version provided. Aborting...
        exit /b
    )
)

if not [%QT_ROOT%]==[] (
    set CMAKE_QT=-DQT_PACKAGE_ROOT^=%QT_ROOT% -DNO_DEFAULT_QT=ON
)

rem Create an output folder
set VS_FOLDER=VS%VS_VER%
if not exist %SCRIPT_DIR%CMake\%VS_FOLDER%\%BUILD_TYPE% (
    mkdir %SCRIPT_DIR%CMake\%VS_FOLDER%\%BUILD_TYPE%
)

rem clone or download dependencies
if not "%NO_UPDATE%"=="TRUE" (
    echo:
    echo Updating Common...
    python %SCRIPT_DIR%\FetchDependencies.py
    echo ErrorLevel %ERRORLEVEL%
)

rem Invoke cmake with required arguments.
echo:
echo Running cmake to generate a VisualStudio solution...
cd %SCRIPT_DIR%CMake\%VS_FOLDER%\%BUILD_TYPE%
%CMAKE_PATH% -G %CMAKE_VS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %CMAKE_QT% %CLI_ONLY% %GUI_ONLY% ..\..\..\..
cd %CURRENT_DIR%
echo Done.

:exit
