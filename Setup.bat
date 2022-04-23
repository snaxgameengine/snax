@echo off
echo SnaX Game Engine ^- https://github.com/snaxgameengine/snax
echo Licensed under the MIT License ^<http://opensource.org/licenses/MIT^>.
echo SPDX-License-Identifier: MIT
echo Copyright (c) 2013 - 2022 Frank-Vegar Mortensen ^<franksvm(at)outlook(dot)com^>.
echo,
echo Permission is hereby  granted, free of charge, to any  person obtaining a copy
echo of this software and associated  documentation files (the "Software"), to deal
echo in the Software  without restriction, including without  limitation the rights
echo to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
echo copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
echo furnished to do so, subject to the following conditions:
echo,	
echo The above copyright notice and this permission notice shall be included in all
echo copies or substantial portions of the Software.
echo,
echo THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
echo IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
echo FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
echo AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
echo LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
echo OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
echo SOFTWARE.
echo,
echo,
echo Welcome to the SnaX Game Engine setup script!
echo =============================================
echo,
echo This script will:
echo 1. Init and update submodules, including VCPKG.
echo 2. Bootstrap VCPKG.
echo 3. Build dependencies using VCPKG, including Qt, for x64.
echo 4. Call GenerateSnaX.bat to generate SnaX projects using CMake.
echo,
set /P c=Do you want to continue (y/n)? 
if /I %c% NEQ y exit /B
if not exist Build\ (
  mkdir Build 
)
@echo on
git submodule init
git submodule update
cd vcpkg
call ./bootstrap-vcpkg.bat -disableMetrics
vcpkg install qtbase qttools qttools[designer] zlib libxml2 boost-crc boost-type-traits physx directxtk directxtk12 directxtex assimp sdl2 glm magic-enum rapidjson --triplet x64-windows
IF %ERRORLEVEL% NEQ 0 (
  echo,
  echo vcpkg install FAILED... Return code: %ERRORLEVEL%
  echo Carefully review the message log to find the cause.
  echo Spaces in directory names are known to cause an error.
  cd ..
) else (
  cd ..
  echo Congratulations! Setup script SUCCEEDED! 
  echo,
  echo VCPKG temporary files takes up a lot of disk space.
  echo Remove these directories if you are low on disk space:
  echo ./vcpkg/packages/
  echo ./vcpkg/buildtrees/
  echo ./vcpkg/downloads/
  echo,
  echo Let's proceed using CMake to generate the Visual Studio projects for SnaX...
  echo,
  call ./GenerateSnaX.bat
)


