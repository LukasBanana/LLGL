#
# CMakeLists.txt file for LLGL Testbed project
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Testbed)


# === Source files ===

# Test project files
find_project_source_files( FilesTestbedBase         "${TEST_PROJECTS_DIR}/Testbed"              )
find_project_source_files( FilesTestbedUnitTests    "${TEST_PROJECTS_DIR}/Testbed/UnitTests"    )

set(
    FilesTestbed
    ${FilesTestbedBase}
    ${FilesTestbedUnitTests}
)

if(APPLE)
    # Compile certain C++ files as objective-C++ to include Obj-C specific headers such as <Cocoa/Cocoa.h>
    set(
        FilesCppAsObjC
        "${TEST_PROJECTS_DIR}/Testbed/UnitTests/TestNativeHandle.cpp"
    )
    set_source_files_properties("${FilesCppAsObjC}" PROPERTIES COMPILE_FLAGS "-x objective-c++")
endif()


# === Source group folders ===

source_group("Testbed"              FILES ${FilesTestbedBase})
source_group("Testbed\\UnitTests"   FILES ${FilesTestbedUnitTests})


# === Include directories ===

include_directories("${TEST_PROJECTS_DIR}/Testbed")

# These include directories are needed for the NativeHandle test: <vulkan/vulkan.h>, <GL/wglext.h>
if(LLGL_BUILD_STATIC_LIB)
    if (LLGL_BUILD_RENDERER_VULKAN)
        find_package(Vulkan)
        if(Vulkan_FOUND)
            include_directories(${Vulkan_INCLUDE_DIR})
        endif()
        ADD_DEFINE(LLGL_TESTBED_INCLUDE_VULKAN)
    endif()

    if(LLGL_BUILD_RENDERER_OPENGL)
        find_package(OpenGL)
        if(OPENGL_FOUND)
            include_directories(${OPENGL_INCLUDE_DIR})
            if(WIN32)
                include_directories("${EXTERNAL_INCLUDE_DIR}/OpenGL/include")
            endif()
            ADD_DEFINE(LLGL_TESTBED_INCLUDE_OPENGL)
        endif()
    endif()
endif()


# === Projects ===

# Testbed project
if(LLGL_BUILD_TESTS)
    add_llgl_example_project(Testbed CXX "${FilesTestbed}" "${LLGL_MODULE_LIBS}")
    if(LLGL_BUILD_WRAPPER_C99)
        ADD_PROJECT_DEFINE(Testbed LLGL_TESTBED_INCLUDE_C99_TESTS)
    endif()
endif(LLGL_BUILD_TESTS)


