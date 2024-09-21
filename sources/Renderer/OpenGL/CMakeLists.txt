#
# CMakeLists.txt file for LLGL/OpenGL backend
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_OpenGL)


# === Options ===

if(LLGL_BUILD_RENDERER_OPENGLES3)
    set(LLGL_GL_ENABLE_OPENGLES "OpenGLES 3.0" CACHE STRING "Enable maximum OpenGLES version to include (requires <GLES3/gl3.h>, <GLES3/gl31.h>, or <GLES3/gl32.h> header files")
    set_property(CACHE LLGL_GL_ENABLE_OPENGLES PROPERTY STRINGS "OpenGLES 3.0" "OpenGLES 3.1" "OpenGLES 3.2")
endif()

option(LLGL_GL_ENABLE_VENDOR_EXT "Enable vendor specific OpenGL extensions (e.g. GL_NV_..., GL_AMD_... etc.)" ON)
option(LLGL_GL_ENABLE_DSA_EXT "Enable OpenGL direct state access (DSA) extension if available" ON)
option(LLGL_GL_ENABLE_OPENGL2X "Enable OpenGL 2.x compatibility profile instead of OpenGL 3+ core profile" OFF)
option(LLGL_GL_INCLUDE_EXTERNAL "Include additional OpenGL header files from 'external' folder" ON)

if(LLGL_GL_ENABLE_VENDOR_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_VENDOR_EXT)
endif()

if(LLGL_GL_ENABLE_DSA_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_DSA_EXT)
endif()

if(LLGL_GL_ENABLE_OPENGL2X)
    ADD_DEFINE(LLGL_GL_ENABLE_OPENGL2X)
endif()

if(LLGL_BUILD_RENDERER_OPENGLES3)
    if(${LLGL_GL_ENABLE_OPENGLES} STREQUAL "OpenGLES 3.2")
        ADD_DEFINE(LLGL_GL_ENABLE_OPENGLES=320)
    elseif(${LLGL_GL_ENABLE_OPENGLES} STREQUAL "OpenGLES 3.1")
        ADD_DEFINE(LLGL_GL_ENABLE_OPENGLES=310)
    elseif(${LLGL_GL_ENABLE_OPENGLES} STREQUAL "OpenGLES 3.0")
        ADD_DEFINE(LLGL_GL_ENABLE_OPENGLES=300)
    else()
        message(SEND_ERROR "Unknown OpenGLES version: ${LLGL_GL_ENABLE_OPENGLES}")
        message("Accepted values for LLGL_GL_ENABLE_OPENGLES are: 'OpenGLES 3.0', 'OpenGLES 3.1', or 'OpenGLES 3.2'")
    endif()
endif()


# === Source files ===

find_source_files(FilesRendererGL                   CXX ${PROJECT_SOURCE_DIR})
find_source_files(FilesRendererGLBuffer             CXX ${PROJECT_SOURCE_DIR}/Buffer)
find_source_files(FilesRendererGLCommand            CXX ${PROJECT_SOURCE_DIR}/Command)
find_source_files(FilesRendererGLExt                CXX ${PROJECT_SOURCE_DIR}/Ext)
find_source_files(FilesRendererGLPlatformBase       CXX ${PROJECT_SOURCE_DIR}/Platform)
find_source_files(FilesRendererGLRenderState        CXX ${PROJECT_SOURCE_DIR}/RenderState)
find_source_files(FilesRendererGLShader             CXX ${PROJECT_SOURCE_DIR}/Shader)
find_source_files(FilesRendererGLTexture            CXX ${PROJECT_SOURCE_DIR}/Texture)
find_source_files(FilesRendererGLProfile            CXX ${PROJECT_SOURCE_DIR}/Profile)
find_source_files(FilesRendererGLProfileGLCompat    CXX ${PROJECT_SOURCE_DIR}/Profile/GLCompat)
find_source_files(FilesRendererGLProfileGLCore      CXX ${PROJECT_SOURCE_DIR}/Profile/GLCore)
find_source_files(FilesRendererGLProfileGLES        CXX ${PROJECT_SOURCE_DIR}/Profile/GLES)
find_source_files(FilesRendererGLProfileWebGL       CXX ${PROJECT_SOURCE_DIR}/Profile/WebGL)
find_source_files(FilesIncludeGL                    INC ${BACKEND_INCLUDE_DIR}/OpenGL)

# Remove selected files if GL2X is disabled
if(NOT LLGL_GL_ENABLE_OPENGL2X)
    list(
        REMOVE_ITEM FilesRendererGLBuffer
            "${PROJECT_SOURCE_DIR}/Buffer/GL2XSharedContextVertexArray.cpp"
            "${PROJECT_SOURCE_DIR}/Buffer/GL2XSharedContextVertexArray.h"
    )
