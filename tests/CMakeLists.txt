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

find_project_source_files( FilesTest_Compute            "${TEST_PROJECTS_DIR}/Test_Compute.cpp"         )
find_project_source_files( FilesTest_D3D12              "${TEST_PROJECTS_DIR}/Test_D3D12.cpp"           )
find_project_source_files( FilesTest_Display            "${TEST_PROJECTS_DIR}/Test_Display.cpp"         )
find_project_source_files( FilesTest_Image              "${TEST_PROJECTS_DIR}/Test_Image.cpp"           )
find_project_source_files( FilesTest_Metal              "${TEST_PROJECTS_DIR}/Test_Metal.cpp"           )
find_project_source_files( FilesTest_OpenGL             "${TEST_PROJECTS_DIR}/Test_OpenGL.cpp"          )
find_project_source_files( FilesTest_Performance        "${TEST_PROJECTS_DIR}/Test_Performance.cpp"     )
find_project_source_files( FilesTest_ShaderReflect      "${TEST_PROJECTS_DIR}/Test_ShaderReflect.cpp"   )
find_project_source_files( FilesTest_SeparateShaders    "${TEST_PROJECTS_DIR}/Test_SeparateShaders.cpp" )
find_project_source_files( FilesTest_Vulkan             "${TEST_PROJECTS_DIR}/Test_Vulkan.cpp"          )
find_project_source_files( FilesTest_Window             "${TEST_PROJECTS_DIR}/Test_Window.cpp"          )


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
    # Platform specific test projects
    add_llgl_example_project(Test_OpenGL CXX "${FilesTest_OpenGL}" "${LLGL_MODULE_LIBS}")
    if(LLGL_BUILD_RENDERER_DIRECT3D12 AND WIN32)
        add_llgl_example_project(Test_D3D12 CXX "${FilesTest_D3D12}" "${LLGL_MODULE_LIBS}")
    endif()
    if(LLGL_BUILD_RENDERER_METAL AND APPLE)
        add_llgl_example_project(Test_Metal CXX "${FilesTest_Metal}" "${LLGL_MODULE_LIBS}")
    endif()
    if(LLGL_BUILD_RENDERER_VULKAN AND NOT APPLE)
        add_llgl_example_project(Test_Vulkan CXX "${FilesTest_Vulkan}" "${LLGL_MODULE_LIBS}")
    endif()
    
    # Common tests
    add_llgl_example_project(Test_Compute           CXX "${FilesTest_Compute}"          "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_Display           CXX "${FilesTest_Display}"          "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_Image             CXX "${FilesTest_Image}"            "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_Performance       CXX "${FilesTest_Performance}"      "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_SeparateShaders   CXX "${FilesTest_SeparateShaders}"  "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_ShaderReflect     CXX "${FilesTest_ShaderReflect}"    "${LLGL_MODULE_LIBS}")
    add_llgl_example_project(Test_Window            CXX "${FilesTest_Window}"           "${LLGL_MODULE_LIBS}")
    
    # Testbed
    add_subdirectory(Testbed)
endif(LLGL_BUILD_TESTS)


