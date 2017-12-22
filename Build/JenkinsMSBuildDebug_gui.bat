REM Debug Builds
REM Usage:
REM   cd <workspace>\RGA\Build
REM   .\JenkinsMSBuildDebug.bat <platform>
REM       <platform>    x86 or x64

set ARCH=%1
msbuild /m:6 /t:Build /p:Configuration=Debug /p:Platform=%ARCH%  /p:OutputPath= ..\output .\CMake\VS2015\RGA.sln
if not %ERRORLEVEL%==0 (
    echo Build of Debug %ARCH% for RGA.sln failed
    exit 1
)
