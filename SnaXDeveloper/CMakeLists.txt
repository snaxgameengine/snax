# SnaX Game Engine - https://github.com/snaxgameengine/snax
# Licensed under the MIT License <http://opensource.org/licenses/MIT>.
# SPDX-License-Identifier: MIT
# Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
#
# Permission is hereby  granted, free of charge, to any  person obtaining a copy
# of this software and associated  documentation files (the "Software"), to deal
# in the Software  without restriction, including without  limitation the rights
# to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
# copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
# IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
# FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
# AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
# LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# SnaXDeveloper
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Gui CONFIG REQUIRED)

find_package(directxtk CONFIG REQUIRED)

file(GLOB_RECURSE M3DSTUDIO_HEADER *.h)
file(GLOB_RECURSE M3DSTUDIO_SOURCE *.cpp)
file(GLOB_RECURSE M3DSTUDIO_UI *.ui)
file(GLOB_RECURSE M3DSTUDIO_QRC *.qrc)

add_executable(SnaXDeveloper WIN32 ${M3DSTUDIO_SOURCE} ${M3DSTUDIO_HEADER} ${M3DSTUDIO_UI} ${M3DSTUDIO_QRC} Resources.rc ../msvc.manifest)
set_target_properties(SnaXDeveloper PROPERTIES COMPILE_FLAGS "/Yustdafx.h /FIstdafx.h")
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h /FI")
target_include_directories(SnaXDeveloper PRIVATE ..)

source_group("ui" FILES ${M3DSTUDIO_UI})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

target_link_libraries(SnaXDeveloper M3DCore M3DEngine M3DEngineExt ChipDialogs)
target_link_libraries(SnaXDeveloper Qt6::Core Qt6::Widgets Qt6::Gui)
target_link_libraries(SnaXDeveloper Microsoft::DirectXTK)
target_link_libraries(SnaXDeveloper "d3d11.lib")
target_link_libraries(SnaXDeveloper "dxguid.lib")
target_link_libraries(SnaXDeveloper "dxgi.lib")
target_link_libraries(SnaXDeveloper "dxgi.lib")

add_dependencies(SnaXDeveloper StdChips)
add_dependencies(SnaXDeveloper StdChips_Dlg)
add_dependencies(SnaXDeveloper Primitives)
add_dependencies(SnaXDeveloper GraphicsChips)
add_dependencies(SnaXDeveloper GraphicsChips_Dlg)
add_dependencies(SnaXDeveloper StdImporters)
add_dependencies(SnaXDeveloper StdImporters_Dlg)
add_dependencies(SnaXDeveloper SnaXViewer)
add_dependencies(SnaXDeveloper PhysXChips)
add_dependencies(SnaXDeveloper PhysXChips_Dlg)

add_custom_command(
    TARGET SnaXDeveloper 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:SnaXDeveloper> ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:SnaXDeveloper>/brotlicommon.dll ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:SnaXDeveloper>/brotlidec.dll ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:SnaXDeveloper>/bz2$<$<CONFIG:Debug>:d>.dll ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:SnaXDeveloper>/freetype$<$<CONFIG:Debug>:d>.dll ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:SnaXDeveloper>/harfbuzz.dll ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:SnaXDeveloper>/pcre2-16$<$<CONFIG:Debug>:d>.dll ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE_DIR:SnaXDeveloper>/libpng16$<$<CONFIG:Debug>:d>.dll ${SNAX_BUILD_MAIN_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SNAX_BUILD_MAIN_DIR}/Publish
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Common/Publish/SFXNoConfig_x64.sfx ${SNAX_BUILD_MAIN_DIR}/Publish
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Common/Publish/x64/7zG.exe         ${SNAX_BUILD_MAIN_DIR}/Publish
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/../Common/Publish/x64/7z.dll          ${SNAX_BUILD_MAIN_DIR}/Publish
	COMMAND ${Qt6_DIR}/../../tools/Qt6/bin/windeployqt.$<IF:$<CONFIG:Debug>,debug.bat,exe> ${SNAX_BUILD_MAIN_DIR} $<IF:$<CONFIG:Debug>,--debug,--release> --no-translations --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime
)

if(CMAKE_CONFIGURATION_TYPES)
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
     if(NOT EXISTS ${CMAKE_BINARY_DIR}/bin/${config}/settings.ini)
         configure_file(settings.ini.in ${CMAKE_BINARY_DIR}/bin/${config}/settings.ini)
	 endif()
   endforeach(config)
else(CMAKE_CONFIGURATION_TYPES)

endif(CMAKE_CONFIGURATION_TYPES)



