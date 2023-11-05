#
# CMakeLists.txt file for LLGL C# project
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

# Originally authored on 10/24/2023

cmake_minimum_required(VERSION 3.7)

project(SharpLLGL CSharp)


# === Source files ===

# C# wrapper files
find_source_files(FilesWrapperCsharp "*.cs" ${PROJECT_SOURCE_DIR})


# === Source group folders ===

source_group("Wrapper\\CSharp" FILES ${FilesWrapperCsharp})


# === Projects ===

add_library(SharpLLGL SHARED ${FilesWrapperCsharp})
set_target_properties(SharpLLGL PROPERTIES FOLDER "LLGL (CSharp)")
target_compile_options(SharpLLGL PUBLIC "/unsafe")

