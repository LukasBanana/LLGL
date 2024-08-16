#
# CMakeLists.txt file for LLGL/Direct3D12 backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Direct3D12)


# === Options ===

set(LLGL_D3D12_ENABLE_FEATURELEVEL "Direct3D 12.0" CACHE STRING "Enable maximum Direct3D 12 feature level (requires newer Windows SDK versions")
set_property(CACHE LLGL_D3D12_ENABLE_FEATURELEVEL PROPERTY STRINGS "Direct3D 12.0" "Direct3D 12.1" "Direct3D 12.2")

if(${LLGL_D3D12_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 12.2")
    ADD_DEFINE(LLGL_D3D12_ENABLE_FEATURELEVEL=2)
elseif(${LLGL_D3D12_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 12.1")
    ADD_DEFINE(LLGL_D3D12_ENABLE_FEATURELEVEL=1)
else()
    ADD_DEFINE(LLGL_D3D12_ENABLE_FEATURELEVEL=0)
endif()


# === Source files ===

# Direct3D12 renderer files
find_source_files(FilesRendererD3D12            CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererD3D12Buffer      CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererD3D12Command     CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererD3D12RenderState CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererD3D12Shader      CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererD3D12Texture     CXX ${PROJECT_SOURCE_DIR}/Texture)
find_source_files(FilesIncludeD3D12             INC ${BACKEND_INCLUDE_DIR}/Direct3D12)

set(FilesRendererD3D12ShaderBuiltin ${PROJECT_SOURCE_DIR}/Shader/Builtin/D3D12Builtin.h)

set(
    FilesD3D12
    ${FilesRendererD3D12}
    ${FilesRendererD3D12Buffer}
    ${FilesRendererD3D12Command}
    ${FilesRendererD3D12Shader}
    ${FilesRendererD3D12ShaderBuiltin}
    ${FilesRendererD3D12RenderState}
    ${FilesRendererD3D12Texture}
    ${FilesIncludeD3D12}
)


# === Source group folders ===

source_group("Direct3D12"                   FILES ${FilesRendererD3D12})
source_group("Direct3D12\\Buffer"           FILES ${FilesRendererD3D12Buffer})
source_group("Direct3D12\\Command"          FILES ${FilesRendererD3D12Command})
source_group("Direct3D12\\RenderState"      FILES ${FilesRendererD3D12RenderState})
source_group("Direct3D12\\Shader"           FILES ${FilesRendererD3D12Shader})
source_group("Direct3D12\\Shader\\Builtin"  FILES ${FilesRendererD3D12ShaderBuiltin})
source_group("Direct3D12\\Texture"          FILES ${FilesRendererD3D12Texture})
source_group("Include\\Platform"            FILES ${FilesIncludeD3D12})


# === Projects ===

if(LLGL_BUILD_RENDERER_DIRECT3D12)
    # Adopt define from LLGL_DXCommon lib
    if(LLGL_D3D12_ENABLE_DXCOMPILER)
        ADD_DEFINE(LLGL_D3D12_ENABLE_DXCOMPILER)
    endif()

    # Direct3D 12 Renderer
    add_llgl_module(LLGL_Direct3D12 LLGL_BUILD_RENDERER_DIRECT3D12 "${FilesD3D12}")
    target_link_libraries(LLGL_Direct3D12 LLGL LLGL_DXCommon d3d12 dxgi D3DCompiler)
endif()


