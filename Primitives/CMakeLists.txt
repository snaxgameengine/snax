# Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# Primitives
cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
cmake_policy(VERSION 3.15)

file(GLOB_RECURSE PRIMITIVES_HEADER *.h)
file(GLOB_RECURSE PRIMITIVES_SOURCE *.cpp)

add_library(Primitives SHARED ${PRIMITIVES_SOURCE} ${PRIMITIVES_HEADER})
set_target_properties(Primitives PROPERTIES COMPILE_FLAGS "/Yustdafx.h")
set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h")
target_include_directories(Primitives PRIVATE ..)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
add_compile_definitions(GENERATOR_USE_GLM)

include_directories("../generator/include")

target_link_libraries(Primitives PRIVATE generator)

add_custom_command(
    TARGET Primitives 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:Primitives>
        ${SNAX_BUILD_3RD_DIR}
)
