@echo off
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
echo Usage:  gen_sln.bat ^[--cmake ^<cmake_path^>^] ^[--build ^<build_type^>^] ^[--vs ^<vs_version^>^]
echo:
echo Options:
echo    --cmake        Path to cmake executable to use. If not specified, the cmake from PATH env variable will be used.
echo    --build        The build type: "release" or "debug". The default is "debug".
echo    --vs           Microsoft Visual Studio verson. Currently supported values are: "2015", "2017". The default is "2015".
echo    --no-update    Do not call UpdateCommon.py script before running cmake.
echo:
echo Examples:
echo    gen_sln.bat
echo    gen_sln.bat --build release
echo    gen_sln.bat --vs 2017
echo    gen_sln.bat --vs 2015 --build debug
goto :exit

:start
set SCRIPT_DIR=%~dp0
set CURRENT_DIR=%CD%

rem Default values
set CMAKE_PATH=cmake
set BUILD_TYPE=Debug
set VS_VER=2015

:begin
if [%1]==[] goto :start_cmake
if "%1"=="--cmake" goto :set_cmake
if "%1"=="--build" goto :set_build
if "%1"=="--vs" goto :set_vs
if "%1"=="--no-update" goto :set_no_update
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
:set_no_update
set NO_UPDATE=TRUE
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
    set CMAKE_VS="Visual Studio 14 2015 Win64"
) else (
    if "%VS_VER%"=="2017" (
        set CMAKE_VS="Visual Studio 15 2017 Win64"
    ) else (
        echo Error: Unknows VisualStudio version provided. Aborting...
        exit /b
    )
)

rem Create an output folder
set VS_FOLDER=VS%VS_VER%
if not exist %SCRIPT_DIR%CMake\%VS_FOLDER% (
    mkdir %SCRIPT_DIR%CMake\%VS_FOLDER%
)

rem Call UpdateCommon.
if not "%NO_UPDATE%"=="TRUE" (
    echo:
    echo Updating Common...
    python %SCRIPT_DIR%..\UpdateCommon.py
    if exist python %SCRIPT_DIR%..\UpdateCommon_PreRelease.py (
        python %SCRIPT_DIR%..\UpdateCommon_PreRelease.py
    )
)

rem Invoke cmake with required arguments.
echo:
echo Running cmake to generate a VisualStudio solution...
cd %SCRIPT_DIR%CMake\%VS_FOLDER%
%CMAKE_PATH% -G %CMAKE_VS% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ..\..\..
cd %CURRENT_DIR%
echo Done.

:exit
