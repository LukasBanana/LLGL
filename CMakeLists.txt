
# === CMake lists for the LLGL - (09/07/2015) ===

cmake_minimum_required(VERSION 3.7)

# === Target platforms ===

if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    set(LLGL_BUILD_64BIT ON)
else()
    set(LLGL_BUILD_64BIT OFF)
endif()

if(NOT DEFINED LLGL_TARGET_PLATFORM)
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
        set(LLGL_TARGET_PLATFORM "Android")
    elseif(WIN32)
        if(LLGL_BUILD_64BIT)
            set(LLGL_TARGET_PLATFORM "Win64")
        else()
            set(LLGL_TARGET_PLATFORM "Win32")
        endif()
    elseif(APPLE)
        if("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
            set(LLGL_TARGET_PLATFORM "iOS")
        else()
            set(LLGL_TARGET_PLATFORM "macOS")
        endif()
    elseif(UNIX)
        set(LLGL_TARGET_PLATFORM "Linux")
    endif()
endif()

set(LLGL_IOS_PLATFORM OFF)
set(LLGL_ANDROID_PLATFORM OFF)

if(${LLGL_TARGET_PLATFORM} STREQUAL "iOS")
    set(LLGL_IOS_PLATFORM ON)
elseif(${LLGL_TARGET_PLATFORM} STREQUAL "Android")
    if(NOT DEFINED ENV{ANDROID_NDK_HOME})
        message(FATAL_ERROR "Cannot build for Android platform: Missing environment variable 'ANDROID_NDK_HOME'")
    endif()
    set(LLGL_ANDROID_PLATFORM ON)
endif()


# === Project ===

project(LLGL)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(APPLE AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.5")
    # Once Xcode schemes are generated for example, we also have to enable them globally or the main projects will disappear
    set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
endif()


# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)

set( PROJECT_INCLUDE_DIR        "${PROJECT_SOURCE_DIR}/include"           )
set( EXAMPLE_PROJECTS_ROOT_DIR  "${PROJECT_SOURCE_DIR}/examples"          )
set( EXAMPLE_CPP_PROJECTS_DIR   "${EXAMPLE_PROJECTS_ROOT_DIR}/Cpp"        )
set( EXAMPLE_C99_PROJECTS_DIR   "${EXAMPLE_PROJECTS_ROOT_DIR}/C99"        )
set( EXAMPLEBASE_PROJECT_DIR    "${EXAMPLE_CPP_PROJECTS_DIR}/ExampleBase" )
set( EXAMPLE_MEDIA_DIR          "${EXAMPLE_PROJECTS_ROOT_DIR}/Media"      )
set( TEST_PROJECTS_DIR          "${PROJECT_SOURCE_DIR}/tests"             )


# === Macros ===

if(LLGL_IOS_PLATFORM OR LLGL_ANDROID_PLATFORM)
    set(LLGL_MOBILE_PLATFORM ON)
else()
    set(LLGL_MOBILE_PLATFORM OFF)
endif()

macro(ADD_DEFINE IDENT)
    add_definitions("-D${IDENT}")
endmacro()

macro(ADD_PROJECT_DEFINE TARGET_NAME IDENT)
    target_compile_definitions(${TARGET_NAME} PRIVATE "-D${IDENT}")
endmacro()

macro(ADD_DEBUG_DEFINE IDENT)
    if(MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D${IDENT}")
    else(MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D${IDENT}")
    endif(MSVC)
endmacro()

macro(SET_RENDERER_PROJECT_PROPERTIES PROJECT_NAME)
    set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "LLGL")
endmacro()

macro(SET_PROJECT_WORKING_DIR PROJECT_NAME WORKING_DIR)
    if(WIN32)
        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.8.2")
            set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIR}")
        endif()
    elseif(APPLE)
        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.5")
            set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_GENERATE_SCHEME TRUE XCODE_SCHEME_WORKING_DIRECTORY "${WORKING_DIR}")
        endif()
    endif()
endmacro()

macro(SET_EXAMPLE_PROJECT_WORKING_DIR PROJECT_NAME)
    if(${PROJECT_NAME} MATCHES "Example_C99_+")
        string(SUBSTRING ${PROJECT_NAME} 12 -1 TEST_BASE_NAME)
        SET_PROJECT_WORKING_DIR(${PROJECT_NAME} "${EXAMPLE_C99_PROJECTS_DIR}/${TEST_BASE_NAME}")
    elseif(${PROJECT_NAME} MATCHES "Example_+")
        string(SUBSTRING ${PROJECT_NAME} 8 -1 TEST_BASE_NAME)
        SET_PROJECT_WORKING_DIR(${PROJECT_NAME} "${EXAMPLE_CPP_PROJECTS_DIR}/${TEST_BASE_NAME}")
    elseif(${PROJECT_NAME} MATCHES "Test_+")
        SET_PROJECT_WORKING_DIR(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/tests")
    elseif(${PROJECT_NAME} MATCHES "Testbed")
        SET_PROJECT_WORKING_DIR(${PROJECT_NAME} "${PROJECT_SOURCE_DIR}/tests/Testbed")
    endif()
endmacro()

macro(SET_EXAMPLE_PROJECT_FOLDER PROJECT_NAME)
    if(${PROJECT_NAME} MATCHES "Example_C99_+")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples (C99)")
    elseif(${PROJECT_NAME} MATCHES "Example_+")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")
    elseif(${PROJECT_NAME} MATCHES "Test_+" OR ${PROJECT_NAME} MATCHES "Testbed")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")
    endif()
endmacro()

macro(ADD_EXAMPLE_PROJECT PROJECT_NAME LINKER_LANG SRC_FILES LIB_FILES)
    if(APPLE)
        # Project configurations
        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC_FILES})

        if (${PROJECT_NAME} MATCHES "Example_+")
            string(LENGTH "Example_" NAME_PREFIX_LEN)
            string(SUBSTRING ${PROJECT_NAME} ${NAME_PREFIX_LEN} -1 PROJECT_BUNDLE_ID_BASE)
        elseif (${PROJECT_NAME} MATCHES "Test_+")
            string(LENGTH "Test_" NAME_PREFIX_LEN)
            string(SUBSTRING ${PROJECT_NAME} ${NAME_PREFIX_LEN} -1 PROJECT_BUNDLE_ID_BASE)
        else()
            set(PROJECT_BUNDLE_ID_BASE ${PROJECT_NAME})
        endif()

        if(LLGL_IOS_PLATFORM)
            set(PROJECT_BUNDLE_NAME ${PROJECT_BUNDLE_ID_BASE})
        else()
            set(PROJECT_BUNDLE_NAME ${PROJECT_NAME})
        endif()

        set(RES_FILES "${SRC_FILES}")
        list(FILTER RES_FILES INCLUDE REGEX ".+\\.(obj|png|jpg|dds|icns)")
        source_group("Resources" FILES ${RES_FILES})

        set(SHADER_FILES "${SRC_FILES}")
        list(FILTER SHADER_FILES INCLUDE REGEX ".+\\.(metal|vert|frag)")
        source_group("Shaders" FILES ${SHADER_FILES})

        set(BundleResourceFiles "${RES_FILES};${SHADER_FILES}")
        if(LLGL_IOS_PLATFORM)
            set(BundleInfoPlistFile "${PROJECT_SOURCE_DIR}/cmake/IOSBundleInfo.plist.in")
            set(BundleIconFile "AppIcon.png")
        else()
            set(BundleInfoPlistFile "${PROJECT_SOURCE_DIR}/cmake/MacOSXBundleInfo.plist.in")
            set(BundleIconFile "AppIcon.icns")
        endif()

        set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME                         "${PROJECT_BUNDLE_NAME}"
            MACOSX_BUNDLE_GUI_IDENTIFIER        "LLGL.Examples.${PROJECT_BUNDLE_ID_BASE}"
            MACOSX_BUNDLE_LONG_VERSION_STRING   "LLGL-1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING  "1.0"
            MACOSX_BUNDLE_BUNDLE_VERSION        1.0
            MACOSX_BUNDLE_ICON_FILE             "${BundleIconFile}"
            MACOSX_BUNDLE                       ON
            MACOSX_BUNDLE_INFO_PLIST            "${BundleInfoPlistFile}"
            RESOURCE                            "${BundleResourceFiles}"
        )
    elseif(LLGL_ANDROID_PLATFORM)
        # Android project configuratons
        add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
        target_link_libraries(${PROJECT_NAME} ${LIB_FILES} android_native_app_glue log)
    else()
        add_executable(${PROJECT_NAME} ${SRC_FILES})
    endif()

    target_link_libraries(${PROJECT_NAME} ${LIB_FILES})

    set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE ${LINKER_LANG} DEBUG_POSTFIX "D")
    SET_EXAMPLE_PROJECT_WORKING_DIR(${PROJECT_NAME})
    SET_EXAMPLE_PROJECT_FOLDER(${PROJECT_NAME})
