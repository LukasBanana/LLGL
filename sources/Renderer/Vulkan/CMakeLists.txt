#
# CMakeLists.txt file for LLGL/Vulkan backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Vulkan)


# === Options ===

if(NOT APPLE)
    option(LLGL_VK_ENABLE_SPIRV_REFLECT "Enable shader reflection of SPIR-V modules (requires the SPIRV submodule)" OFF)
endif()

if(LLGL_VK_ENABLE_SPIRV_REFLECT)
    ADD_DEFINE(LLGL_VK_ENABLE_SPIRV_REFLECT)
endif()


# === Source files ===

# SPIR-V renderer files
find_source_files(FilesRendererSPIRV            CXX ${PROJECT_SOURCE_DIR}/../SPIRV) # TODO: maybe make this a static lib like DXCommon?

# Vulkan renderer files
find_source_files(FilesRendererVK               CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererVKBuffer         CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererVKCommand        CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererVKExt            CXX ${PROJECT_SOURCE_DIR}/Ext)
find_source_files(FilesRendererVKMemory         CXX ${PROJECT_SOURCE_DIR}/Memory)
find_source_files(FilesRendererVKRenderState    CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererVKShader         CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererVKTexture        CXX ${PROJECT_SOURCE_DIR}/Texture)
find_source_files(FilesIncludeVK                INC ${BACKEND_INCLUDE_DIR}/Vulkan)

set(
    FilesVK
    ${FilesRendererVK}
    ${FilesRendererVKBuffer}
    ${FilesRendererVKCommand}
    ${FilesRendererVKExt}
    ${FilesRendererVKMemory}
    ${FilesRendererVKRenderState}
    ${FilesRendererVKShader}
    ${FilesRendererVKTexture}
    ${FilesIncludeVK}
)

if(LLGL_VK_ENABLE_SPIRV_REFLECT)
    set(FilesVK ${FilesVK} ${FilesRendererSPIRV})
endif()


# === Source group folders ===

source_group("SPIRV"                FILES ${FilesRendererSPIRV})

source_group("Vulkan"               FILES ${FilesRendererVK})
source_group("Vulkan\\Buffer"       FILES ${FilesRendererVKBuffer})
source_group("Vulkan\\Command"      FILES ${FilesRendererVKCommand})
source_group("Vulkan\\Ext"          FILES ${FilesRendererVKExt})
source_group("Vulkan\\Memory"       FILES ${FilesRendererVKMemory})
source_group("Vulkan\\RenderState"  FILES ${FilesRendererVKRenderState})
source_group("Vulkan\\Shader"       FILES ${FilesRendererVKShader})
source_group("Vulkan\\Texture"      FILES ${FilesRendererVKTexture})
source_group("Include\\Platform"    FILES ${FilesIncludeVK})


# === Include directories ===

if(LLGL_VK_ENABLE_SPIRV_REFLECT)
    # SPIRV Submodule
    include_directories("${EXTERNAL_INCLUDE_DIR}/SPIRV-Headers/include")
endif()


# === Projects ===

if(LLGL_BUILD_RENDERER_VULKAN)
    # Vulkan Renderer
    if(LLGL_ANDROID_PLATFORM)
        add_llgl_module(LLGL_Vulkan LLGL_BUILD_RENDERER_VULKAN "${FilesVK}")
        
        target_link_libraries(LLGL_Vulkan LLGL vulkan)
    else()
        find_package(Vulkan REQUIRED)
        if(Vulkan_FOUND)
            include_directories(${Vulkan_INCLUDE_DIR})
            
            add_llgl_module(LLGL_Vulkan LLGL_BUILD_RENDERER_VULKAN "${FilesVK}")
            
            target_link_libraries(LLGL_Vulkan LLGL ${Vulkan_LIBRARY})
        else()
            message(FATAL_ERROR "LLGL_BUILD_RENDERER_VULKAN failed: missing Vulkan libraries")
        endif()
    endif()
endif()


