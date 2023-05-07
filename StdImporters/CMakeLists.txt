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

# StdImporters
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

find_package(directxtex CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

file(GLOB_RECURSE STDIMPORTERS_HEADER *.h)
file(GLOB_RECURSE STDIMPORTERS_SOURCE *.cpp)

add_library(StdImporters SHARED ${STDIMPORTERS_SOURCE} ${STDIMPORTERS_HEADER})
set_target_properties(StdImporters PROPERTIES COMPILE_FLAGS "/Yustdafx.h")
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
target_include_directories(StdImporters PRIVATE ..)

target_link_libraries(StdImporters M3DCore M3DEngine StdChips GraphicsChips Microsoft::DirectXTex assimp::assimp)

get_target_property(assimp_dll_location assimp::assimp LOCATION)
get_filename_component(assimp_dll_name ${assimp_dll_location} NAME)

add_custom_command(
    TARGET StdImporters 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:StdImporters>
        ${SNAX_BUILD_CHIPS_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE_DIR:StdImporters>/${assimp_dll_name}
        ${SNAX_BUILD_3RD_DIR}
)