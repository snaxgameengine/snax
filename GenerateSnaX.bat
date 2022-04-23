@echo off
echo,
echo,
echo Welcome to the SnaX Game Engine generator script!
echo =================================================
echo,
echo This script will use CMake to generate, and optionally
echo build, Visual Studio projects and solution for SnaX.
echo,

set /P c=Do you want to continue (y/n)? 
if /I %c% NEQ y exit /B

:selgen
echo,
echo Which generator would you like to use:
echo 1. Visual Studio 16 2019
echo 2. Visual Studio 17 2022
set /P p=Select preset (1-2): 

if %p% == 1 (
  set p="SnaXVS2019"
  set q="VS2019"
) else (
  if %p% == 2 (
    set p="SnaXVS2022"
    set q="VS2022"
  ) else (
      goto :selgen
  )
)

echo,
set /P g=Would you like to use CMake GUI for configuration (y/n)? 
if /I %g% EQU n goto :nongui

echo,
echo Starting cmake-gui...
start "" cmake-gui -S . --preset=%p%
exit /B

:nongui

cmake -S . --preset=%p%
IF %ERRORLEVEL% NEQ 0 (
  echo,
  echo Generating SnaX projects failed... cmake returned code: %ERRORLEVEL%
) else (
  echo,
  echo SnaX projects for Visual Studio are now generated!
  echo We can build SnaX from command line right now, 
  echo or you can cancel the script here, and proceed with
  echo development in Visual Studio!
  call ./BuildSnaX.bat ./Build/
)


