REM Release Builds
REM Usage:
REM   .\JenkinsMSBuildRelease.bat <solution-path> <platform>
REM      <platform>        x86 or x64

set SOLUTION_PATH=%1
set ARCH=%2

msbuild /m:6 /t:Build /p:Configuration=Release /p:Platform=%ARCH% /p:OutputPath=..\output %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Build of Release %ARCH% failed for RGA.sln
    exit 1
)

