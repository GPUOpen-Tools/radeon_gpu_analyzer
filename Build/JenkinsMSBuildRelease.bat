REM Release Builds
REM Usage:
REM   OUTPUT_PATH set by WindowsBuild.bat
REM   cd <workspace>\RGA\Build
REM   .\JenkinsMSBuildRelease.bat <solution-path> <platform>
REM      <platform>        x86 or x64

set SOLUTION_PATH=%1
set ARCH=%2

msbuild /m:6 /t:Rebuild /p:Configuration=Release /p:Platform=%ARCH% /p:OutputPath=%OUTPUT_PATH% %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Release build of solution %SOLUTION_PATH% failed!
    exit 1

)
