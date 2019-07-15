REM Debug Builds
REM Usage:
REM   cd <workspace>\RGA\Build
REM   .\JenkinsMSBuildDebug.bat <solution-path> <platform>
REM       <platform>    x86 or x64

set SOLUTION_PATH=%1
set ARCH=%2

msbuild /nodeReuse:false /m:4 /t:Build /p:Configuration=Debug /p:Platform=%ARCH%  /p:OutputPath= ..\output %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Build of Debug %ARCH% for RGA.sln failed
    exit 1
)
