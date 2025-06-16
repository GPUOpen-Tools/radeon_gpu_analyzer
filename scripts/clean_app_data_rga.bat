REM Copyright (c) 2017-2025 Advanced Micro Devices, Inc. All rights reserved.

SET RGA_DATA_FOLDER="%APPDATA%\RadeonGPUAnalyzer"
if EXIST "%RGA_DATA_FOLDER%\RGAConfig_2_3.xml" (
    del /f /q "%RGA_DATA_FOLDER%\RGAConfig_2_3.xml"
)
if exist "%RGA_DATA_FOLDER%\*.log" (
    del /f /q "%RGA_DATA_FOLDER%\*.log"
)
if exist "%RGA_DATA_FOLDER%\VersionInfo.xml" (
    del /f /q "%RGA_DATA_FOLDER%\VersionInfo.xml"
)

