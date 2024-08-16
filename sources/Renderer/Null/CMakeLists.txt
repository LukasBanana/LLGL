#
# CMakeLists.txt file for LLGL/Null backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Null)


# === Source files ===

# Null renderer files
find_source_files(FilesRendererNull             CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererNullBuffer       CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererNullCommand      CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererNullRenderState  CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererNullShader       CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererNullTexture      CXX ${PROJECT_SOURCE_DIR}/Texture)

set(
    FilesNull
    ${FilesRendererNull}
    ${FilesRendererNullBuffer}
    ${FilesRendererNullCommand}
    ${FilesRendererNullRenderState}
    ${FilesRendererNullShader}
    ${FilesRendererNullTexture}
)


# === Source group folders ===

source_group("Null"                 FILES ${FilesRendererNull})
source_group("Null\\Buffer"         FILES ${FilesRendererNullBuffer})
source_group("Null\\Command"        FILES ${FilesRendererNullCommand})
source_group("Null\\RenderState"    FILES ${FilesRendererNullRenderState})
source_group("Null\\Shader"         FILES ${FilesRendererNullShader})
source_group("Null\\Texture"        FILES ${FilesRendererNullTexture})


# === Projects ===

if(LLGL_BUILD_RENDERER_NULL)
    # Null Renderer
    add_llgl_module(LLGL_Null LLGL_BUILD_RENDERER_NULL "${FilesNull}")
    target_link_libraries(LLGL_Null LLGL)
endif()


