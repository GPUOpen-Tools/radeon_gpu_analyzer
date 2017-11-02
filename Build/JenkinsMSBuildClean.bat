REM Clean all configurations of RGA builds
REM Usage:
REM   .\JenkinsMSBuildClean.bat

msbuild /m:6 /t:Clean /p:Configuration=Debug_Static /p:Platform=x86 .\VS2015\RadeonGPUAnalyzer.sln
msbuild /m:6 /t:Clean /p:Configuration=Debug_Static /p:Platform=x64 .\VS2015\RadeonGPUAnalyzer.sln
msbuild /m:6 /t:Clean /p:Configuration=Release_Static /p:Platform=x86 .\VS2015\RadeonGPUAnalyzer.sln
msbuild /m:6 /t:Clean /p:Configuration=Release_Static /p:Platform=x64 .\VS2015\RadeonGPUAnalyzer.sln