else()
    list(
        REMOVE_ITEM FilesRendererGLBuffer
        "${PROJECT_SOURCE_DIR}/Buffer/GL3PlusSharedContextVertexArray.cpp"
        "${PROJECT_SOURCE_DIR}/Buffer/GL3PlusSharedContextVertexArray.h"
    )
endif()

# Remove selected files for GLES3 renderer
set(FilesRendererGLES3Shader ${FilesRendererGLShader})
list(
    REMOVE_ITEM FilesRendererGLES3Shader
    "${PROJECT_SOURCE_DIR}/Shader/GLSeparableShader.cpp"
    "${PROJECT_SOURCE_DIR}/Shader/GLSeparableShader.h"
    "${PROJECT_SOURCE_DIR}/Shader/GLProgramPipeline.cpp"
    "${PROJECT_SOURCE_DIR}/Shader/GLProgramPipeline.h"
)

if(WIN32)
    find_source_files(FilesRendererGLPlatform       CXX ${PROJECT_SOURCE_DIR}/Platform/Win32)
    find_source_files(FilesIncludeGLPlatform        INC ${BACKEND_INCLUDE_DIR}/OpenGL/Win32)
elseif(APPLE)
    if(LLGL_MOBILE_PLATFORM)
        find_source_files(FilesRendererGLPlatform   OBJC ${PROJECT_SOURCE_DIR}/Platform/IOS)
        find_source_files(FilesIncludeGLPlatform    INC ${BACKEND_INCLUDE_DIR}/OpenGL/IOS)
    else()
        find_source_files(FilesRendererGLPlatform   OBJC ${PROJECT_SOURCE_DIR}/Platform/MacOS)
        find_source_files(FilesIncludeGLPlatform    INC ${BACKEND_INCLUDE_DIR}/OpenGL/MacOS)
    endif()
elseif(UNIX)
    if(LLGL_ANDROID_PLATFORM)
        find_source_files(FilesRendererGLPlatform   CXX ${PROJECT_SOURCE_DIR}/Platform/Android)
        find_source_files(FilesIncludeGLPlatform    INC ${BACKEND_INCLUDE_DIR}/OpenGL/Android)
    elseif(EMSCRIPTEN)
        find_source_files(FilesRendererGLPlatform   CXX ${PROJECT_SOURCE_DIR}/Platform/Wasm)
        find_source_files(FilesIncludeGLPlatform    INC ${BACKEND_INCLUDE_DIR}/OpenGL/Wasm)        
    else()
        find_source_files(FilesRendererGLPlatform   CXX ${PROJECT_SOURCE_DIR}/Platform/Linux)
        find_source_files(FilesIncludeGLPlatform    INC ${BACKEND_INCLUDE_DIR}/OpenGL/Linux)
    endif()
endif()


# === Source group folders ===

set(
    FilesRendererGLProfileAll
    ${FilesRendererGLProfile}
    ${FilesRendererGLProfileGLCompat}
    ${FilesRendererGLProfileGLCore}
    ${FilesRendererGLProfileGLES}
    ${FilesRendererGLProfileWebGL}
)

source_group("OpenGL"                   FILES ${FilesRendererGL})
source_group("OpenGL\\Buffer"           FILES ${FilesRendererGLBuffer})
source_group("OpenGL\\Command"          FILES ${FilesRendererGLCommand})
source_group("OpenGL\\Ext"              FILES ${FilesRendererGLExt})
source_group("OpenGL\\Platform"         FILES ${FilesRendererGLPlatformBase} ${FilesRendererGLPlatform})
source_group("OpenGL\\RenderState"      FILES ${FilesRendererGLRenderState})
source_group("OpenGL\\Shader"           FILES ${FilesRendererGLShader})
source_group("OpenGL\\Texture"          FILES ${FilesRendererGLTexture})
source_group("OpenGL\\Profile"          FILES ${FilesRendererGLProfileAll})
source_group("Include\\Platform"        FILES ${FilesIncludeGL} ${FilesIncludeGLPlatform})


# === Include directories ===

if(LLGL_GL_INCLUDE_EXTERNAL)
    # OpenGL extension headers
    include_directories("${EXTERNAL_INCLUDE_DIR}/OpenGL/include")
endif()


# === Projects ===

