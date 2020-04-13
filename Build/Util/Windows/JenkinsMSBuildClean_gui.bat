REM Clean all configurations of RGA builds
REM Usage:
REM   .\JenkinsMSBuildClean.bat <solution-path>

set SOLUTION_PATH=%1

msbuild /nodeReuse:false /m:4 /t:Clean /p:Configuration=Debug /p:Platform=x86 %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Clean of Debug x86 failed for RGA.sln
)
msbuild /nodeReuse:false /m:4 /t:Clean /p:Configuration=Debug /p:Platform=x64 %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Clean of Debug x64 failed for RGA.sln
)
msbuild /nodeReuse:false /m:4 /t:Clean /p:Configuration=Release /p:Platform=x86 %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Clean of Release x86 failed for RGA.sln
)
msbuild /nodeReuse:false /m:4 /t:Clean /p:Configuration=Release /p:Platform=x64 %SOLUTION_PATH%
if not %ERRORLEVEL%==0 (
    echo Clean of Release x64 failed for RGA.sln
)
