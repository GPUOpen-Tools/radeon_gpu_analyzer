REM Release Builds
REM Usage:
REM   .\JenkinsMSBuildRelease.bat <platform>
REM      <platform>        x86 or x64

set ARCH=%1

msbuild /m:6 /t:Build /p:Configuration=Release /p:Platform=%ARCH% /p:OutputPath=..\output .\CMake\VS2015\RGA.sln
if not %ERRORLEVEL%==0 (
    echo Build of Release %ARCH% failed for RGA.sln
    exit 1
)

