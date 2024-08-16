#
# CMakeLists.txt file for LLGL example projects
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Examples)


# Example Projects
if(LLGL_BUILD_EXAMPLES)

    add_subdirectory(Cpp)
    
    if(LLGL_BUILD_WRAPPER_C99)
        add_subdirectory(C99)
    endif(LLGL_BUILD_WRAPPER_C99)
    
    if(LLGL_BUILD_WRAPPER_CSHARP)
        add_subdirectory(CSharp)
    endif(LLGL_BUILD_WRAPPER_CSHARP)
    
endif(LLGL_BUILD_EXAMPLES)