if(LLGL_BUILD_RENDERER_WEBGL)
    # WebGL Renderer
    set(
        FilesWebGL
        ${FilesRendererGL}
        ${FilesRendererGLBuffer}
        ${FilesRendererGLCommand}
        ${FilesRendererGLExt}
        ${FilesRendererGLPlatform}
        ${FilesRendererGLPlatformBase}
        ${FilesRendererGLRenderState}
        ${FilesRendererGLES3Shader}
        ${FilesRendererGLTexture}
        ${FilesRendererGLProfile}
        ${FilesRendererGLProfileWebGL}
        ${FilesIncludeGL}
        ${FilesIncludeGLPlatform}
    )
    
    include("${EXTERNAL_MODULE_DIR}/FindOpenGLES3.cmake")
    if(OPENGLES_FOUND)
        include_directories(${OPENGLES_INCLUDE_DIR})
        add_llgl_module(LLGL_WebGL LLGL_BUILD_RENDERER_WEBGL "${FilesWebGL}")
        target_link_libraries(LLGL_WebGL LLGL ${OPENGLES_LIBRARIES})
        ADD_PROJECT_DEFINE(LLGL_WebGL LLGL_WEBGL)
    else()
        message(FATAL_ERROR "LLGL_BUILD_RENDERER_WEBGL failed: missing OpenGLES libraries")
    endif()
endif()

if(LLGL_BUILD_RENDERER_OPENGLES3)
    # OpenGLES Renderer
    set(
        FilesGLES3
        ${FilesRendererGL}
        ${FilesRendererGLBuffer}
        ${FilesRendererGLCommand}
        ${FilesRendererGLExt}
        ${FilesRendererGLPlatform}
        ${FilesRendererGLPlatformBase}
        ${FilesRendererGLRenderState}
        ${FilesRendererGLES3Shader}
        ${FilesRendererGLTexture}
        ${FilesRendererGLProfile}
        ${FilesRendererGLProfileGLES}
        ${FilesIncludeGL}
        ${FilesIncludeGLPlatform}
    )

    include("${EXTERNAL_MODULE_DIR}/FindOpenGLES3.cmake")
    if(OPENGLES_FOUND)
        include_directories(${OPENGLES_INCLUDE_DIR})

        add_llgl_module(LLGL_OpenGLES3 LLGL_BUILD_RENDERER_OPENGLES3 "${FilesGLES3}")
        
        if(APPLE)
            target_link_libraries(LLGL_OpenGLES3 LLGL ${OPENGLES_LIBRARIES} "-framework Foundation -framework UIKit -framework QuartzCore -framework OpenGLES -framework GLKit")
            ADD_PROJECT_DEFINE(LLGL_OpenGLES3 GLES_SILENCE_DEPRECATION)
        else()
            target_link_libraries(LLGL_OpenGLES3 LLGL EGL GLESv3)
        endif()
        
        ADD_PROJECT_DEFINE(LLGL_OpenGLES3 LLGL_OPENGLES3)
    else()
        message(FATAL_ERROR "LLGL_BUILD_RENDERER_OPENGLES3 failed: missing OpenGLES libraries")
    endif()
endif()

if(LLGL_BUILD_RENDERER_OPENGL)
    # OpenGL Renderer
    set(
        FilesGL
        ${FilesRendererGL}
        ${FilesRendererGLBuffer}
        ${FilesRendererGLCommand}
        ${FilesRendererGLExt}
        ${FilesRendererGLPlatform}
        ${FilesRendererGLPlatformBase}
        ${FilesRendererGLRenderState}
        ${FilesRendererGLShader}
        ${FilesRendererGLTexture}
        ${FilesRendererGLProfile}
        ${FilesIncludeGL}
        ${FilesIncludeGLPlatform}
    )

    if(LLGL_GL_ENABLE_OPENGL2X)
        list(APPEND FilesGL ${FilesRendererGLProfileGLCompat})
    else()
        list(APPEND FilesGL ${FilesRendererGLProfileGLCore})
    endif()

    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    if(OPENGL_FOUND)
        include_directories(${OPENGL_INCLUDE_DIR})
        
        add_llgl_module(LLGL_OpenGL LLGL_BUILD_RENDERER_OPENGL "${FilesGL}")
        
        target_link_libraries(LLGL_OpenGL LLGL ${OPENGL_LIBRARIES})      

        if(APPLE)
            ADD_PROJECT_DEFINE(LLGL_OpenGL GL_SILENCE_DEPRECATION)
        endif()
        
        ADD_PROJECT_DEFINE(LLGL_OpenGL LLGL_OPENGL)
    else()
        message(FATAL_ERROR "LLGL_BUILD_RENDERER_OPENGL failed: missing OpenGL libraries")
    endif()
endif()


