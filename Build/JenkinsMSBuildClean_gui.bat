REM Clean all configurations of RGA builds
REM Usage:
REM   .\JenkinsMSBuildClean.bat

msbuild /m:6 /t:Clean /p:Configuration=Debug /p:Platform=x86 .\CMake\VS2015\RGA.sln
if not %ERRORLEVEL%==0 (
    echo Clean of Debug x86 failed for RGA.sln
)
msbuild /m:6 /t:Clean /p:Configuration=Debug /p:Platform=x64 .\CMake\VS2015\RGA.sln
if not %ERRORLEVEL%==0 (
    echo Clean of Debug x64 failed for RGA.sln
)
msbuild /m:6 /t:Clean /p:Configuration=Release /p:Platform=x86 .\CMake\VS2015\RGA.sln
if not %ERRORLEVEL%==0 (
    echo Clean of Release x86 failed for RGA.sln
)
msbuild /m:6 /t:Clean /p:Configuration=Release /p:Platform=x64 .\CMake\VS2015\RGA.sln
if not %ERRORLEVEL%==0 (
    echo Clean of Release x64 failed for RGA.sln
)
