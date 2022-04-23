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

# StdChips_Dlg
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(LibXml2 CONFIG REQUIRED)
find_package(Qt6 COMPONENTS Core Widgets Gui CONFIG REQUIRED)

file(GLOB_RECURSE STDCHIPS_DLG_HEADER *.h)
file(GLOB_RECURSE STDCHIPS_DLG_SOURCE *.cpp)
file(GLOB_RECURSE STDCHIPS_DLG_UI *.ui)
file(GLOB_RECURSE STDCHIPS_DLG_QRC *.qrc)

add_library(StdChips_Dlg SHARED ${STDCHIPS_DLG_SOURCE} ${STDCHIPS_DLG_HEADER} ${STDCHIPS_DLG_UI} ${STDCHIPS_DLG_QRC})
set_target_properties(StdChips_Dlg PROPERTIES COMPILE_FLAGS "/Yustdafx.h /FIstdafx.h")
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h /FI")
target_include_directories(StdChips_Dlg PRIVATE ..)


source_group("ui" FILES ${STDCHIPS_DLG_UI})

target_link_libraries(StdChips_Dlg PUBLIC M3DCore M3DEngine ChipDialogs StdChips Qt6::Core Qt6::Widgets Qt6::Gui)

add_custom_command(
    TARGET StdChips_Dlg 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:StdChips_Dlg>
        ${SNAX_BUILD_DIALOGS_DIR}
)
