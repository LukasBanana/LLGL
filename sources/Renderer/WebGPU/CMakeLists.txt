#
# CMakeLists.txt file for LLGL/WebGPU backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_WebGPU)


# === Source files ===

# WebGPU renderer files
find_source_files(FilesRendererWebGPU             CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererWebGPUBuffer       CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererWebGPUCommand      CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererWebGPURenderState  CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererWebGPUShader       CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererWebGPUTexture      CXX ${PROJECT_SOURCE_DIR}/Texture)

set(
    FilesWebGPU
    ${FilesRendererWebGPU}
    ${FilesRendererWebGPUBuffer}
    ${FilesRendererWebGPUCommand}
    ${FilesRendererWebGPURenderState}
    ${FilesRendererWebGPUShader}
    ${FilesRendererWebGPUTexture}
)


# === Source group folders ===

source_group("WebGPU"                 FILES ${FilesRendererWebGPU})
source_group("WebGPU\\Buffer"         FILES ${FilesRendererWebGPUBuffer})
source_group("WebGPU\\Command"        FILES ${FilesRendererWebGPUCommand})
source_group("WebGPU\\RenderState"    FILES ${FilesRendererWebGPURenderState})
source_group("WebGPU\\Shader"         FILES ${FilesRendererWebGPUShader})
source_group("WebGPU\\Texture"        FILES ${FilesRendererWebGPUTexture})


# === Projects ===

if(LLGL_BUILD_RENDERER_WEBGPU)
    # WebGPU Renderer
    find_package(WebGPU REQUIRED)
    add_llgl_module(LLGL_WebGPU LLGL_BUILD_RENDERER_WEBGPU "${FilesWebGPU}")
    target_link_libraries(LLGL_WebGPU PRIVATE LLGL WebGPU::WebGPU)
endif()

