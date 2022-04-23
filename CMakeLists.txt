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

cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

message("Current Configuration: " $ENV{BUILD_CONFIG})

SET( SNAX_BUILD_DIR_BASE ${CMAKE_BINARY_DIR}/bin/$<CONFIG>/)
SET( SNAX_BUILD_DIR ${SNAX_BUILD_DIR_BASE})

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SUBSYSTEM:WINDOWS")

SET( SNAX_BUILD_MAIN_DIR "${SNAX_BUILD_DIR}")
SET( SNAX_BUILD_CHIPS_DIR "${SNAX_BUILD_DIR}/Chips/")
SET( SNAX_BUILD_DIALOGS_DIR "${SNAX_BUILD_DIR}/Dialogs/")
SET( SNAX_BUILD_3RD_DIR "${SNAX_BUILD_DIR}/3rd/")

# We do not use a unicode build, but relies on UTF-8 for everything!
# add_definitions(-D_UNICODE -DUNICODE)

# Multithreaded build!
add_definitions(/MP)

project(SnaX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 /permissive-")

add_subdirectory(M3DCore)
add_subdirectory(M3DEngine)
add_subdirectory(M3DEngineExt)
add_subdirectory(ChipDialogs)
add_subdirectory(StdChips)
add_subdirectory(StdChips_Dlg)
add_subdirectory(Generator)
add_subdirectory(Primitives)
add_subdirectory(HBAOPlus)
add_subdirectory(GraphicsChips)
add_subdirectory(GraphicsChips_Dlg)
add_subdirectory(StdImporters)
add_subdirectory(StdImporters_Dlg)
add_subdirectory(PhysXChips)
add_subdirectory(PhysXChips_Dlg)
add_subdirectory(SnaXViewer)
add_subdirectory(SnaXDeveloper)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT SnaXDeveloper)
set_property(TARGET SnaXDeveloper PROPERTY VS_DEBUGGER_COMMAND ${SNAX_BUILD_DIR}/SnaXDeveloper.exe)
set_property(TARGET SnaXDeveloper PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${SNAX_BUILD_DIR})

set_property(TARGET SnaXViewer PROPERTY VS_DEBUGGER_COMMAND ${SNAX_BUILD_DIR}/SnaXViewer.exe)
set_property(TARGET SnaXViewer PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${SNAX_BUILD_DIR})