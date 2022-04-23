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

#PhysXChips
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

# PhysX does not have a find_package setup therefore we have to find it manually
include(SelectLibraryConfigurations)

SET(CMAKE_FIND_LIBRARY_SUFFIXES ".lib" ".dll")

find_path(PHYSX_INCLUDE_DIR PxPhysics.h)

find_library(PHYSX_LIBRARY_RELEASE PhysX_64.lib NAMES PHYSX PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(PHYSX_LIBRARY_DEBUG PhysX_64.lib NAMES PHYSX PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PHYSX)

find_library(PHYSXCOMMON_LIBRARY_RELEASE PhysXCommon_64.lib NAMES PHYSXCOMMON PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(PHYSXCOMMON_LIBRARY_DEBUG PhysXCommon_64.lib NAMES PHYSXCOMMON PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PHYSXCOMMON)

find_library(PHYSXCOOKING_LIBRARY_RELEASE PhysXCooking_64.lib NAMES PHYSXCOOKING PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(PHYSXCOOKING_LIBRARY_DEBUG PhysXCooking_64.lib NAMES PHYSXCOOKING PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PHYSXCOOKING)

find_library(PHYSXFOUNDATION_LIBRARY_RELEASE PhysXFoundation_64.lib NAMES PHYSXFOUNDATION PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(PHYSXFOUNDATION_LIBRARY_DEBUG PhysXFoundation_64.lib NAMES PHYSXFOUNDATION PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PHYSXFOUNDATION)

find_library(PHYSXEXTENSIONS_LIBRARY_RELEASE PhysXExtensions_static_64.lib NAMES PHYSXEXTENSIONS PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(PHYSXEXTENSIONS_LIBRARY_DEBUG PhysXExtensions_static_64.lib NAMES PHYSXEXTENSIONS PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PHYSXEXTENSIONS)

find_library(PHYSXPVDSDK_LIBRARY_RELEASE PhysXPvdSDK_static_64.lib NAMES PHYSXPVDSDK PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib" NO_DEFAULT_PATH REQUIRED)
find_library(PHYSXPVDSDK_LIBRARY_DEBUG PhysXPvdSDK_static_64.lib NAMES PHYSXPVDSDK PATHS "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib" NO_DEFAULT_PATH REQUIRED)
select_library_configurations(PHYSXPVDSDK)


file(GLOB_RECURSE PHYSXCHIPS_SOURCE *.cpp)
file(GLOB_RECURSE PHYSXCHIPS_HEADER *.h)

add_library(PhysXChips SHARED ${PHYSXCHIPS_SOURCE} ${PHYSXCHIPS_HEADER})
set_target_properties(PhysXChips PROPERTIES COMPILE_FLAGS "/Yupch.h")
set_source_files_properties(pch.cpp PROPERTIES COMPILE_FLAGS "/Ycpch.h")
target_include_directories(PhysXChips PRIVATE .. PUBLIC ${PHYSX_INCLUDE_DIR})

target_link_libraries(PhysXChips M3DCore)
target_link_libraries(PhysXChips M3DEngine)
target_link_libraries(PhysXChips StdChips)
target_link_libraries(PhysXChips ${PHYSX_LIBRARY})
target_link_libraries(PhysXChips ${PHYSXCOMMON_LIBRARY})
target_link_libraries(PhysXChips ${PHYSXCOOKING_LIBRARY})
target_link_libraries(PhysXChips ${PHYSXFOUNDATION_LIBRARY})
target_link_libraries(PhysXChips ${PHYSXEXTENSIONS_LIBRARY})
target_link_libraries(PhysXChips ${PHYSXPVDSDK_LIBRARY})

add_custom_command(
    TARGET PhysXChips 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:PhysXChips> ${SNAX_BUILD_CHIPS_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:PhysXChips>/PhysX_64.dll            ${SNAX_BUILD_3RD_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:PhysXChips>/PhysXCommon_64.dll      ${SNAX_BUILD_3RD_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:PhysXChips>/PhysXCooking_64.dll     ${SNAX_BUILD_3RD_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE_DIR:PhysXChips>/PhysXFoundation_64.dll  ${SNAX_BUILD_3RD_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXDevice64.dll  ${SNAX_BUILD_3RD_DIR}
	COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/bin/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXGpu_64.dll    ${SNAX_BUILD_3RD_DIR}
)
