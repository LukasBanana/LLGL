#
# CMakeLists.txt file for LLGL/Null backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Direct3D9)


# === Source files ===

# Direct3D9 renderer files
find_source_files(FilesRendererD3D9             CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererD3D9Buffer       CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererD3D9Command      CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererD3D9RenderState  CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererD3D9Shader       CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererD3D9Texture      CXX ${PROJECT_SOURCE_DIR}/Texture)

set(
    FilesD3D9
    ${FilesRendererD3D9}
    ${FilesRendererD3D9Buffer}
    ${FilesRendererD3D9Command}
    ${FilesRendererD3D9RenderState}
    ${FilesRendererD3D9Shader}
    ${FilesRendererD3D9Texture}
)


# === Source group folders ===

source_group("Direct3D9"                FILES ${FilesRendererD3D9})
source_group("Direct3D9\\Buffer"        FILES ${FilesRendererD3D9Buffer})
source_group("Direct3D9\\Command"       FILES ${FilesRendererD3D9Command})
source_group("Direct3D9\\RenderState"   FILES ${FilesRendererD3D9RenderState})
source_group("Direct3D9\\Shader"        FILES ${FilesRendererD3D9Shader})
source_group("Direct3D9\\Texture"       FILES ${FilesRendererD3D9Texture})


# === Projects ===

if(LLGL_BUILD_RENDERER_DIRECT3D9)
    # Direct3D 9 Renderer
    add_llgl_module(LLGL_Direct3D9 LLGL_BUILD_RENDERER_DIRECT3D9 "${FilesD3D9}")
    target_link_libraries(LLGL_Direct3D9 LLGL d3d9 D3DCompiler)
endif()


