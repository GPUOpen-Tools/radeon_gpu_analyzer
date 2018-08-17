REM Clean specified RGA project or solution configuration
REM Usage:
REM   OUTPUT_PATH defined by parent script, WindowsBuild.bat
REM   cd <workspace>\RGA\Build
REM   .\JenkinsMSBuildClean.bat <solution-path> <platform> <configuration>

set SOLUTION_PATH=%1
set ARCH=%2
set CONFIG=%3

msbuild /m:6 /t:Clean /p:Configuration=%CONFIG% /p:Platform=%ARCH% /p:OutputPath=%OUTPUT_PATH% %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Clean of solution %SOLUTION_PATH% failed!
    exit 1
)