endmacro()


# === Functions ===

function(bundle_project_files OUTPUT_LIST SRC_FILES)
    if(APPLE)
        if (LLGL_IOS_PLATFORM)
            file(
                GLOB ProjectFiles
                ${EXAMPLEBASE_PROJECT_DIR}/iOS/AppIcon.png
                ${EXAMPLEBASE_PROJECT_DIR}/iOS/LaunchScreen.storyboard
            )
        else()
            file(
                GLOB ProjectFiles
                ${EXAMPLEBASE_PROJECT_DIR}/macOS/AppIcon.icns
            )
        endif()
        list(APPEND ProjectFiles "${SRC_FILES}")
    else()
        set(ProjectFiles "${SRC_FILES}")
    endif()
    set(${OUTPUT_LIST} ${ProjectFiles} PARENT_SCOPE)
endfunction()

function(find_project_source_files OUTPUT_LIST PROJECT_DIR)
    if("${PROJECT_DIR}" MATCHES ".+\\.(cpp|c)")
        set(ProjectFiles "${PROJECT_DIR}")
    else()
        if(APPLE)
            file(
                GLOB ProjectFiles
                ${PROJECT_DIR}/*.c
                ${PROJECT_DIR}/*.cpp
                ${PROJECT_DIR}/*.metal
                ${PROJECT_DIR}/*.vert
                ${PROJECT_DIR}/*.frag
            )
        else()
            file(
                GLOB ProjectFiles
                ${PROJECT_DIR}/*.c
                ${PROJECT_DIR}/*.cpp
            )
        endif()
    endif()
    bundle_project_files(BundleProjectFiles "${ProjectFiles}")
    set(${OUTPUT_LIST} ${BundleProjectFiles} PARENT_SCOPE)
endfunction()


# === Preprocessor definitions ===

if(WIN32)
    ADD_DEFINE(_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINE(_SCL_SECURE_NO_WARNINGS)
    ADD_DEFINE(UNICODE)
    ADD_DEFINE(NOMINMAX)
    if(MSVC)
        # Disable some warnings for MSVC compiler
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4251 /wd4290 /wd4103")
    endif(MSVC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DLLGL_DEBUG")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLLGL_DEBUG")
endif(WIN32)

ADD_DEBUG_DEFINE(LLGL_DEBUG)

if(LLGL_MOBILE_PLATFORM)
    ADD_DEFINE(LLGL_MOBILE_PLATFORM)
endif()


# === Initialize summary variables ===

set(SUMMARY_LIBRARY_TYPE "Unknown")
set(SUMMARY_TARGET_ARCH "Unknown")


# === Options ===

if(WIN32)
    set(LLGL_D3D11_ENABLE_FEATURELEVEL "Direct3D 11.0" CACHE STRING "Enable maximum Direct3D 11 feature level (requires <d3d11.h>, <d3d11_1.h>, <d3d11_2.h>, or <d3d11_3.h> header files")
    set_property(CACHE LLGL_D3D11_ENABLE_FEATURELEVEL PROPERTY STRINGS "Direct3D 11.0" "Direct3D 11.1" "Direct3D 11.2" "Direct3D 11.3")
elseif(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(LLGL_BUILD_64BIT 1)
else()
    set(LLGL_BUILD_64BIT 0)
endif()

option(LLGL_ENABLE_CHECKED_CAST "Enable dynamic checked cast (only in Debug mode)" ON)
option(LLGL_ENABLE_DEBUG_LAYER "Enable renderer debug layer (for both Debug and Release mode)" ON)
option(LLGL_ENABLE_SPIRV_REFLECT "Enable shader reflection of SPIR-V modules (requires the SPIRV submodule)" OFF)
option(LLGL_ENABLE_JIT_COMPILER "Enable Just-in-Time (JIT) compilation for emulated deferred command buffers (experimental)" OFF)
option(LLGL_ENABLE_EXCEPTIONS "Enable C++ exceptions" OFF)

option(LLGL_PREFER_STL_CONTAINERS "Prefers C++ STL containers over custom containers, e.g. std::vector over SmallVector<T>" OFF)

option(LLGL_GL_ENABLE_VENDOR_EXT "Enable vendor specific OpenGL extensions (e.g. GL_NV_..., GL_AMD_... etc.)" ON)
option(LLGL_GL_ENABLE_DSA_EXT "Enable OpenGL direct state access (DSA) extension if available" ON)
option(LLGL_GL_ENABLE_OPENGL2X "Enable support for OpenGL 2.x compatibility profile" OFF)
option(LLGL_GL_INCLUDE_EXTERNAL "Include additional OpenGL header files from 'external' folder" ON)

option(LLGL_BUILD_STATIC_LIB "Build LLGL as static lib (Only allows a single render system!)" OFF)
option(LLGL_BUILD_TESTS "Include test projects" OFF)
option(LLGL_BUILD_EXAMPLES "Include example projects" OFF)

option(LLGL_BUILD_RENDERER_NULL "Include Null renderer project" ON)

if(APPLE)
    option(LLGL_MACOS_ENABLE_COREVIDEO "Enable CoreVideo framework (for refresh rate of built-in displays)" ON)
endif()

if(LLGL_MOBILE_PLATFORM)
    option(LLGL_BUILD_RENDERER_OPENGLES3 "Include OpenGLES 3 renderer project" ON)
else()
    option(LLGL_BUILD_RENDERER_OPENGL "Include OpenGL renderer project" ON)
endif()

if(APPLE)
    option(LLGL_BUILD_RENDERER_METAL "Include Metal renderer project (experimental)" OFF)
endif()

if(NOT APPLE)
    option(LLGL_BUILD_RENDERER_VULKAN "Include Vulkan renderer project (experimental)" OFF)
endif()

if(WIN32)
    option(LLGL_BUILD_RENDERER_DIRECT3D11 "Include Direct3D11 renderer project" ON)
    option(LLGL_BUILD_RENDERER_DIRECT3D12 "Include Direct3D12 renderer project (experimental)" OFF)
    option(LLGL_D3D12_ENABLE_DXCOMPILER "Enable support for DirectXShaderCompiler (DXC); requires Windows Kit to be installed" OFF)
    option(LLGL_BUILD_WRAPPER_CSHARP "Include wrapper for C#" OFF)
endif()

option(LLGL_BUILD_WRAPPER_C99 "Include wrapper for C99" OFF)

if(LLGL_ENABLE_CHECKED_CAST)
    ADD_DEBUG_DEFINE(LLGL_ENABLE_CHECKED_CAST)
endif()

if(LLGL_ENABLE_DEBUG_LAYER)
    ADD_DEFINE(LLGL_ENABLE_DEBUG_LAYER)
endif()

if(LLGL_ENABLE_SPIRV_REFLECT)
    ADD_DEFINE(LLGL_ENABLE_SPIRV_REFLECT)
endif()

if(LLGL_ENABLE_JIT_COMPILER)
    ADD_DEFINE(LLGL_ENABLE_JIT_COMPILER)
endif()

if(LLGL_ENABLE_EXCEPTIONS)
    ADD_DEFINE(LLGL_ENABLE_EXCEPTIONS)
endif()

if(LLGL_GL_ENABLE_VENDOR_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_VENDOR_EXT)
endif()

if(LLGL_GL_ENABLE_DSA_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_DSA_EXT)
endif()

if(LLGL_GL_ENABLE_OPENGL2X)
    ADD_DEFINE(LLGL_GL_ENABLE_OPENGL2X)
endif()

if(LLGL_BUILD_STATIC_LIB)
    ADD_DEFINE(LLGL_BUILD_STATIC_LIB)
endif()

if(LLGL_PREFER_STL_CONTAINERS)
    ADD_DEFINE(LLGL_PREFER_STL_CONTAINERS)
endif()

if(LLGL_D3D12_ENABLE_DXCOMPILER)
    ADD_DEFINE(LLGL_D3D12_ENABLE_DXCOMPILER)
endif()

if(WIN32)
    if(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.3")
        ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=3)
    elseif(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.2")
        ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=2)
    elseif(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.1")
        ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=1)
    else()
        ADD_DEFINE(LLGL_D3D11_ENABLE_FEATURELEVEL=0)
    endif()
endif()

if(APPLE AND LLGL_BUILD_RENDERER_OPENGL)
    ADD_DEFINE(GL_SILENCE_DEPRECATION)
endif()

if(LLGL_MACOS_ENABLE_COREVIDEO)
    ADD_DEFINE(LLGL_MACOS_ENABLE_COREVIDEO)
endif()

if(LLGL_MOBILE_PLATFORM)
    set(ARCH_ARM64 ON)
    set(SUMMARY_TARGET_ARCH "ARM64")
elseif(APPLE OR LLGL_BUILD_64BIT)
    set(ARCH_AMD64 ON)
    set(SUMMARY_TARGET_ARCH "AMD64 (x86-x64)")
else()
    set(ARCH_IA32 ON)
    set(SUMMARY_TARGET_ARCH "IA-32 (x86)")
endif()


# === Global files ===

set(FilesMsvcNatvis ${PROJECT_SOURCE_DIR}/LLGL.natvis)

# Common files
file(GLOB FilesInclude                      ${PROJECT_INCLUDE_DIR}/LLGL/*.*)
file(GLOB FilesIncludeBackend               ${PROJECT_INCLUDE_DIR}/LLGL/Backend/*.inl)
file(GLOB FilesIncludeContainer             ${PROJECT_INCLUDE_DIR}/LLGL/Container/*.*)
file(GLOB FilesIncludeUtils                 ${PROJECT_INCLUDE_DIR}/LLGL/Utils/*.*)
file(GLOB FilesIncludePlatformBase          ${PROJECT_INCLUDE_DIR}/LLGL/Platform/*.*)
file(GLOB FilesCore                         ${PROJECT_SOURCE_DIR}/sources/Core/*.*)
file(GLOB FilesPlatformBase                 ${PROJECT_SOURCE_DIR}/sources/Platform/*.*)
file(GLOB FilesRenderer                     ${PROJECT_SOURCE_DIR}/sources/Renderer/*.*)

if(LLGL_ENABLE_JIT_COMPILER)
    file(GLOB FilesJIT                      ${PROJECT_SOURCE_DIR}/sources/JIT/*.*)
    if(ARCH_IA32)
        file(GLOB FilesJITArch              ${PROJECT_SOURCE_DIR}/sources/JIT/Arch/IA32/*.*)
    elseif(ARCH_AMD64)
        file(GLOB FilesJITArch              ${PROJECT_SOURCE_DIR}/sources/JIT/Arch/AMD64/*.*)
    elseif(ARCH_ARM64)
        file(GLOB FilesJITArch              ${PROJECT_SOURCE_DIR}/sources/JIT/Arch/ARM64/*.*)
    endif()
    if(WIN32)
        file(GLOB FilesJITPlatform          ${PROJECT_SOURCE_DIR}/sources/JIT/Platform/Win32/*.*)
    else()
        file(GLOB FilesJITPlatform          ${PROJECT_SOURCE_DIR}/sources/JIT/Platform/POSIX/*.*)
    endif()
endif()

if(LLGL_ENABLE_DEBUG_LAYER)
    file(GLOB FilesRendererDbg              ${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/*.*)
    file(GLOB FilesRendererDbgBuffer        ${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/Buffer/*.*)
    file(GLOB FilesRendererDbgRenderState   ${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/RenderState/*.*)
    file(GLOB FilesRendererDbgShader        ${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/Shader/*.*)
    file(GLOB FilesRendererDbgTexture       ${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/Texture/*.*)
endif()

if(WIN32)
    file(GLOB FilesPlatform                 ${PROJECT_SOURCE_DIR}/sources/Platform/Win32/*.*)
elseif(APPLE)
    if(LLGL_MOBILE_PLATFORM)
        file(GLOB FilesPlatform             ${PROJECT_SOURCE_DIR}/sources/Platform/IOS/*.*)
    else()
        file(GLOB FilesPlatform             ${PROJECT_SOURCE_DIR}/sources/Platform/MacOS/*.*)
    endif()
elseif(UNIX)
    if(LLGL_ANDROID_PLATFORM)
        file(GLOB FilesPlatform             ${PROJECT_SOURCE_DIR}/sources/Platform/Android/*.*)
    else()
        file(GLOB FilesPlatform             ${PROJECT_SOURCE_DIR}/sources/Platform/Linux/*.*)
    endif()
endif()

# Null renderer files
file(GLOB FilesRendererNull                 ${PROJECT_SOURCE_DIR}/sources/Renderer/Null/*.*)
file(GLOB FilesRendererNullBuffer           ${PROJECT_SOURCE_DIR}/sources/Renderer/Null/Buffer/*.*)
file(GLOB FilesRendererNullCommand          ${PROJECT_SOURCE_DIR}/sources/Renderer/Null/Command/*.*)
file(GLOB FilesRendererNullRenderState      ${PROJECT_SOURCE_DIR}/sources/Renderer/Null/RenderState/*.*)
file(GLOB FilesRendererNullShader           ${PROJECT_SOURCE_DIR}/sources/Renderer/Null/Shader/*.*)
file(GLOB FilesRendererNullTexture          ${PROJECT_SOURCE_DIR}/sources/Renderer/Null/Texture/*.*)

# OpenGL renderer files
file(GLOB FilesRendererGL                   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/*.*)
file(GLOB FilesRendererGLBuffer             ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Buffer/*.*)
file(GLOB FilesRendererGLCommand            ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Command/*.*)
file(GLOB FilesRendererGLExt                ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Ext/*.*)
file(GLOB FilesRendererGLPlatformBase       ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/*.*)
file(GLOB FilesRendererGLRenderState        ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/RenderState/*.*)
file(GLOB FilesRendererGLShader             ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/*.*)
file(GLOB FilesRendererGLTexture            ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Texture/*.*)
file(GLOB FilesRendererGLCoreProfile        ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/GLCoreProfile/*.*)
file(GLOB FilesRendererGLESProfile          ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/GLESProfile/*.*)

# Remove selected files if GL2X is disabled
if(NOT LLGL_GL_ENABLE_OPENGL2X)
    list(
        REMOVE_ITEM FilesRendererGLBuffer
        "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Buffer/GL2XVertexArray.cpp"
        "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Buffer/GL2XVertexArray.h"
    )
    list(
        REMOVE_ITEM FilesRendererGLTexture
        "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Texture/GL2XSampler.cpp"
        "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Texture/GL2XSampler.h"
    )
endif()

# Remove selected files for GLES3 renderer
set(FilesRendererGLES3Shader ${FilesRendererGLShader})
list(
    REMOVE_ITEM FilesRendererGLES3Shader
    "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/GLSeparableShader.cpp"
    "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/GLSeparableShader.h"
    "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/GLProgramPipeline.cpp"
    "${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/GLProgramPipeline.h"
)

if(WIN32)
    file(GLOB FilesRendererGLPlatform       ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Win32/*.*)
    file(GLOB FilesIncludePlatform          ${PROJECT_INCLUDE_DIR}/LLGL/Platform/Win32/*.*)
elseif(APPLE)
    if(LLGL_MOBILE_PLATFORM)
        file(GLOB FilesRendererGLPlatform   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/IOS/*.*)
        file(GLOB FilesIncludePlatform      ${PROJECT_INCLUDE_DIR}/LLGL/Platform/IOS/*.*)
    else()
        file(GLOB FilesRendererGLPlatform   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/MacOS/*.*)
        file(GLOB FilesIncludePlatform      ${PROJECT_INCLUDE_DIR}/LLGL/Platform/MacOS/*.*)
    endif()
elseif(UNIX)
    if(LLGL_ANDROID_PLATFORM)
        file(GLOB FilesRendererGLPlatform   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Android/*.*)
        file(GLOB FilesIncludePlatform      ${PROJECT_INCLUDE_DIR}/LLGL/Platform/Android/*.*)
    else()
        file(GLOB FilesRendererGLPlatform   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Linux/*.*)
        file(GLOB FilesIncludePlatform      ${PROJECT_INCLUDE_DIR}/LLGL/Platform/Linux/*.*)
    endif()
endif()

# SPIR-V renderer files
file(GLOB FilesRendererSPIRV                ${PROJECT_SOURCE_DIR}/sources/Renderer/SPIRV/*.*)

# Vulkan renderer files
file(GLOB FilesRendererVK                   ${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/*.*)
file(GLOB FilesRendererVKBuffer             ${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Buffer/*.*)
file(GLOB FilesRendererVKExt                ${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Ext/*.*)
file(GLOB FilesRendererVKMemory             ${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Memory/*.*)
file(GLOB FilesRendererVKRenderState        ${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/RenderState/*.*)
file(GLOB FilesRendererVKShader             ${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Shader/*.*)
file(GLOB FilesRendererVKTexture            ${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Texture/*.*)

# Metal renderer files
file(GLOB FilesRendererMTL                  ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/*.*)
file(GLOB FilesRendererMTLBuffer            ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Buffer/*.*)
file(GLOB FilesRendererMTLCommand           ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Command/*.*)
file(GLOB FilesRendererMTLRenderState       ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/RenderState/*.*)
file(GLOB FilesRendererMTLShader            ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Shader/*.*)
file(GLOB FilesRendererMTLTexture           ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Texture/*.*)

set(
    FilesRendererMTLShaderBuiltin
    ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Shader/Builtin/MTBuiltin.h
    ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Shader/Builtin/MTBuiltin.mm
)

# Direct3D common renderer files
file(GLOB FilesRendererDXCommon             ${PROJECT_SOURCE_DIR}/sources/Renderer/DXCommon/*.*)
if(LLGL_D3D12_ENABLE_DXCOMPILER)
    file(GLOB FilesRendererDXCommonDXC      ${PROJECT_SOURCE_DIR}/sources/Renderer/DXCommon/DXC/*.*)
endif()

# Direct3D12 renderer files
file(GLOB FilesRendererD3D12                ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/*.*)
file(GLOB FilesRendererD3D12Buffer          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Buffer/*.*)
file(GLOB FilesRendererD3D12Command         ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Command/*.*)
file(GLOB FilesRendererD3D12RenderState     ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/RenderState/*.*)
file(GLOB FilesRendererD3D12Shader          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Shader/*.*)
file(GLOB FilesRendererD3D12Texture         ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Texture/*.*)

set(
    FilesRendererD3D12ShaderBuiltin
    ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Shader/Builtin/D3D12Builtin.h
)

# Direct3D11 renderer files
file(GLOB FilesRendererD3D11                ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/*.*)
file(GLOB FilesRendererD3D11Buffer          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Buffer/*.*)
file(GLOB FilesRendererD3D11RenderState     ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/RenderState/*.*)
file(GLOB FilesRendererD3D11Shader          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Shader/*.*)
file(GLOB FilesRendererD3D11Texture         ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Texture/*.*)

set(
    FilesRendererD3D11ShaderBuiltin
    ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Shader/Builtin/D3D11Builtin.h
)

# Test project files
find_project_source_files( FilesTest_BlendStates        "${TEST_PROJECTS_DIR}/Test_BlendStates.cpp"     )
find_project_source_files( FilesTest_Compute            "${TEST_PROJECTS_DIR}/Test_Compute.cpp"         )
find_project_source_files( FilesTest_Container          "${TEST_PROJECTS_DIR}/Test_Container.cpp"       )
find_project_source_files( FilesTest_D3D12              "${TEST_PROJECTS_DIR}/Test_D3D12.cpp"           )
find_project_source_files( FilesTest_Display            "${TEST_PROJECTS_DIR}/Test_Display.cpp"         )
find_project_source_files( FilesTest_Image              "${TEST_PROJECTS_DIR}/Test_Image.cpp"           )
find_project_source_files( FilesTest_JIT                "${TEST_PROJECTS_DIR}/Test_JIT.cpp"             )
find_project_source_files( FilesTest_Metal              "${TEST_PROJECTS_DIR}/Test_Metal.cpp"           )
find_project_source_files( FilesTest_OpenGL             "${TEST_PROJECTS_DIR}/Test_OpenGL.cpp"          )
find_project_source_files( FilesTest_Performance        "${TEST_PROJECTS_DIR}/Test_Performance.cpp"     )
find_project_source_files( FilesTest_ShaderReflect      "${TEST_PROJECTS_DIR}/Test_ShaderReflect.cpp"   )
find_project_source_files( FilesTest_SeparateShaders    "${TEST_PROJECTS_DIR}/Test_SeparateShaders.cpp" )
find_project_source_files( FilesTest_Vulkan             "${TEST_PROJECTS_DIR}/Test_Vulkan.cpp"          )
find_project_source_files( FilesTest_Window             "${TEST_PROJECTS_DIR}/Test_Window.cpp"          )

find_project_source_files( FilesTestbed                 "${TEST_PROJECTS_DIR}/Testbed"                  )

# Example project files
file(GLOB FilesExampleBase ${EXAMPLEBASE_PROJECT_DIR}/*.*)

if(LLGL_ANDROID_PLATFORM)
    set(ANDROID_APP_GLUE_DIR "$ENV{ANDROID_NDK_HOME}/sources/android/native_app_glue")
    set(FilesAndroidNativeAppGlue "${ANDROID_APP_GLUE_DIR}/android_native_app_glue.c" "${ANDROID_APP_GLUE_DIR}/android_native_app_glue.h")
elseif(LLGL_IOS_PLATFORM)
    file(
        GLOB FilesExampleBaseIOS
        ${EXAMPLEBASE_PROJECT_DIR}/iOS/*.h
        ${EXAMPLEBASE_PROJECT_DIR}/iOS/*.mm
        ${EXAMPLEBASE_PROJECT_DIR}/iOS/*.m
    )
endif()

if(APPLE)
    set_source_files_properties(${EXAMPLEBASE_PROJECT_DIR}/ExampleBase.cpp PROPERTIES COMPILE_FLAGS -xobjective-c++)
endif()

find_project_source_files( FilesExample_Animation        "${EXAMPLE_CPP_PROJECTS_DIR}/Animation"        )
find_project_source_files( FilesExample_BufferArray      "${EXAMPLE_CPP_PROJECTS_DIR}/BufferArray"      )
find_project_source_files( FilesExample_ClothPhysics     "${EXAMPLE_CPP_PROJECTS_DIR}/ClothPhysics"     )
find_project_source_files( FilesExample_ComputeShader    "${EXAMPLE_CPP_PROJECTS_DIR}/ComputeShader"    )
find_project_source_files( FilesExample_Fonts            "${EXAMPLE_CPP_PROJECTS_DIR}/Fonts"            )
find_project_source_files( FilesExample_HelloTriangle    "${EXAMPLE_CPP_PROJECTS_DIR}/HelloTriangle"    )
find_project_source_files( FilesExample_Instancing       "${EXAMPLE_CPP_PROJECTS_DIR}/Instancing"       )
find_project_source_files( FilesExample_Mapping          "${EXAMPLE_CPP_PROJECTS_DIR}/Mapping"          )
find_project_source_files( FilesExample_MultiContext     "${EXAMPLE_CPP_PROJECTS_DIR}/MultiContext"     )
find_project_source_files( FilesExample_MultiRenderer    "${EXAMPLE_CPP_PROJECTS_DIR}/MultiRenderer"    )
find_project_source_files( FilesExample_MultiThreading   "${EXAMPLE_CPP_PROJECTS_DIR}/MultiThreading"   )
find_project_source_files( FilesExample_PBR              "${EXAMPLE_CPP_PROJECTS_DIR}/PBR"              )
find_project_source_files( FilesExample_PostProcessing   "${EXAMPLE_CPP_PROJECTS_DIR}/PostProcessing"   )
find_project_source_files( FilesExample_PrimitiveRestart "${EXAMPLE_CPP_PROJECTS_DIR}/PrimitiveRestart" )
find_project_source_files( FilesExample_Queries          "${EXAMPLE_CPP_PROJECTS_DIR}/Queries"          )
find_project_source_files( FilesExample_RenderTarget     "${EXAMPLE_CPP_PROJECTS_DIR}/RenderTarget"     )
find_project_source_files( FilesExample_ResourceBinding  "${EXAMPLE_CPP_PROJECTS_DIR}/ResourceBinding"  )
find_project_source_files( FilesExample_ShadowMapping    "${EXAMPLE_CPP_PROJECTS_DIR}/ShadowMapping"    )
find_project_source_files( FilesExample_StencilBuffer    "${EXAMPLE_CPP_PROJECTS_DIR}/StencilBuffer"    )
find_project_source_files( FilesExample_StreamOutput     "${EXAMPLE_CPP_PROJECTS_DIR}/StreamOutput"     )
find_project_source_files( FilesExample_Tessellation     "${EXAMPLE_CPP_PROJECTS_DIR}/Tessellation"     )
find_project_source_files( FilesExample_Texturing        "${EXAMPLE_CPP_PROJECTS_DIR}/Texturing"        )
find_project_source_files( FilesExample_UnorderedAccess  "${EXAMPLE_CPP_PROJECTS_DIR}/UnorderedAccess"  )
find_project_source_files( FilesExample_VolumeRendering  "${EXAMPLE_CPP_PROJECTS_DIR}/VolumeRendering"  )

find_project_source_files( FilesExampleC99_HelloTriangle "${EXAMPLE_C99_PROJECTS_DIR}/HelloTriangle"    )

# Append extra resources for mobile platforms
if(LLGL_IOS_PLATFORM)
    list(
        APPEND FilesExample_Fonts
        "${EXAMPLE_MEDIA_DIR}/Textures/FontAtlas_CourierNew_Bold_16.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/FontAtlas_LucidaConsole_Bold_32.png"
    )
    list(
        APPEND FilesExample_Texturing
        "${EXAMPLE_MEDIA_DIR}/Textures/Crate-DXT1-MipMapped.dds"
        "${EXAMPLE_MEDIA_DIR}/Textures/Crate.jpg"
    )
    list(
        APPEND FilesExample_PostProcessing
        "${EXAMPLE_MEDIA_DIR}/Models/WiredBox.obj"
    )
    list(
        APPEND FilesExample_ClothPhysics
        "${EXAMPLE_MEDIA_DIR}/Textures/Logo_LLGL.png"
    )
    list(
        APPEND FilesExample_RenderTarget
        "${EXAMPLE_MEDIA_DIR}/Textures/Crate.jpg"
    )
    list(
        APPEND FilesExample_ResourceBinding
        "${EXAMPLE_MEDIA_DIR}/Textures/Crate.jpg"
        "${EXAMPLE_MEDIA_DIR}/Textures/TilesGray512.jpg"
        "${EXAMPLE_MEDIA_DIR}/Textures/TilesBlue512.jpg"
        "${EXAMPLE_MEDIA_DIR}/Models/UVSphere.obj"
    )
    list(
        APPEND FilesExample_StencilBuffer
        "${EXAMPLE_MEDIA_DIR}/Models/Portal-Scene.obj"
        "${EXAMPLE_MEDIA_DIR}/Models/Portal-Stencil.obj"
        "${EXAMPLE_MEDIA_DIR}/Models/WiredBox.obj"
        "${EXAMPLE_MEDIA_DIR}/Models/Pyramid.obj"
    )
    list(
        APPEND FilesExample_ShadowMapping
        "${EXAMPLE_MEDIA_DIR}/Models/SimpleRoom.obj"
        "${EXAMPLE_MEDIA_DIR}/Models/WiredBox.obj"
    )
    list(
        APPEND FilesExample_Animation
        "${EXAMPLE_MEDIA_DIR}/Models/PenroseStairs-Top.obj"
        "${EXAMPLE_MEDIA_DIR}/Models/PenroseStairs-Bottom.obj"
        "${EXAMPLE_MEDIA_DIR}/Models/IcoSphere.obj"
        "${EXAMPLE_MEDIA_DIR}/Textures/TilesGray512.jpg"
    )
    list(
        APPEND FilesExample_VolumeRendering
        "${EXAMPLE_MEDIA_DIR}/Models/Suzanne.obj"
    )
    list(
        APPEND FilesExample_Instancing
        "${EXAMPLE_MEDIA_DIR}/Textures/Grass.jpg"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_0.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_1.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_2.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_3.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_4.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_5.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_6.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_7.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_8.png"
        "${EXAMPLE_MEDIA_DIR}/Textures/Plants_9.png"
    )
endif()


# === Source group folders ===

source_group("NatVis" FILES ${FilesMsvcNatvis})

source_group("Include" FILES ${FilesInclude})
source_group("Include\\Backend" FILES ${FilesIncludeBackend})
source_group("Include\\Container" FILES ${FilesIncludeContainer})
source_group("Include\\Utils" FILES ${FilesIncludeUtils})
source_group("Sources\\Core" FILES ${FilesCore})

if(LLGL_ENABLE_JIT_COMPILER)
    source_group("Sources\\JIT" FILES ${FilesJIT})
    source_group("Sources\\JIT\\Arch" FILES ${FilesJITArch})
    source_group("Sources\\JIT\\Platform" FILES ${FilesJITPlatform})
endif()

source_group("Include\\Platform" FILES ${FilesIncludePlatformBase} ${FilesIncludePlatform})
source_group("Sources\\Platform" FILES ${FilesPlatformBase} ${FilesPlatform})
source_group("Sources\\Renderer" FILES ${FilesRenderer})

if(LLGL_ENABLE_DEBUG_LAYER)
    source_group("Sources\\Renderer\\DebugLayer" FILES ${FilesRendererDbg})
    source_group("Sources\\Renderer\\DebugLayer\\Buffer" FILES ${FilesRendererDbgBuffer})
    source_group("Sources\\Renderer\\DebugLayer\\RenderState" FILES ${FilesRendererDbgRenderState})
    source_group("Sources\\Renderer\\DebugLayer\\Shader" FILES ${FilesRendererDbgShader})
    source_group("Sources\\Renderer\\DebugLayer\\Texture" FILES ${FilesRendererDbgTexture})
endif()

source_group("Null" FILES ${FilesRendererNull})
source_group("Null\\Buffer" FILES ${FilesRendererNullBuffer})
source_group("Null\\Command" FILES ${FilesRendererNullCommand})
source_group("Null\\RenderState" FILES ${FilesRendererNullRenderState})
source_group("Null\\Shader" FILES ${FilesRendererNullShader})
source_group("Null\\Texture" FILES ${FilesRendererNullTexture})

source_group("OpenGL" FILES ${FilesRendererGL})
source_group("OpenGL\\Buffer" FILES ${FilesRendererGLBuffer})
source_group("OpenGL\\Command" FILES ${FilesRendererGLCommand})
source_group("OpenGL\\Ext" FILES ${FilesRendererGLExt})
source_group("OpenGL\\Platform" FILES ${FilesRendererGLPlatformBase} ${FilesRendererGLPlatform})
source_group("OpenGL\\RenderState" FILES ${FilesRendererGLRenderState})
source_group("OpenGL\\Shader" FILES ${FilesRendererGLShader})
source_group("OpenGL\\Texture" FILES ${FilesRendererGLTexture})
source_group("OpenGL\\GLCoreProfile" FILES ${FilesRendererGLCoreProfile})
source_group("OpenGL\\GLESProfile" FILES ${FilesRendererGLESProfile})

source_group("SPIRV" FILES ${FilesRendererSPIRV})

source_group("Vulkan" FILES ${FilesRendererVK})
source_group("Vulkan\\Buffer" FILES ${FilesRendererVKBuffer})
source_group("Vulkan\\Ext" FILES ${FilesRendererVKExt})
source_group("Vulkan\\Memory" FILES ${FilesRendererVKMemory})
source_group("Vulkan\\RenderState" FILES ${FilesRendererVKRenderState})
source_group("Vulkan\\Shader" FILES ${FilesRendererVKShader})
source_group("Vulkan\\Texture" FILES ${FilesRendererVKTexture})

source_group("Metal" FILES ${FilesRendererMTL})
source_group("Metal\\Buffer" FILES ${FilesRendererMTLBuffer})
source_group("Metal\\Command" FILES ${FilesRendererMTLCommand})
source_group("Metal\\RenderState" FILES ${FilesRendererMTLRenderState})
source_group("Metal\\Shader" FILES ${FilesRendererMTLShader})
source_group("Metal\\Shader\\Bulitin" FILES ${FilesRendererMTLShaderBuiltin})
source_group("Metal\\Texture" FILES ${FilesRendererMTLTexture})

source_group("DXCommon" FILES ${FilesRendererDXCommon})
source_group("DXCommon\\DXC" FILES ${FilesRendererDXCommonDXC})

source_group("Direct3D11" FILES ${FilesRendererD3D11})
source_group("Direct3D11\\Buffer" FILES ${FilesRendererD3D11Buffer})
source_group("Direct3D11\\RenderState" FILES ${FilesRendererD3D11RenderState})
source_group("Direct3D11\\Shader" FILES ${FilesRendererD3D11Shader})
source_group("Direct3D11\\Shader\\Builtin" FILES ${FilesRendererD3D11ShaderBuiltin})
source_group("Direct3D11\\Texture" FILES ${FilesRendererD3D11Texture})

source_group("Direct3D12" FILES ${FilesRendererD3D12})
source_group("Direct3D12\\Buffer" FILES ${FilesRendererD3D12Buffer})
source_group("Direct3D12\\Command" FILES ${FilesRendererD3D12Command})
source_group("Direct3D12\\RenderState" FILES ${FilesRendererD3D12RenderState})
source_group("Direct3D12\\Shader" FILES ${FilesRendererD3D12Shader})
source_group("Direct3D12\\Shader\\Builtin" FILES ${FilesRendererD3D12ShaderBuiltin})
source_group("Direct3D12\\Texture" FILES ${FilesRendererD3D12Texture})

source_group("Sources" FILES ${FilesExampleBase})

if(LLGL_ANDROID_PLATFORM)
    source_group("android_native_app_glue" FILES ${FilesAndroidNativeAppGlue})
elseif(LLGL_IOS_PLATFORM)
    source_group("Sources\\iOS" FILES ${FilesExampleBaseIOS})
endif()


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")

include(cmake/FindGaussianLib.cmake)

if(LLGL_BUILD_EXAMPLES)
    include_directories("${EXAMPLEBASE_PROJECT_DIR}")
endif()


# === Projects ===

set(
    FilesLLGL
    ${FilesInclude}
    ${FilesIncludeBackend}
    ${FilesIncludeContainer}
    ${FilesIncludeUtils}
    ${FilesIncludePlatformBase}
    ${FilesIncludePlatform}
    ${FilesCore}
    ${FilesPlatformBase}
    ${FilesPlatform}
    ${FilesRenderer}
)

set(FilesExampleBaseAll ${FilesExampleBase})

if(LLGL_IOS_PLATFORM)
    set(FilesExampleBaseAll ${FilesExampleBaseAll} ${FilesExampleBaseIOS})
endif()

if(MSVC)
    set(FilesLLGL ${FilesLLGL} ${FilesMsvcNatvis})
endif()

if(LLGL_ENABLE_JIT_COMPILER)
    set(
        FilesLLGL
        ${FilesLLGL}
        ${FilesJIT}
        ${FilesJITArch}
        ${FilesJITPlatform}
    )
endif()

if(LLGL_ENABLE_DEBUG_LAYER)
    set(
        FilesLLGL
        ${FilesLLGL}
        ${FilesRendererDbg}
        ${FilesRendererDbgBuffer}
        ${FilesRendererDbgRenderState}
        ${FilesRendererDbgShader}
        ${FilesRendererDbgTexture}
    )
endif()

set(
    FilesNull
    ${FilesRendererNull}
    ${FilesRendererNullBuffer}
    ${FilesRendererNullCommand}
    ${FilesRendererNullRenderState}
    ${FilesRendererNullShader}
    ${FilesRendererNullTexture}
)

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
    ${FilesRendererGLCoreProfile}
)

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
    ${FilesRendererGLESProfile}
)

set(
    FilesVK
    ${FilesRendererVK}
    ${FilesRendererVKBuffer}
    ${FilesRendererVKExt}
    ${FilesRendererVKMemory}
    ${FilesRendererVKRenderState}
    ${FilesRendererVKShader}
    ${FilesRendererVKTexture}
)

set(
    FilesMTL
    ${FilesRendererMTL}
    ${FilesRendererMTLBuffer}
    ${FilesRendererMTLCommand}
    ${FilesRendererMTLRenderState}
    ${FilesRendererMTLShader}
    ${FilesRendererMTLShaderBuiltin}
    ${FilesRendererMTLTexture}
)

if(LLGL_ENABLE_SPIRV_REFLECT)
    set(FilesVK ${FilesVK} ${FilesRendererSPIRV})
endif()

set(
    FilesD3D12
    ${FilesRendererD3D12}
    ${FilesRendererD3D12Buffer}
    ${FilesRendererD3D12Command}
    ${FilesRendererD3D12Shader}
    ${FilesRendererD3D12ShaderBuiltin}
    ${FilesRendererD3D12RenderState}
    ${FilesRendererD3D12Texture}
    ${FilesRendererDXCommon}
)

if(LLGL_D3D12_ENABLE_DXCOMPILER)
    set(FilesD3D12 ${FilesD3D12} ${FilesRendererDXCommonDXC})
endif()

set(
    FilesD3D11
    ${FilesRendererD3D11}
    ${FilesRendererD3D11Buffer}
    ${FilesRendererD3D11Shader}
    ${FilesRendererD3D11ShaderBuiltin}
    ${FilesRendererD3D11RenderState}
    ${FilesRendererD3D11Texture}
    ${FilesRendererDXCommon}
)

# Wrapper: C99
if(LLGL_BUILD_WRAPPER_C99)
    file(GLOB FilesWrapperC99 ${PROJECT_SOURCE_DIR}/wrapper/C99/*.*)
    file(GLOB FilesIncludeC99 ${PROJECT_INCLUDE_DIR}/LLGL-C/*.*)
    source_group("Wrapper\\C99\\Include" FILES ${FilesIncludeC99})
    source_group("Wrapper\\C99\\Sources" FILES ${FilesWrapperC99})
    set(FilesLLGL ${FilesLLGL} ${FilesIncludeC99} ${FilesWrapperC99})
endif()

# Base project
if(LLGL_BUILD_STATIC_LIB)
    set(SUMMARY_LIBRARY_TYPE "Static")
    add_library(LLGL STATIC ${FilesLLGL})
else()
    set(SUMMARY_LIBRARY_TYPE "Shared")
    add_library(LLGL SHARED ${FilesLLGL})
endif()
set(LLGL_ALL_TARGETS LLGL)

include(GNUInstallDirs)
target_include_directories(LLGL INTERFACE
                           $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(LLGL_ANDROID_PLATFORM)
    target_link_libraries(LLGL android log)
elseif(LLGL_IOS_PLATFORM)
    target_link_libraries(LLGL PUBLIC "-framework Foundation -framework UIKit")
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(LLGL ${COCOA_LIBRARY})
    if(LLGL_MACOS_ENABLE_COREVIDEO)
        target_link_libraries(LLGL "-framework CoreVideo")
    endif()
elseif(UNIX)
    target_link_libraries(LLGL X11 pthread Xrandr)
endif()

SET_RENDERER_PROJECT_PROPERTIES(LLGL)

set(LLGL_DEPENDENCIES LLGL)

if(LLGL_BUILD_RENDERER_NULL)
    # Null Renderer
    if(LLGL_BUILD_STATIC_LIB)
        add_library(LLGL_Null STATIC ${FilesNull})
        set(LLGL_DEPENDENCIES ${LLGL_DEPENDENCIES} LLGL_Null)
    else()
        add_library(LLGL_Null SHARED ${FilesNull})
    endif()
    
    SET_RENDERER_PROJECT_PROPERTIES(LLGL_Null)
    target_link_libraries(LLGL_Null LLGL ${OPENGL_LIBRARIES})
    
    ADD_DEFINE(LLGL_BUILD_RENDERER_NULL)

    list(APPEND LLGL_ALL_TARGETS LLGL_Null)
endif()

if(LLGL_BUILD_RENDERER_OPENGLES3)
    # OpenGLES Renderer
    include(cmake/FindOpenGLES3.cmake)
    if(OPENGLES_FOUND)
        include_directories(${OPENGLES_INCLUDE_DIR})
        
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_OpenGLES3 STATIC ${FilesGLES3})
            set(LLGL_DEPENDENCIES ${LLGL_DEPENDENCIES} LLGL_OpenGLES3)
        else()
            add_library(LLGL_OpenGLES3 SHARED ${FilesGLES3})
        endif()
        
        SET_RENDERER_PROJECT_PROPERTIES(LLGL_OpenGLES3)
        
        if(APPLE)
            target_link_libraries(LLGL_OpenGLES3 LLGL ${OPENGLES_LIBRARIES} "-framework Foundation -framework UIKit -framework QuartzCore -framework OpenGLES -framework GLKit")
            ADD_DEFINE(GLES_SILENCE_DEPRECATION)
        else()
            target_link_libraries(LLGL_OpenGLES3 LLGL ${OPENGLES_LIBRARIES} EGL)
        endif()
        
        ADD_DEFINE(LLGL_BUILD_RENDERER_OPENGLES3)
        ADD_PROJECT_DEFINE(LLGL_OpenGLES3 LLGL_OPENGLES3)

        list(APPEND LLGL_ALL_TARGETS LLGL_OpenGLES3)
    else()
        message("Missing OpenGLES -> LLGL_OpenGLES3 renderer will be excluded from project")
    endif()
endif()

if(LLGL_BUILD_RENDERER_OPENGL)
    # OpenGL Renderer
    set(OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL REQUIRED)
    if(OpenGL_FOUND)
        include_directories(${OPENGL_INCLUDE_DIR})
        
        if(LLGL_GL_INCLUDE_EXTERNAL)
            include_directories("${PROJECT_SOURCE_DIR}/external/OpenGL/include")
        endif()
        
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_OpenGL STATIC ${FilesGL})
            set(LLGL_DEPENDENCIES ${LLGL_DEPENDENCIES} LLGL_OpenGL)
        else()
            add_library(LLGL_OpenGL SHARED ${FilesGL})
        endif()
        
        SET_RENDERER_PROJECT_PROPERTIES(LLGL_OpenGL)
        target_link_libraries(LLGL_OpenGL LLGL ${OPENGL_LIBRARIES})
        
        ADD_DEFINE(LLGL_BUILD_RENDERER_OPENGL)
        ADD_PROJECT_DEFINE(LLGL_OpenGL LLGL_OPENGL)

        list(APPEND LLGL_ALL_TARGETS LLGL_OpenGL)
    else()
        message("Missing OpenGL -> LLGL_OpenGL renderer will be excluded from project")
    endif()
endif()

if(NOT APPLE AND LLGL_BUILD_RENDERER_VULKAN)
    # Vulkan Renderer
    find_package(Vulkan REQUIRED)
    if(Vulkan_FOUND)
        include_directories(${Vulkan_INCLUDE_DIR})
        
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_Vulkan STATIC ${FilesVK})
            set(LLGL_DEPENDENCIES ${LLGL_DEPENDENCIES} LLGL_Vulkan)
        else()
            add_library(LLGL_Vulkan SHARED ${FilesVK})
        endif()
        
        SET_RENDERER_PROJECT_PROPERTIES(LLGL_Vulkan)
        target_link_libraries(LLGL_Vulkan LLGL ${Vulkan_LIBRARY})
        
        ADD_DEFINE(LLGL_BUILD_RENDERER_VULKAN)
        
        list(APPEND LLGL_ALL_TARGETS LLGL_Vulkan)
    else()
        message("Missing Vulkan -> LLGL_Vulkan renderer will be excluded from project")
    endif()
    
    if(LLGL_ENABLE_SPIRV_REFLECT)
        # SPIRV Submodule
        include_directories("${PROJECT_SOURCE_DIR}/external/SPIRV-Headers/include")
    endif()
endif()

if(APPLE AND LLGL_BUILD_RENDERER_METAL)
    # Metal Renderer
    include(cmake/FindMetal.cmake)
    include(cmake/FindMetalKit.cmake)
    if(METAL_FOUND AND METALKIT_FOUND)
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_Metal STATIC ${FilesMTL})
            set(LLGL_DEPENDENCIES ${LLGL_DEPENDENCIES} LLGL_Metal)
        else()
            add_library(LLGL_Metal SHARED ${FilesMTL})
        endif()
        
        SET_RENDERER_PROJECT_PROPERTIES(LLGL_Metal)
        
        if(LLGL_MOBILE_PLATFORM)
            target_link_libraries(LLGL_Metal PUBLIC LLGL "-framework Foundation -framework UIKit -framework QuartzCore -framework Metal -framework MetalKit")
        else()
            target_link_libraries(LLGL_Metal LLGL ${METAL_LIBRARY} ${METALKIT_LIBRARY})
        endif()
        
        ADD_DEFINE(LLGL_BUILD_RENDERER_METAL)
        
        list(APPEND LLGL_ALL_TARGETS LLGL_Metal)
    else()
        message("Missing Metal/MetalKit -> LLGL_Metal renderer will be excluded from project")
    endif()
endif()

if(WIN32)
    if(LLGL_BUILD_RENDERER_DIRECT3D11)
        # Direct3D 11 Renderer
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_Direct3D11 STATIC ${FilesD3D11})
            set(LLGL_DEPENDENCIES ${LLGL_DEPENDENCIES} LLGL_Direct3D11)
        else()
            add_library(LLGL_Direct3D11 SHARED ${FilesD3D11})
        endif()
        
        SET_RENDERER_PROJECT_PROPERTIES(LLGL_Direct3D11)
        target_link_libraries(LLGL_Direct3D11 LLGL d3d11 dxgi D3DCompiler)
        
        ADD_DEFINE(LLGL_BUILD_RENDERER_DIRECT3D11)
        
        list(APPEND LLGL_ALL_TARGETS LLGL_Direct3D11)
    endif()
    
    if(LLGL_BUILD_RENDERER_DIRECT3D12)
        # Direct3D 12 Renderer
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_Direct3D12 STATIC ${FilesD3D12})
            set(LLGL_DEPENDENCIES ${LLGL_DEPENDENCIES} LLGL_Direct3D12)
        else()
            add_library(LLGL_Direct3D12 SHARED ${FilesD3D12})
        endif()
        
        SET_RENDERER_PROJECT_PROPERTIES(LLGL_Direct3D12)
        target_link_libraries(LLGL_Direct3D12 LLGL d3d12 dxgi D3DCompiler)

        ADD_DEFINE(LLGL_BUILD_RENDERER_DIRECT3D12)
        
        list(APPEND LLGL_ALL_TARGETS LLGL_Direct3D12)
    endif()
endif()

# Test Projects
if(GaussLib_INCLUDE_DIR)
    # Test Projects
    if(LLGL_BUILD_TESTS AND NOT LLGL_MOBILE_PLATFORM)
        # Platform specific test projects
        ADD_EXAMPLE_PROJECT(Test_OpenGL CXX "${FilesTest_OpenGL}" "${LLGL_DEPENDENCIES}")
        if(WIN32)
            ADD_EXAMPLE_PROJECT(Test_D3D12 CXX "${FilesTest_D3D12}" "${LLGL_DEPENDENCIES}")
        endif()
        if(APPLE)
            ADD_EXAMPLE_PROJECT(Test_Metal CXX "${FilesTest_Metal}" "${LLGL_DEPENDENCIES}")
        elseif(LLGL_BUILD_RENDERER_VULKAN AND VULKAN_FOUND)
            ADD_EXAMPLE_PROJECT(Test_Vulkan CXX "${FilesTest_Vulkan}" "${LLGL_DEPENDENCIES}")
        endif()
        
        # Common tests
        ADD_EXAMPLE_PROJECT(Test_BlendStates        CXX "${FilesTest_BlendStates}"      "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_Compute            CXX "${FilesTest_Compute}"          "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_Container          CXX "${FilesTest_Container}"        "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_Display            CXX "${FilesTest_Display}"          "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_Image              CXX "${FilesTest_Image}"            "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_JIT                CXX "${FilesTest_JIT}"              "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_Performance        CXX "${FilesTest_Performance}"      "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_SeparateShaders    CXX "${FilesTest_SeparateShaders}"  "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_ShaderReflect      CXX "${FilesTest_ShaderReflect}"    "${LLGL_DEPENDENCIES}")
        ADD_EXAMPLE_PROJECT(Test_Window             CXX "${FilesTest_Window}"           "${LLGL_DEPENDENCIES}")
        
        # Testbed
        ADD_EXAMPLE_PROJECT(Testbed                 CXX "${FilesTestbed}"               "${LLGL_DEPENDENCIES}")
    endif()

    # Example Projects
    if(LLGL_BUILD_EXAMPLES)
        # Base project (static lib) for all examples
        add_library(ExampleBase STATIC ${FilesExampleBaseAll})
        set_target_properties(ExampleBase PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(ExampleBase "${LLGL_DEPENDENCIES}")
        set_target_properties(ExampleBase PROPERTIES FOLDER "Examples")
        
        # Native "app glue" for Android examples
        if(LLGL_ANDROID_PLATFORM)
            add_library(android_native_app_glue STATIC ${FilesAndroidNativeAppGlue})
            target_link_libraries(ExampleBase android_native_app_glue)
            include_directories(${ANDROID_APP_GLUE_DIR})
        endif(LLGL_ANDROID_PLATFORM)
        
        set(EXAMPLE_PROJECT_LIBS ExampleBase ${LLGL_DEPENDENCIES})
        
        if(LLGL_ANDROID_PLATFORM)
            # Android specific examples
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
        endif()

        # Common examples
        ADD_EXAMPLE_PROJECT(Example_Animation           CXX "${FilesExample_Animation}"         "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_BufferArray         CXX "${FilesExample_BufferArray}"       "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_ClothPhysics        CXX "${FilesExample_ClothPhysics}"      "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_ComputeShader       CXX "${FilesExample_ComputeShader}"     "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_Fonts               CXX "${FilesExample_Fonts}"             "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_HelloTriangle       CXX "${FilesExample_HelloTriangle}"     "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_Instancing          CXX "${FilesExample_Instancing}"        "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_Mapping             CXX "${FilesExample_Mapping}"           "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_MultiContext        CXX "${FilesExample_MultiContext}"      "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_MultiRenderer       CXX "${FilesExample_MultiRenderer}"     "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_MultiThreading      CXX "${FilesExample_MultiThreading}"    "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_PBR                 CXX "${FilesExample_PBR}"               "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_PostProcessing      CXX "${FilesExample_PostProcessing}"    "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_PrimitiveRestart    CXX "${FilesExample_PrimitiveRestart}"  "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_Queries             CXX "${FilesExample_Queries}"           "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_RenderTarget        CXX "${FilesExample_RenderTarget}"      "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_ResourceBinding     CXX "${FilesExample_ResourceBinding}"   "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_ShadowMapping       CXX "${FilesExample_ShadowMapping}"     "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_StencilBuffer       CXX "${FilesExample_StencilBuffer}"     "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_StreamOutput        CXX "${FilesExample_StreamOutput}"      "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_Tessellation        CXX "${FilesExample_Tessellation}"      "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_Texturing           CXX "${FilesExample_Texturing}"         "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_UnorderedAccess     CXX "${FilesExample_UnorderedAccess}"   "${EXAMPLE_PROJECT_LIBS}")
        ADD_EXAMPLE_PROJECT(Example_VolumeRendering     CXX "${FilesExample_VolumeRendering}"   "${EXAMPLE_PROJECT_LIBS}")

        if (LLGL_BUILD_WRAPPER_C99)
            # C99 wrapper examples
            ADD_EXAMPLE_PROJECT(Example_C99_HelloTriangle C "${FilesExampleC99_HelloTriangle}" "${EXAMPLE_PROJECT_LIBS}")
        endif(LLGL_BUILD_WRAPPER_C99)
    endif(LLGL_BUILD_EXAMPLES)
else(GaussLib_INCLUDE_DIR)
    # If GaussianLib is not available, tests and examples cannot be enabled
    if(LLGL_BUILD_TESTS)
        message(SEND_ERROR "LLGL_BUILD_TESTS is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
    if(LLGL_BUILD_EXAMPLES)
        message(SEND_ERROR "LLGL_BUILD_EXAMPLES is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
endif(GaussLib_INCLUDE_DIR)

# Wrapper: C#
if(WIN32 AND LLGL_BUILD_WRAPPER_CSHARP)
    add_subdirectory(wrapper/CSharp)
endif()

# Install targets, headers, and CMake config files
install(TARGETS ${LLGL_ALL_TARGETS} EXPORT LLGLTargets RUNTIME LIBRARY ARCHIVE)
install(DIRECTORY "${PROJECT_INCLUDE_DIR}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(EXPORT LLGLTargets NAMESPACE LLGL:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/LLGL")
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LLGLConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LLGL
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LLGLConfig.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/LLGL")

# Summary Information
message("~~~ Build Summary ~~~")

message("Target Platform: ${LLGL_TARGET_PLATFORM}")
message("Target Architecture: ${SUMMARY_TARGET_ARCH}")
message("Target Library: ${SUMMARY_LIBRARY_TYPE}")

if(LLGL_BUILD_RENDERER_NULL)
    message("Build Renderer: Null")
endif()

if(LLGL_BUILD_RENDERER_OPENGL AND OpenGL_FOUND)
    message("Build Renderer: OpenGL")
endif()

if(LLGL_BUILD_RENDERER_OPENGLES3 AND OPENGLES_FOUND)
    message("Build Renderer: OpenGLES 3")
endif()

if(NOT APPLE AND LLGL_BUILD_RENDERER_VULKAN AND VULKAN_FOUND)
    message("Build Renderer: Vulkan")
endif()

if(APPLE AND LLGL_BUILD_RENDERER_METAL)
    message("Build Renderer: Metal")
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D11)
    if(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.3")
        message("Build Renderer: Direct3D 11.3")
    elseif(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.2")
        message("Build Renderer: Direct3D 11.2")
    elseif(${LLGL_D3D11_ENABLE_FEATURELEVEL} STREQUAL "Direct3D 11.1")
        message("Build Renderer: Direct3D 11.1")
    else()
        message("Build Renderer: Direct3D 11.0")
    endif()
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D12)
    message("Build Renderer: Direct3D 12.0")
endif()

if(LLGL_BUILD_WRAPPER_C99)
    message("Build Wrapper: C99")
endif()

if(WIN32 AND LLGL_BUILD_WRAPPER_CSHARP)
    message("Build Wrapper: C#")
endif()

if(LLGL_ENABLE_SPIRV_REFLECT)
    message("Including Submodule: SPIRV-Headers")
endif()


