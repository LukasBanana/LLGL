#
# CMakeLists.txt file for LLGL/Direct3D11 backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Direct3D11)


# === Options ===

set(LLGL_D3D11_ENABLE_FEATURELEVEL "Direct3D 11.0" CACHE STRING "Enable maximum Direct3D 11 feature level (requires <d3d11.h>, <d3d11_1.h>, <d3d11_2.h>, or <d3d11_3.h> header files")
set_property(CACHE LLGL_D3D11_ENABLE_FEATURELEVEL PROPERTY STRINGS "Direct3D 11.0" "Direct3D 11.1" "Direct3D 11.2" "Direct3D 11.3")

if(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.3")
    ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=3)
elseif(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.2")
    ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=2)
elseif(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.1")
    ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=1)
else()
    ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=0)
endif()


# === Source files ===

# Direct3D11 renderer files
find_source_files(FilesRendererD3D11            CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererD3D11Buffer      CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererD3D11Command     CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererD3D11RenderState CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererD3D11Shader      CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererD3D11Texture     CXX ${PROJECT_SOURCE_DIR}/Texture)
find_source_files(FilesIncludeD3D11             INC ${BACKEND_INCLUDE_DIR}/Direct3D11)

set(FilesRendererD3D11ShaderBuiltin ${PROJECT_SOURCE_DIR}/Shader/Builtin/D3D11Builtin.h)

set(
    FilesD3D11
    ${FilesRendererD3D11}
    ${FilesRendererD3D11Buffer}
    ${FilesRendererD3D11Command}
    ${FilesRendererD3D11Shader}
    ${FilesRendererD3D11ShaderBuiltin}
    ${FilesRendererD3D11RenderState}
    ${FilesRendererD3D11Texture}
    ${FilesIncludeD3D11}
)


# === Source group folders ===

source_group("Direct3D11"                   FILES ${FilesRendererD3D11})
source_group("Direct3D11\\Buffer"           FILES ${FilesRendererD3D11Buffer})
source_group("Direct3D11\\Command"          FILES ${FilesRendererD3D11Command})
source_group("Direct3D11\\RenderState"      FILES ${FilesRendererD3D11RenderState})
source_group("Direct3D11\\Shader"           FILES ${FilesRendererD3D11Shader})
source_group("Direct3D11\\Shader\\Builtin"  FILES ${FilesRendererD3D11ShaderBuiltin})
source_group("Direct3D11\\Texture"          FILES ${FilesRendererD3D11Texture})
source_group("Include\\Platform"            FILES ${FilesIncludeD3D11})


# === Projects ===

if(LLGL_BUILD_RENDERER_DIRECT3D11)
    # Direct3D 11 Renderer
    add_llgl_module(LLGL_Direct3D11 LLGL_BUILD_RENDERER_DIRECT3D11 "${FilesD3D11}")
    target_link_libraries(LLGL_Direct3D11 LLGL LLGL_DXCommon d3d11 dxgi D3DCompiler)
endif()


