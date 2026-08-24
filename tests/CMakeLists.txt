#
# CMakeLists.txt file for LLGL test projects
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Tests)


# === Source files ===

find_project_source_files( FilesTest_Display    "${TEST_PROJECTS_DIR}/Test_Display.cpp" )
find_project_source_files( FilesTest_Image      "${TEST_PROJECTS_DIR}/Test_Image.cpp"   )
find_project_source_files( FilesTest_Window     "${TEST_PROJECTS_DIR}/Test_Window.cpp"  )


# === Include directories ===

include_directories("${EXAMPLE_CPP_PROJECTS_DIR}/ExampleBase")
include_directories("${EXTERNAL_INCLUDE_DIR}") # <stb/stb_image.h>

if(GaussLib_INCLUDE_DIR)
    include_directories(${GaussLib_INCLUDE_DIR})
elseif(LLGL_BUILD_TESTS)
    message(FATAL_ERROR "LLGL_BUILD_TESTS failed: missing 'GaussLib_INCLUDE_DIR' path")
endif()


# === Projects ===

# Test Projects
if(LLGL_BUILD_TESTS)
    # Tests for manual testing outside the testbed; Display and window tests are tricky to do in a CI environment
    add_llgl_example_project(Test_Display   CXX "${FilesTest_Display}"  "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_Image     CXX "${FilesTest_Image}"    "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_Window    CXX "${FilesTest_Window}"   "${LLGL_MODULE_LIBS}")
    
    # Testbed
    add_subdirectory(Testbed)
endif(LLGL_BUILD_TESTS)


