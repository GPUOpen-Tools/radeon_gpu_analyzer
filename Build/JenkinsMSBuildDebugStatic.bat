REM Debug Builds
REM Usage:
REM   cd <workspace>\RGA\Build
REM   .\JenkinsMSBuildDebugStatic.bat <platform>
REM       <platform>    x86 or x64

set ARCH=%1
msbuild /m:6 /t:Build /p:Configuration=Debug_Static /p:Platform=%ARCH% /p:OutputPath=..\output .\VS2015\RadeonGPUAnalyzer.sln
