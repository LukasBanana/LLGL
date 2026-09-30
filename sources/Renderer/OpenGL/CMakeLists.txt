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
option(LLGL_GL_INCLUDE_EXTERNAL "Include additional OpenGL header files from 'external' folder" ON)

if(LLGL_GL_ENABLE_VENDOR_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_VENDOR_EXT)
endif()

if(LLGL_GL_ENABLE_DSA_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_DSA_EXT)
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

set(LLGL_GL_PROFILE_TARGET 0) # Build module as 'OpenGL'
if(NOT LLGL_MOBILE_PLATFORM)
    set(LLGL_GL_PROFILES "OpenGL Core" CACHE STRING "Select the OpenGL profiles to enable")
    set_property(CACHE LLGL_GL_PROFILES PROPERTY STRINGS "OpenGL Core" "OpenGL Compatibility" "OpenGL Core + Compatibility")

    if(${LLGL_GL_PROFILES} STREQUAL "OpenGL Core")
        set(LLGL_GL_PROFILE_TARGET 0) # Build module as 'OpenGL' (LLGL_GL_ENABLE_OPENGL2X=OFF)
    elseif(${LLGL_GL_PROFILES} STREQUAL "OpenGL Compatibility")
        set(LLGL_GL_PROFILE_TARGET 1) # Build module as 'OpenGL' (LLGL_GL_ENABLE_OPENGL2X=ON)
    elseif(${LLGL_GL_PROFILES} STREQUAL "OpenGL Core + Compatibility")
        set(LLGL_GL_PROFILE_TARGET 2) # Build two separate modules 'OpenGLCore' (LLGL_GL_ENABLE_OPENGL2X=OFF) and 'OpenGLCompat' (LLGL_GL_ENABLE_OPENGL2X=ON)
    else()
        message(SEND_ERROR "Unknown OpenGL profile: ${LLGL_GL_PROFILES}")
        message("Accepted values for LLGL_GL_PROFILES are: 'OpenGL Core', 'OpenGL Compatibility', or 'OpenGL Core + Compatibility'")
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
function(remove_disabled_gl2x_files OUTPUT_LIST ENABLE_GL2X)
    set(ReducedList ${${OUTPUT_LIST}})

    if(NOT ENABLE_GL2X)
        list(
            REMOVE_ITEM ReducedList
                "${PROJECT_SOURCE_DIR}/Buffer/GL2XSharedContextVertexArray.cpp"
                "${PROJECT_SOURCE_DIR}/Buffer/GL2XSharedContextVertexArray.h"
        )
    else()
        list(
            REMOVE_ITEM ReducedList
                "${PROJECT_SOURCE_DIR}/Buffer/GL3PlusSharedContextVertexArray.cpp"
                "${PROJECT_SOURCE_DIR}/Buffer/GL3PlusSharedContextVertexArray.h"
        )
    endif()

    # Propagate the modified list back to the caller since list() only edits the function's local copy
    set(${OUTPUT_LIST} "${ReducedList}" PARENT_SCOPE)
endfunction()

function(set_llgl_opengl_target_profile TARGET_NAME)
    # Set LLGL_OPENGL_PROFILE macro based on target name, i.e. "OpenGLCore", "OpenGLCompat" etc.
    string(LENGTH "LLGL_" TARGET_PREFIX_LEN)
    string(SUBSTRING ${TARGET_NAME} ${TARGET_PREFIX_LEN} -1 OpenGLProfileName)
    ADD_PROJECT_DEFINE_VALUE(${TARGET_NAME} LLGL_OPENGL_PROFILE ${OpenGLProfileName})
