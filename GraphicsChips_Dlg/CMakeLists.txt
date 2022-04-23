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

# GraphicsChips_Dlg
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Gui CONFIG REQUIRED)
find_path(DIRECTXTEX_INCLUDE_DIR DirectXTex.h)

find_package(directxtex CONFIG REQUIRED)

file(GLOB_RECURSE GRAPHICSCHIPS_DLG_HEADER *.h)
file(GLOB_RECURSE GRAPHICSCHIPS_DLG_SOURCE *.cpp)
file(GLOB_RECURSE GRAPHICSCHIPS_DLG_UI *.ui)
file(GLOB_RECURSE GRAPHICSCHIPS_DLG_QRC *.qrc)

add_library(GraphicsChips_Dlg SHARED ${GRAPHICSCHIPS_DLG_SOURCE} ${GRAPHICSCHIPS_DLG_HEADER} ${GRAPHICSCHIPS_DLG_UI} ${GRAPHICSCHIPS_DLG_QRC})
set_target_properties(GraphicsChips_Dlg PROPERTIES COMPILE_FLAGS "/Yustdafx.h /FIstdafx.h")
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h /FI")
target_include_directories(GraphicsChips_Dlg PRIVATE ..)

#include_directories(${DIRECTXTEX_INCLUDE_DIR})

source_group("ui" FILES ${GRAPHICSCHIPS_DLG_UI})

target_link_libraries(GraphicsChips_Dlg M3DCore)
target_link_libraries(GraphicsChips_Dlg M3DEngine)
target_link_libraries(GraphicsChips_Dlg ChipDialogs)
target_link_libraries(GraphicsChips_Dlg GraphicsChips)
target_link_libraries(GraphicsChips_Dlg Qt6::Core)
target_link_libraries(GraphicsChips_Dlg Qt6::Widgets)
target_link_libraries(GraphicsChips_Dlg Qt6::Gui)
target_link_libraries(GraphicsChips_Dlg Microsoft::DirectXTex)

add_custom_command(
    TARGET GraphicsChips_Dlg 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:GraphicsChips_Dlg>
        ${SNAX_BUILD_DIALOGS_DIR}
)