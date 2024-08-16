#
# CMakeLists.txt file for LLGL/Metal backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_Metal)


# === Source files ===

# Metal renderer files
find_source_files(FilesRendererMT               OBJC ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererMTBuffer         OBJC ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererMTCommand        OBJC ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererMTRenderState    OBJC ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererMTShader         OBJC ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererMTTexture        OBJC ${PROJECT_SOURCE_DIR}/Texture)
find_source_files(FilesIncludeMT                INC  ${BACKEND_INCLUDE_DIR}/Metal)

set(
    FilesRendererMTShaderBuiltin
    ${PROJECT_SOURCE_DIR}/Shader/Builtin/MTBuiltin.h
    ${PROJECT_SOURCE_DIR}/Shader/Builtin/MTBuiltin.mm
)

set(
    FilesMT
    ${FilesRendererMT}
    ${FilesRendererMTBuffer}
    ${FilesRendererMTCommand}
    ${FilesRendererMTRenderState}
    ${FilesRendererMTShader}
    ${FilesRendererMTShaderBuiltin}
    ${FilesRendererMTTexture}
    ${FilesIncludeMT}
)


# === Source group folders ===

source_group("Metal"                    FILES ${FilesRendererMT})
source_group("Metal\\Buffer"            FILES ${FilesRendererMTBuffer})
source_group("Metal\\Command"           FILES ${FilesRendererMTCommand})
source_group("Metal\\RenderState"       FILES ${FilesRendererMTRenderState})
source_group("Metal\\Shader"            FILES ${FilesRendererMTShader})
source_group("Metal\\Shader\\Builtin"   FILES ${FilesRendererMTShaderBuiltin})
source_group("Metal\\Texture"           FILES ${FilesRendererMTTexture})
source_group("Include\\Platform"        FILES ${FilesIncludeMT})


# === Projects ===

if(LLGL_BUILD_RENDERER_METAL)
    # Metal Renderer
    include("${EXTERNAL_MODULE_DIR}/FindMetal.cmake")
    include("${EXTERNAL_MODULE_DIR}/FindMetalKit.cmake")
    if(METAL_FOUND AND METALKIT_FOUND)
        add_llgl_module(LLGL_Metal LLGL_BUILD_RENDERER_METAL "${FilesMT}")
        
        if(LLGL_MOBILE_PLATFORM)
            target_link_libraries(LLGL_Metal LLGL "-framework Foundation -framework UIKit -framework QuartzCore -framework Metal -framework MetalKit")
        else()
            target_link_libraries(LLGL_Metal LLGL ${METAL_LIBRARY} ${METALKIT_LIBRARY})
        endif()
    else()
        message(FATAL_ERROR "LLGL_BUILD_RENDERER_METAL failed: Missing Metal/MetalKit framework")
    endif()
endif()


