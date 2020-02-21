REM Create MSI package
cd %WORKSPACE%
@echo on
REM update the RGA-Installer.aip file with the current version information
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --major`) do (
    set MAJOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --minor`) do (
    set MINOR=%%F
)
for /F "tokens=* USEBACKQ" %%F in (`python %WORKSPACE%\RGA\Build\Util\Windows\JenkinsGetVersion.py --update`) do (
    set UPDATE=%%F
)
set BUILD=%BUILD_NUMBER%
set VERSION=%MAJOR%.%MINOR%.%UPDATE%.%BUILD%

cd %WORKSPACE%\RGA\Installer
"C:\Program Files (x86)\Caphyon\Advanced Installer 13.5\bin\x86\AdvancedInstaller.com" /edit %WORKSPACE%\RGA\Installer\RGA-Installer.aip /SetVersion %VERSION%
"C:\Program Files (x86)\Caphyon\Advanced Installer 13.5\bin\x86\AdvancedInstaller.com" /edit %WORKSPACE%\RGA\Installer\RGA-Installer.aip /SetProperty SETUP_FILE_NAME=RGA-%VERSION%-Installer.msi
"C:\Program Files (x86)\Caphyon\Advanced Installer 13.5\bin\x86\AdvancedInstaller.com" /edit %WORKSPACE%\RGA\Installer\RGA-Installer.aip /SetProperty ARPCOMMENTS=RGA-%VERSION%-Installer.msi
"C:\Program Files (x86)\Caphyon\Advanced Installer 13.5\bin\x86\AdvancedInstaller.com" /edit %WORKSPACE%\RGA\Installer\RGA-Installer.aip /SetProperty PackageFileName=RGA-%VERSION%-Installer.msi -buildname x64_Release_Static
"C:\Program Files (x86)\Caphyon\Advanced Installer 13.5\bin\x86\AdvancedInstaller.com" /edit %WORKSPACE%\RGA\Installer\RGA-Installer.aip /SetIcon -icon %WORKSPACE%\RGA\RadeonGPUAnalyzerGUI\res\icons\rgaIcon.ico

REM Create RGA msi installer file
"C:\Program Files (x86)\Caphyon\Advanced Installer 13.5\bin\x86\AdvancedInstaller.com" /build %WORKSPACE%\RGA\Installer\RGA-Installer.aip -buildslist x64_Release_Static

