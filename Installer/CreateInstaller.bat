@echo off
echo Welcome to SnaX Installer script!
echo,
echo ==============================================================
echo 1. Make sure you have Inno Setup version 6 or later installed!
echo 2. Remember to set correct version in main.cpp!
echo 3. Use Release build only!
echo 4. Update SnaXInstaller.iss with 
echo    correct path to current vc_redist.x64.exe.
echo ==============================================================
echo,
set /p build_path="Enter directory for build: "
if "%build_path%" == "" (
set build_path=..\Build\bin\Release
)
set /p build_version="Which version are you building (eg 1.7.0.0): "

C:\"Program Files (x86)\Inno Setup 6"\iscc "/dBuildDir=%build_path%" "/dMyAppVersion=%build_version%" SnaXInstaller.iss

echo Script finished!
pause