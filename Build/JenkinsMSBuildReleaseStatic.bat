REM Release Builds
REM Usage:
REM   .\JenkinsMSBuildReleaseStatic.bat <platform>
REM      <platform>        x86 or x64

set ARCH=%1

msbuild /m:6 /t:Build /p:Configuration=Release_Static /p:Platform=%ARCH% /p:OutputPath=..\output .\VS2015\RadeonGPUAnalyzer.sln