endfunction()

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
        if(LLGL_LINUX_ENABLE_WAYLAND)
            find_source_files(FilesRendererGLPlatform   CXX ${PROJECT_SOURCE_DIR}/Platform/Linux ${PROJECT_SOURCE_DIR}/Platform/Linux/X11 ${PROJECT_SOURCE_DIR}/Platform/Linux/Wayland)
        else()
            find_source_files(FilesRendererGLPlatform   CXX ${PROJECT_SOURCE_DIR}/Platform/Linux ${PROJECT_SOURCE_DIR}/Platform/Linux/X11)
        endif()
    
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
    remove_disabled_gl2x_files(FilesWebGL OFF)
    
    include("${EXTERNAL_MODULE_DIR}/FindOpenGLES3.cmake")
    if(OPENGLES_FOUND)
        include_directories(${OPENGLES_INCLUDE_DIR})
        add_llgl_module(LLGL_WebGL LLGL_BUILD_RENDERER_WEBGL "${FilesWebGL}")
        target_link_libraries(LLGL_WebGL LLGL ${OPENGLES_LIBRARIES})
        ADD_PROJECT_DEFINE(LLGL_WebGL LLGL_WEBGL)
        set_llgl_opengl_target_profile(LLGL_WebGL)
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
    remove_disabled_gl2x_files(FilesGLES3 OFF)

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
        set_llgl_opengl_target_profile(LLGL_OpenGLES3)
    else()
        message(FATAL_ERROR "LLGL_BUILD_RENDERER_OPENGLES3 failed: missing OpenGLES libraries")
    endif()
endif()

function(add_llgl_desktop_opengl_target TARGET_NAME ENABLE_GL2X)
    # Collect Desktop OpenGL source files
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
    remove_disabled_gl2x_files(FilesGL ${ENABLE_GL2X})

    if(ENABLE_GL2X)
        list(APPEND FilesGL ${FilesRendererGLProfileGLCompat})
    else()
        list(APPEND FilesGL ${FilesRendererGLProfileGLCore})
    endif()

    # Build Desktop OpenGL target module
    add_llgl_module(${TARGET_NAME} LLGL_BUILD_RENDERER_OPENGL "${FilesGL}")
    target_link_libraries(${TARGET_NAME} LLGL ${OPENGL_LIBRARIES})      

    # Add platform-specific definitions and link libraries
    if(APPLE)
        ADD_PROJECT_DEFINE(${TARGET_NAME} GL_SILENCE_DEPRECATION)
    endif()

    if(ENABLE_GL2X)
        ADD_PROJECT_DEFINE(${TARGET_NAME} LLGL_GL_ENABLE_OPENGL2X)
    endif()
    
    ADD_PROJECT_DEFINE(${TARGET_NAME} LLGL_OPENGL) #TODO: rename this to LLGL_DESKTOP_OPENGL
    set_llgl_opengl_target_profile(${TARGET_NAME})

    if(LLGL_LINUX_ENABLE_WAYLAND)
        target_link_libraries(${TARGET_NAME} LLGL OpenGL::EGL wayland-egl)

        ADD_PROJECT_DEFINE(${TARGET_NAME} LLGL_OPENGL_WAYLAND)
    endif()
endfunction()

if(LLGL_BUILD_RENDERER_OPENGL)
    # Desktop OpenGL Renderer
    set(OpenGL_GL_PREFERENCE GLVND)
    if(LLGL_LINUX_ENABLE_WAYLAND)
        find_package(OpenGL REQUIRED COMPONENTS EGL)
    else()
        find_package(OpenGL REQUIRED)
    endif()

    if(OPENGL_FOUND)
        include_directories(${OPENGL_INCLUDE_DIR})

        if(LLGL_LINUX_ENABLE_WAYLAND)
            if(OpenGL_EGL_FOUND)
                include_directories(${OPENGL_EGL_INCLUDE_DIRS})
            else()
                message(FATAL_ERROR "LLGL_BUILD_RENDERER_OPENGL failed: missing EGL libraries")
            endif()
        endif()

        if(LLGL_GL_PROFILE_TARGET EQUAL 0) # OpenGL module (Core)
            add_llgl_desktop_opengl_target(LLGL_OpenGL OFF)
        elseif(LLGL_GL_PROFILE_TARGET EQUAL 1) # OpenGL module (Compatibility)
            add_llgl_desktop_opengl_target(LLGL_OpenGL ON)
        elseif(LLGL_GL_PROFILE_TARGET EQUAL 2) # OpenGLCore (Core) and OpenGLCompat modules (Compatibility)
            add_llgl_desktop_opengl_target(LLGL_OpenGLCore OFF)
            add_llgl_desktop_opengl_target(LLGL_OpenGLCompat ON)
        endif()

    else()
        message(FATAL_ERROR "LLGL_BUILD_RENDERER_OPENGL failed: missing OpenGL libraries")
    endif()

endif()


