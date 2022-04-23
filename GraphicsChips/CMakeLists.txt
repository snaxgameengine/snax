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

# GraphicsChips
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

find_path(DIRECTXTK12_INCLUDE_DIR DirectXTK12/ScreenGrab.h)

find_package(directxtk12 CONFIG REQUIRED)

file(GLOB_RECURSE GRAPHICSCHIPS_HEADER *.h)
file(GLOB_RECURSE GRAPHICSCHIPS_SOURCE *.cpp)

add_library(GraphicsChips SHARED ${GRAPHICSCHIPS_SOURCE} ${GRAPHICSCHIPS_HEADER})
set_target_properties(GraphicsChips PROPERTIES COMPILE_FLAGS "/Yupch.h")
set_source_files_properties(pch.cpp PROPERTIES COMPILE_FLAGS "/Ycpch.h")
target_include_directories(GraphicsChips PRIVATE ..)

target_link_libraries(GraphicsChips Primitives M3DCore M3DEngine StdChips HBAOPlus)

target_link_libraries(GraphicsChips  Microsoft::DirectXTK12)
target_link_libraries(GraphicsChips "dxgi.lib")
target_link_libraries(GraphicsChips "d3d12.lib")
target_link_libraries(GraphicsChips "D3DCompiler.lib")
target_link_libraries(GraphicsChips "dxguid.lib")

add_custom_command(
    TARGET GraphicsChips 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:GraphicsChips>
        ${SNAX_BUILD_CHIPS_DIR}
)
