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

# StdImporters_Dlg
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets Gui CONFIG REQUIRED)

file(GLOB_RECURSE STDIMPORTERS_DLG_HEADER *.h)
file(GLOB_RECURSE STDIMPORTERS_DLG_SOURCE *.cpp)
file(GLOB_RECURSE STDIMPORTERS_DLG_UI *.ui)
file(GLOB_RECURSE STDIMPORTERS_DLG_QRC *.qrc)

add_library(StdImporters_Dlg SHARED ${STDIMPORTERS_DLG_SOURCE} ${STDIMPORTERS_DLG_HEADER} ${STDIMPORTERS_DLG_UI} ${STDIMPORTERS_DLG_QRC})
set_target_properties(StdImporters_Dlg PROPERTIES COMPILE_FLAGS "/Yustdafx.h /FIstdafx.h")
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h /FI")
target_include_directories(StdImporters_Dlg PRIVATE ..)

source_group("ui" FILES ${STDIMPORTERS_DLG_UI})

target_link_libraries(StdImporters_Dlg M3DCore M3DEngine ChipDialogs StdImporters Qt6::Core Qt6::Widgets Qt6::Gui)

add_custom_command(
    TARGET StdImporters_Dlg 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:StdImporters_Dlg>
        ${SNAX_BUILD_DIALOGS_DIR}
)