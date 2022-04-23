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

# M3DEngine
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

find_package(LibXml2 CONFIG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(SDL2 CONFIG REQUIRED)


file(GLOB_RECURSE M3DENGINE_HEADER *.h)
file(GLOB_RECURSE M3DENGINE_SOURCE *.cpp)

add_library(M3DEngine SHARED ${M3DENGINE_SOURCE} ${M3DENGINE_HEADER})
set_target_properties(M3DEngine PROPERTIES COMPILE_FLAGS "/Yupch.h")
set_source_files_properties(pch.cpp PROPERTIES COMPILE_FLAGS "/Ycpch.h")
target_include_directories(M3DEngine PRIVATE ..)

target_link_libraries(M3DEngine PUBLIC M3DCore LibXml2::LibXml2 ZLIB::ZLIB PUBLIC "dinput8.lib" "dxguid.lib" SDL2::SDL2)

add_custom_command(
    TARGET M3DEngine 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:M3DEngine> ${SNAX_BUILD_MAIN_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:M3DEngine>/libxml2.dll ${SNAX_BUILD_MAIN_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:M3DEngine>/zlib$<$<CONFIG:Debug>:d>1.dll ${SNAX_BUILD_MAIN_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:M3DEngine>/iconv-2.dll ${SNAX_BUILD_MAIN_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:M3DEngine>/lzma$<$<CONFIG:Debug>:d>.dll ${SNAX_BUILD_MAIN_DIR}/
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:M3DEngine>/SDL2$<$<CONFIG:Debug>:d>.dll ${SNAX_BUILD_MAIN_DIR}/
)
