@echo off
echo,
echo,
echo Welcome to the SnaX Game Engine build script!
echo =============================================
echo,
echo This script will use CMake to initiate SnaX build.
echo,

set /P c=Do you want to continue (y/n)? 
if /I %c% NEQ y exit /B

:selgen
echo,
echo Which configuration would you like to build?
echo 1. Debug
echo 2. Release
set /P p=Select option (1-2): 

if %p% == 1 (
  set p=Debug
) else (
  if %p% == 2 (
    set p=Release
  ) else (
    goto :selgen
  )
)

set t=%1
if "%t%" NEQ "" goto :buildselected

echo,
echo Where is the SnaX.sln you want to build (Press enter for default './Build/')?
set /P t=Select path: 

:buildselected

if "%t%" == "" (
  set t=.\Build
)

echo,
echo Starting build process...
echo cmake --build %t% --config %p%
cmake --build %t% --config %p%
echo,
if %ERRORLEVEL% NEQ 0 (
  echo Build failed... cmake returned code: %ERRORLEVEL%
  exit /B
) 

echo Build SUCCEEDED!
echo,
set /P la=Do you want to launch SnaX Developer now (y/n)? 
if /I %la% NEQ y exit /B
echo,
echo Starting Snax Developer...
start "" %t%\bin\%p%\SnaxDeveloper.exe


