#
# CMakeLists.txt file for LLGL/DXCommon support lib
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_DXCommon)


# === Options ===

option(LLGL_D3D12_ENABLE_DXCOMPILER "Enable support for DirectXShaderCompiler (DXC); requires Windows Kit to be installed" OFF)

if(LLGL_D3D12_ENABLE_DXCOMPILER)
    ADD_DEFINE(LLGL_D3D12_ENABLE_DXCOMPILER)
endif()


# === Source files ===

# Direct3D common renderer files
find_source_files(FilesSupportLibDXCommon CXX ${PROJECT_SOURCE_DIR})

set(FilesDXCommon "${FilesSupportLibDXCommon}")

if(LLGL_D3D12_ENABLE_DXCOMPILER)
    find_source_files(FilesSupportLibDXCommonDXC CXX ${PROJECT_SOURCE_DIR}/DXC)
    list(APPEND FilesDXCommon "${FilesSupportLibDXCommonDXC}")
endif()


# === Source group folders ===

source_group("DXCommon"         FILES ${FilesSupportLibDXCommon})
source_group("DXCommon\\DXC"    FILES ${FilesSupportLibDXCommonDXC})


# === Projects ===

if(LLGL_BUILD_RENDERER_DIRECT3D11 OR LLGL_BUILD_RENDERER_DIRECT3D12)
    add_library(LLGL_DXCommon STATIC "${FilesDXCommon}")
    set_llgl_module_properties(LLGL_DXCommon)
endif()


