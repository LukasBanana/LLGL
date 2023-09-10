#
# CMakeLists.txt file for LLGL/Direct3D12 backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

cmake_minimum_required(VERSION 3.7)

project(LLGL_Direct3D11)


# === Source files ===

# Direct3D12 renderer files
find_source_files(FilesRendererD3D12            CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererD3D12Buffer      CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererD3D12Command     CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererD3D12RenderState CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererD3D12Shader      CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererD3D12Texture     CXX ${PROJECT_SOURCE_DIR}/Texture)

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
)


# === Source group folders ===

source_group("Direct3D12"                   FILES ${FilesRendererD3D12})
source_group("Direct3D12\\Buffer"           FILES ${FilesRendererD3D12Buffer})
source_group("Direct3D12\\Command"          FILES ${FilesRendererD3D12Command})
source_group("Direct3D12\\RenderState"      FILES ${FilesRendererD3D12RenderState})
source_group("Direct3D12\\Shader"           FILES ${FilesRendererD3D12Shader})
source_group("Direct3D12\\Shader\\Builtin"  FILES ${FilesRendererD3D12ShaderBuiltin})
source_group("Direct3D12\\Texture"          FILES ${FilesRendererD3D12Texture})


# === Projects ===

if(LLGL_BUILD_RENDERER_DIRECT3D12)
    # Direct3D 12 Renderer
    add_llgl_module(LLGL_Direct3D12 LLGL_BUILD_RENDERER_DIRECT3D12 "${FilesD3D12}")
    target_link_libraries(LLGL_Direct3D12 LLGL LLGL_DXCommon d3d12 dxgi D3DCompiler)
endif()

