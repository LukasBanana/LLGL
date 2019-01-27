
# === CMake lists for the LLGL - (09/07/2015) ===

cmake_minimum_required(VERSION 2.8)
project(LLGL)


# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(EXAMPLE_PROJECTS_DIR "${PROJECT_SOURCE_DIR}/examples/Cpp")


# === Preprocessor definitions ===

if(DEFINED IOS_PLATFORM)
    set(MOBILE_PLATFORM ON)
else()
    set(MOBILE_PLATFORM OFF)
endif()

macro(ADD_DEFINE IDENT)
    add_definitions(-D${IDENT})
endmacro()

macro(ADD_DEBUG_DEFINE IDENT)
    if(MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D${IDENT}")
    else(MSVC)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D${IDENT}")
    endif(MSVC)
endmacro()

macro(ENABLE_CXX11 PROJECT_NAME)
    if(MOBILE_PLATFORM)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        target_compile_features(${PROJECT_NAME} PRIVATE cxx_range_for)
    endif()
endmacro()

macro(ADD_TEST_PROJECT TEST_NAME TEST_FILES LIB_FILES)
    if(APPLE)
        add_executable(${TEST_NAME} MACOSX_BUNDLE ${TEST_FILES})
    else()
        add_executable(${TEST_NAME} ${TEST_FILES})
    endif()
    target_link_libraries(${TEST_NAME} ${LIB_FILES})
    set_target_properties(${TEST_NAME} PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
    ENABLE_CXX11(${TEST_NAME})
endmacro()

macro(ADD_FRAMEWORK PROJECT_NAME FRAMEWORK_NAME)
    find_library(
        "FRAMEWORK_${FRAMEWORK_NAME}"
        NAMES ${FRAMEWORK_NAME}
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH
    )
    if(${FRAMEWORK_${FRAMEWORK_NAME}} STREQUAL FRAMEWORK_${FRAMEWORK_NAME}-NOTFOUND)
        MESSAGE(ERROR ": Framework ${FRAMEWORK_NAME} not found")
    else()
        TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${FRAMEWORK_${FRAMEWORK_NAME}})
    endif()
endmacro(ADD_FRAMEWORK)

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

if(MOBILE_PLATFORM)
    ADD_DEFINE(LLGL_MOBILE_PLATFORM)
endif()


# === Initialize summary variables ===

set(SUMMARY_TARGET_PLATFORM "Unknown")
set(SUMMARY_LIBRARY_TYPE "Unknown")
set(SUMMARY_TARGET_ARCH "Unknown")


# === Options ===

if(WIN32)
    set(LLGL_D3D11_ENABLE_FEATURELEVEL "Direct3D 11.0" CACHE STRING "Enable maximum Direct3D 11 feature level (requires <d3d11.h>, <d3d11_1.h>, <d3d11_2.h>, or <d3d11_3.h> header files")
    set_property(CACHE LLGL_D3D11_ENABLE_FEATURELEVEL PROPERTY STRINGS "Direct3D 11.0" "Direct3D 11.1" "Direct3D 11.2" "Direct3D 11.3")
elseif(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif()

option(LLGL_ENABLE_CHECKED_CAST "Enable dynamic checked cast (only in Debug mode)" ON)
option(LLGL_ENABLE_DEBUG_LAYER "Enable renderer debug layer (for both Debug and Release mode)" ON)
option(LLGL_ENABLE_UTILITY "Enable utility functions (LLGL/Utility.h)" ON)
option(LLGL_ENABLE_SPIRV_REFLECT "Enable shader reflection of SPIR-V modules (requires the SPIRV submodule)" OFF)
option(LLGL_ENABLE_JIT_COMPILER "Enables Just-in-Time (JIT) compilation for emulated deferred command buffers (experimental)" OFF)

option(LLGL_GL_ENABLE_EXT_PLACEHOLDERS "Enable OpenGL extension placeholders" ON)
option(LLGL_GL_ENABLE_VENDOR_EXT "Enable vendor specific OpenGL extensions (e.g. GL_NV_..., GL_AMD_... etc.)" ON)
option(LLGL_GL_ENABLE_DSA_EXT "Enable OpenGL direct state access (DSA) extension if available" ON)
option(LLGL_GL_INCLUDE_EXTERNAL "Include additional OpenGL header files from 'external' folder" ON)

option(LLGL_BUILD_STATIC_LIB "Build LLGL as static lib (Only allows a single render system!)" OFF)
option(LLGL_BUILD_TESTS "Include test projects" OFF)
option(LLGL_BUILD_EXAMPLES "Include example projects" OFF)

if(MOBILE_PLATFORM)
    option(LLGL_BUILD_RENDERER_OPENGLES3 "Include OpenGL ES 3 renderer project" ON)
else()
    option(LLGL_BUILD_RENDERER_OPENGL "Include OpenGL renderer project" ON)
endif()

if(APPLE)
    option(LLGL_BUILD_RENDERER_METAL "Include Metal renderer project (experimental)" OFF)
endif()

if(NOT APPLE)
    option(LLGL_BUILD_RENDERER_VULKAN "Include Vulkan renderer project (experimental)" OFF)
    option(LLGL_VK_ENABLE_EXT "Enable extensions for Vulkan 1.1+" OFF)
endif()

if(WIN32)
    option(LLGL_BUILD_RENDERER_DIRECT3D11 "Include Direct3D11 renderer project" ON)
    option(LLGL_BUILD_RENDERER_DIRECT3D12 "Include Direct3D12 renderer project (experimental)" OFF)
    option(LLGL_BUILD_WRAPPER_CSHARP "Include wrapper for C#" OFF)
endif()

if(LLGL_ENABLE_CHECKED_CAST)
    ADD_DEBUG_DEFINE(LLGL_ENABLE_CHECKED_CAST)
endif()

if(LLGL_ENABLE_DEBUG_LAYER)
    ADD_DEFINE(LLGL_ENABLE_DEBUG_LAYER)
endif()

if(LLGL_ENABLE_UTILITY)
    ADD_DEFINE(LLGL_ENABLE_UTILITY)
endif()

if(LLGL_ENABLE_SPIRV_REFLECT)
    ADD_DEFINE(LLGL_ENABLE_SPIRV_REFLECT)
endif()

if(LLGL_ENABLE_JIT_COMPILER)
    ADD_DEFINE(LLGL_ENABLE_JIT_COMPILER)
endif()

if(LLGL_GL_ENABLE_EXT_PLACEHOLDERS)
    ADD_DEFINE(LLGL_GL_ENABLE_EXT_PLACEHOLDERS)
endif()

if(LLGL_GL_ENABLE_VENDOR_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_VENDOR_EXT)
endif()

if(LLGL_GL_ENABLE_DSA_EXT)
    ADD_DEFINE(LLGL_GL_ENABLE_DSA_EXT)
endif()

if(LLGL_BUILD_STATIC_LIB)
    ADD_DEFINE(LLGL_BUILD_STATIC_LIB)
endif()

if(LLGL_VK_ENABLE_EXT)
    ADD_DEFINE(LLGL_VK_ENABLE_EXT)
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

if(MOBILE_PLATFORM)
	set(ARCH_ARM64 ON)
	set(SUMMARY_TARGET_ARCH "ARM64")
elseif(APPLE)
	set(ARCH_AMD64 ON)
	set(SUMMARY_TARGET_ARCH "AMD64 (x86-x64)")
else()
	set(ARCH_IA32 ON)
	set(SUMMARY_TARGET_ARCH "IA-32 (x86)")
endif()


# === Global files ===

# Common files
file(GLOB FilesInclude                      ${PROJECT_INCLUDE_DIR}/LLGL/*.*)
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
endif()

if(WIN32)
    file(GLOB FilesPlatform                 ${PROJECT_SOURCE_DIR}/sources/Platform/Win32/*.*)
elseif(APPLE)
    if(MOBILE_PLATFORM)
        file(GLOB FilesPlatform             ${PROJECT_SOURCE_DIR}/sources/Platform/IOS/*.*)
    else()
        file(GLOB FilesPlatform             ${PROJECT_SOURCE_DIR}/sources/Platform/MacOS/*.*)
    endif()
elseif(UNIX)
    file(GLOB FilesPlatform                 ${PROJECT_SOURCE_DIR}/sources/Platform/Linux/*.*)
endif()

# OpenGL common renderer files
file(GLOB FilesRendererGLCommon             ${PROJECT_SOURCE_DIR}/sources/Renderer/GLCommon/*.*)
file(GLOB FilesRendererGLCommonTexture      ${PROJECT_SOURCE_DIR}/sources/Renderer/GLCommon/Texture/*.*)

# OpenGL renderer files
file(GLOB FilesRendererGL                   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/*.*)
file(GLOB FilesRendererGLBuffer             ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Buffer/*.*)
file(GLOB FilesRendererGLCommand            ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Command/*.*)
file(GLOB FilesRendererGLExt                ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Ext/*.*)
file(GLOB FilesRendererGLPlatformBase       ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/*.*)
file(GLOB FilesRendererGLRenderState        ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/RenderState/*.*)
file(GLOB FilesRendererGLShader             ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/*.*)
file(GLOB FilesRendererGLTexture            ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Texture/*.*)

if(WIN32)
    file(GLOB FilesRendererGLPlatform       ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Win32/*.*)
    file(GLOB FilesIncludePlatform          ${PROJECT_INCLUDE_DIR}/LLGL/Platform/Win32/*.*)
    set(SUMMARY_TARGET_PLATFORM "Win32")
elseif(APPLE)
    if(MOBILE_PLATFORM)
        file(GLOB FilesRendererGLPlatform   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGLES/Platform/IOS/*.*)
        file(GLOB FilesIncludePlatform      ${PROJECT_INCLUDE_DIR}/LLGL/Platform/IOS/*.*)
        set(SUMMARY_TARGET_PLATFORM "iOS")
    else()
        file(GLOB FilesRendererGLPlatform   ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/MacOS/*.*)
        file(GLOB FilesIncludePlatform      ${PROJECT_INCLUDE_DIR}/LLGL/Platform/MacOS/*.*)
        set(SUMMARY_TARGET_PLATFORM "macOS")
    endif()
elseif(UNIX)
    file(GLOB FilesRendererGLPlatform       ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Linux/*.*)
    file(GLOB FilesIncludePlatform          ${PROJECT_INCLUDE_DIR}/LLGL/Platform/Linux/*.*)
    set(SUMMARY_TARGET_PLATFORM "Linux")
endif()

# OpenGLES3 renderer files
file(GLOB FilesRendererGLES3                ${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGLES3/*.*)

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
file(GLOB FilesRendererMTLRenderState       ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/RenderState/*.*)
file(GLOB FilesRendererMTLShader            ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Shader/*.*)
file(GLOB FilesRendererMTLTexture           ${PROJECT_SOURCE_DIR}/sources/Renderer/Metal/Texture/*.*)

# Direct3D common renderer files
file(GLOB FilesRendererDXCommon             ${PROJECT_SOURCE_DIR}/sources/Renderer/DXCommon/*.*)

# Direct3D12 renderer files
file(GLOB FilesRendererD3D12                ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/*.*)
file(GLOB FilesRendererD3D12Buffer          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Buffer/*.*)
file(GLOB FilesRendererD3D12Command         ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Command/*.*)
file(GLOB FilesRendererD3D12RenderState     ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/RenderState/*.*)
file(GLOB FilesRendererD3D12Shader          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Shader/*.*)
file(GLOB FilesRendererD3D12Texture         ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Texture/*.*)

# Direct3D11 renderer files
file(GLOB FilesRendererD3D11                ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/*.*)
file(GLOB FilesRendererD3D11Buffer          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Buffer/*.*)
file(GLOB FilesRendererD3D11RenderState     ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/RenderState/*.*)
file(GLOB FilesRendererD3D11Shader          ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Shader/*.*)
file(GLOB FilesRendererD3D11Texture         ${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Texture/*.*)

# Test project files
set(TestProjectsPath ${PROJECT_SOURCE_DIR}/tests)

set(FilesTest_Window ${TestProjectsPath}/Test_Window.cpp)
set(FilesTest_OpenGL ${TestProjectsPath}/Test_OpenGL.cpp)
set(FilesTest_D3D12 ${TestProjectsPath}/Test_D3D12.cpp)
set(FilesTest_Vulkan ${TestProjectsPath}/Test_Vulkan.cpp)
set(FilesTest_Metal ${TestProjectsPath}/Test_Metal.cpp)
set(FilesTest_Compute ${TestProjectsPath}/Test_Compute.cpp)
set(FilesTest_Performance ${TestProjectsPath}/Test_Performance.cpp)
set(FilesTest_Display ${TestProjectsPath}/Test_Display.cpp)
set(FilesTest_Image ${TestProjectsPath}/Test_Image.cpp)
set(FilesTest_BlendStates ${TestProjectsPath}/Test_BlendStates.cpp)
set(FilesTest_JIT ${TestProjectsPath}/Test_JIT.cpp)

# Example project files
file(GLOB FilesExampleBase ${EXAMPLE_PROJECTS_DIR}/ExampleBase/*.*)

set(FilesExample_HelloTriangle ${EXAMPLE_PROJECTS_DIR}/HelloTriangle/Example.cpp)
set(FilesExample_Tessellation ${EXAMPLE_PROJECTS_DIR}/Tessellation/Example.cpp)
set(FilesExample_Texturing ${EXAMPLE_PROJECTS_DIR}/Texturing/Example.cpp)
set(FilesExample_Queries ${EXAMPLE_PROJECTS_DIR}/Queries/Example.cpp)
set(FilesExample_RenderTarget ${EXAMPLE_PROJECTS_DIR}/RenderTarget/Example.cpp)
set(FilesExample_MultiContext ${EXAMPLE_PROJECTS_DIR}/MultiContext/Example.cpp)
set(FilesExample_BufferArray ${EXAMPLE_PROJECTS_DIR}/BufferArray/Example.cpp)
set(FilesExample_ComputeShader ${EXAMPLE_PROJECTS_DIR}/ComputeShader/Example.cpp)
set(FilesExample_StreamOutput ${EXAMPLE_PROJECTS_DIR}/StreamOutput/Example.cpp)
set(FilesExample_Instancing ${EXAMPLE_PROJECTS_DIR}/Instancing/Example.cpp)
set(FilesExample_PostProcessing ${EXAMPLE_PROJECTS_DIR}/PostProcessing/Example.cpp)
set(FilesExample_MultiRenderer ${EXAMPLE_PROJECTS_DIR}/MultiRenderer/Example.cpp)
set(FilesExample_ShadowMapping ${EXAMPLE_PROJECTS_DIR}/ShadowMapping/Example.cpp)
set(FilesExample_UnorderedAccess ${EXAMPLE_PROJECTS_DIR}/UnorderedAccess/Example.cpp)
set(FilesExample_MultiThreading ${EXAMPLE_PROJECTS_DIR}/MultiThreading/Example.cpp)
set(FilesExample_PBR ${EXAMPLE_PROJECTS_DIR}/PBR/Example.cpp)


# === Source group folders ===

source_group("Include" FILES ${FilesInclude})
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
endif()

source_group("Sources\\GLCommon" FILES ${FilesRendererGLCommon})
source_group("Sources\\GLCommon\\Texture" FILES ${FilesRendererGLCommonTexture})

source_group("Sources\\OpenGL" FILES ${FilesRendererGL})
source_group("Sources\\OpenGL\\Buffer" FILES ${FilesRendererGLBuffer})
source_group("Sources\\OpenGL\\Command" FILES ${FilesRendererGLCommand})
source_group("Sources\\OpenGL\\Ext" FILES ${FilesRendererGLExt})
source_group("Sources\\OpenGL\\Platform" FILES ${FilesRendererGLPlatformBase} ${FilesRendererGLPlatform})
source_group("Sources\\OpenGL\\RenderState" FILES ${FilesRendererGLRenderState})
source_group("Sources\\OpenGL\\Shader" FILES ${FilesRendererGLShader})
source_group("Sources\\OpenGL\\Texture" FILES ${FilesRendererGLTexture})

source_group("Sources\\OpenGLES3" FILES ${FilesRendererGLES3})

source_group("Sources\\SPIRV" FILES ${FilesRendererSPIRV})

source_group("Sources\\Vulkan" FILES ${FilesRendererVK})
source_group("Sources\\Vulkan\\Buffer" FILES ${FilesRendererVKBuffer})
source_group("Sources\\Vulkan\\Ext" FILES ${FilesRendererVKExt})
source_group("Sources\\Vulkan\\Memory" FILES ${FilesRendererVKMemory})
source_group("Sources\\Vulkan\\RenderState" FILES ${FilesRendererVKRenderState})
source_group("Sources\\Vulkan\\Shader" FILES ${FilesRendererVKShader})
source_group("Sources\\Vulkan\\Texture" FILES ${FilesRendererVKTexture})

source_group("Sources\\Metal" FILES ${FilesRendererMTL})
source_group("Sources\\Metal\\Buffer" FILES ${FilesRendererMTLBuffer})
source_group("Sources\\Metal\\RenderState" FILES ${FilesRendererMTLRenderState})
source_group("Sources\\Metal\\Shader" FILES ${FilesRendererMTLShader})
source_group("Sources\\Metal\\Texture" FILES ${FilesRendererMTLTexture})

source_group("Sources\\DXCommon" FILES ${FilesRendererDXCommon})

source_group("Sources\\Direct3D11" FILES ${FilesRendererD3D11})
source_group("Sources\\Direct3D11\\Buffer" FILES ${FilesRendererD3D11Buffer})
source_group("Sources\\Direct3D11\\RenderState" FILES ${FilesRendererD3D11RenderState})
source_group("Sources\\Direct3D11\\Shader" FILES ${FilesRendererD3D11Shader})
source_group("Sources\\Direct3D11\\Texture" FILES ${FilesRendererD3D11Texture})

source_group("Sources\\Direct3D12" FILES ${FilesRendererD3D12})
source_group("Sources\\Direct3D12\\Buffer" FILES ${FilesRendererD3D12Buffer})
source_group("Sources\\Direct3D12\\Command" FILES ${FilesRendererD3D12Command})
source_group("Sources\\Direct3D12\\RenderState" FILES ${FilesRendererD3D12RenderState})
source_group("Sources\\Direct3D12\\Shader" FILES ${FilesRendererD3D12Shader})
source_group("Sources\\Direct3D12\\Texture" FILES ${FilesRendererD3D12Texture})

source_group("Sources" FILES ${FilesExampleBase})


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")

include(cmake/FindGaussianLib.cmake)

if(LLGL_BUILD_EXAMPLES)
    include_directories("${PROJECT_SOURCE_DIR}/examples/Cpp/ExampleBase")
endif()


# === Projects ===

set(
    FilesLLGL
    ${FilesInclude}
    ${FilesIncludePlatformBase}
    ${FilesIncludePlatform}
    ${FilesCore}
    ${FilesPlatformBase}
    ${FilesPlatform}
    ${FilesRenderer}
)

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
    set(FilesLLGL ${FilesLLGL} ${FilesRendererDbg})
endif()

set(
    FilesGL
    ${FilesRendererGL}
    ${FilesRendererGLBuffer}
    ${FilesRendererGLCommand}
    ${FilesRendererGLCommon}
    ${FilesRendererGLCommonTexture}
    ${FilesRendererGLExt}
    ${FilesRendererGLPlatform}
    ${FilesRendererGLPlatformBase}
    ${FilesRendererGLRenderState}
    ${FilesRendererGLShader}
    ${FilesRendererGLTexture}
)

set(
    FilesGLES3
    ${FilesRendererGLES3}
    ${FilesRendererGLCommon}
    ${FilesRendererGLCommonTexture}
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
    ${FilesRendererMTLRenderState}
    ${FilesRendererMTLShader}
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
    ${FilesRendererD3D12RenderState}
    ${FilesRendererD3D12Texture}
    ${FilesRendererDXCommon}
)

set(
    FilesD3D11
    ${FilesRendererD3D11}
    ${FilesRendererD3D11Buffer}
    ${FilesRendererD3D11Shader}
    ${FilesRendererD3D11RenderState}
    ${FilesRendererD3D11Texture}
    ${FilesRendererDXCommon}
)

# Base project
if(LLGL_BUILD_STATIC_LIB)
    set(SUMMARY_LIBRARY_TYPE "Static")
    add_library(LLGL STATIC ${FilesLLGL})
else()
    set(SUMMARY_LIBRARY_TYPE "Shared")
    add_library(LLGL SHARED ${FilesLLGL})
endif()

if(APPLE)
    if(MOBILE_PLATFORM)
        ADD_FRAMEWORK(LLGL UIKit)
    else()
        find_library(COCOA_LIBRARY Cocoa)
        target_link_libraries(LLGL ${COCOA_LIBRARY})
    endif()
elseif(UNIX)
    target_link_libraries(LLGL X11 pthread Xxf86vm Xrandr)
endif()

if(MINGW)
    set_target_properties(LLGL PROPERTIES PREFIX "")
endif()

set_target_properties(LLGL PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
ENABLE_CXX11(LLGL)

set(TEST_PROJECT_LIBS LLGL)

if(LLGL_BUILD_RENDERER_OPENGLES3)
    # OpenGLES Renderer
    include(cmake/FindOpenGLES.cmake)
    if(OPENGLES_FOUND)
        include_directories(${OPENGLES_INCLUDE_DIR})
        
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_OpenGLES3 STATIC ${FilesGLES3})
            set(TEST_PROJECT_LIBS LLGL_OpenGLES3)
        else()
            add_library(LLGL_OpenGLES3 SHARED ${FilesGLES3})
        endif()
        
        set_target_properties(LLGL_OpenGLES3 PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(LLGL_OpenGLES3 LLGL ${OPENGLES_LIBRARIES})
        
        if(MOBILE_PLATFORM)
            ADD_FRAMEWORK(LLGL_OpenGLES3 UIKit)
            ADD_FRAMEWORK(LLGL_OpenGLES3 QuartzCore)
            ADD_FRAMEWORK(LLGL_OpenGLES3 OpenGLES)
        else()
            ENABLE_CXX11(LLGL_OpenGLES3)
        endif()
    else()
        message("Missing OpenGL ES -> LLGL_OpenGLES3 renderer will be excluded from project")
    endif()
endif()

if(LLGL_BUILD_RENDERER_OPENGL)
    # OpenGL Renderer
    find_package(OpenGL)
    if(OpenGL_FOUND)
        include_directories(${OPENGL_INCLUDE_DIR})
        
        if(LLGL_GL_INCLUDE_EXTERNAL)
            include_directories("${PROJECT_SOURCE_DIR}/external/OpenGL/include")
        endif()
        
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_OpenGL STATIC ${FilesGL})
            set(TEST_PROJECT_LIBS LLGL_OpenGL)
        else()
            add_library(LLGL_OpenGL SHARED ${FilesGL})
        endif()
        
        if(MINGW)
            set_target_properties(LLGL_OpenGL PROPERTIES PREFIX "")
        endif()
        
        set_target_properties(LLGL_OpenGL PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(LLGL_OpenGL LLGL ${OPENGL_LIBRARIES})
        ENABLE_CXX11(LLGL_OpenGL)
    else()
        message("Missing OpenGL -> LLGL_OpenGL renderer will be excluded from project")
    endif()
endif()

if(LLGL_BUILD_RENDERER_VULKAN)
    # Vulkan Renderer
    include(cmake/FindVulkan.cmake)
    if(VULKAN_FOUND)
        include_directories(${VULKAN_INCLUDE_DIR})
        add_library(LLGL_Vulkan SHARED ${FilesVK})
        set_target_properties(LLGL_Vulkan PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(LLGL_Vulkan LLGL ${VULKAN_LIBRARY})
        ENABLE_CXX11(LLGL_Vulkan)
    else()
        message("Missing Vulkan -> LLGL_Vulkan renderer will be excluded from project")
    endif()
    
    if(MINGW)
        set_target_properties(LLGL_Vulkan PROPERTIES PREFIX "")
    endif()
    
    if(LLGL_ENABLE_SPIRV_REFLECT)
        # SPIRV Submodule
        include_directories("${PROJECT_SOURCE_DIR}/external/SPIRV/include")
    endif()
endif()

if(APPLE AND LLGL_BUILD_RENDERER_METAL)
    # Metal Renderer
    include(cmake/FindMetal.cmake)
    include(cmake/FindMetalKit.cmake)
    if(METAL_FOUND AND METALKIT_FOUND)
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_Metal STATIC ${FilesMTL})
            set(TEST_PROJECT_LIBS LLGL_Metal)
        else()
            add_library(LLGL_Metal SHARED ${FilesMTL})
        endif()
        
        set_target_properties(LLGL_Metal PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(LLGL_Metal LLGL ${METAL_LIBRARY} ${METALKIT_LIBRARY})
        ENABLE_CXX11(LLGL_Metal)
    else()
        message("Missing Metal/MetalKit -> LLGL_Metal renderer will be excluded from project")
    endif()
endif()

if(WIN32)
    if(LLGL_BUILD_RENDERER_DIRECT3D11)
        # Direct3D 11 Renderer
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_Direct3D11 STATIC ${FilesD3D11})
            set(TEST_PROJECT_LIBS LLGL_Direct3D11)
        else()
            add_library(LLGL_Direct3D11 SHARED ${FilesD3D11})
        endif()
        
        if(MINGW)
            set_target_properties(LLGL_Direct3D11 PROPERTIES PREFIX "")
        endif()
        
        set_target_properties(LLGL_Direct3D11 PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(LLGL_Direct3D11 LLGL d3d11 dxgi D3DCompiler)
        ENABLE_CXX11(LLGL_Direct3D11)
    endif()
    
    if(LLGL_BUILD_RENDERER_DIRECT3D12)
        # Direct3D 12 Renderer
        if(LLGL_BUILD_STATIC_LIB)
            add_library(LLGL_Direct3D12 STATIC ${FilesD3D12})
            set(TEST_PROJECT_LIBS LLGL_Direct3D12)
        else()
            add_library(LLGL_Direct3D12 SHARED ${FilesD3D12})
        endif()
        
        if(MINGW)
            set_target_properties(LLGL_Direct3D12 PROPERTIES PREFIX "")
        endif()
        
        set_target_properties(LLGL_Direct3D12 PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(LLGL_Direct3D12 LLGL d3d12 dxgi D3DCompiler)
        target_compile_definitions(LLGL_Direct3D12 PUBLIC -DLLGL_DX_ENABLE_D3D12)
        ENABLE_CXX11(LLGL_Direct3D12)
    endif()
endif()

if(GaussLib_INCLUDE_DIR)
    # Test Projects
    if(LLGL_BUILD_TESTS)
        ADD_TEST_PROJECT(Test_OpenGL "${FilesTest_OpenGL}" "${TEST_PROJECT_LIBS}")
        if(WIN32)
            ADD_TEST_PROJECT(Test_D3D12 "${FilesTest_D3D12}" "${TEST_PROJECT_LIBS}")
        endif()
        if(LLGL_BUILD_RENDERER_VULKAN AND VULKAN_FOUND)
            ADD_TEST_PROJECT(Test_Vulkan "${FilesTest_Vulkan}" "${TEST_PROJECT_LIBS}")
        endif()
        if(APPLE)
            ADD_TEST_PROJECT(Test_Metal "${FilesTest_Metal}" "${TEST_PROJECT_LIBS}")
        endif()
        ADD_TEST_PROJECT(Test_Compute "${FilesTest_Compute}" "${TEST_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Test_Performance "${FilesTest_Performance}" "${TEST_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Test_Display "${FilesTest_Display}" "${TEST_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Test_Image "${FilesTest_Image}" "${TEST_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Test_BlendStates "${FilesTest_BlendStates}" "${TEST_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Test_Window "${FilesTest_Window}" "${TEST_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Test_JIT "${FilesTest_JIT}" "${TEST_PROJECT_LIBS}")
    endif()

    # Example Projects
    if(LLGL_BUILD_EXAMPLES)
        add_library(ExampleBase STATIC ${FilesExampleBase})
        set_target_properties(ExampleBase PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
        target_link_libraries(ExampleBase "${TEST_PROJECT_LIBS}")
        ENABLE_CXX11(ExampleBase)
        
        set(EXAMPLE_PROJECT_LIBS ${TEST_PROJECT_LIBS} ExampleBase)
        
        ADD_TEST_PROJECT(Example_HelloTriangle "${FilesExample_HelloTriangle}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_Tessellation "${FilesExample_Tessellation}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_Texturing "${FilesExample_Texturing}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_Queries "${FilesExample_Queries}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_RenderTarget "${FilesExample_RenderTarget}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_MultiContext "${FilesExample_MultiContext}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_BufferArray "${FilesExample_BufferArray}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_ComputeShader "${FilesExample_ComputeShader}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_StreamOutput "${FilesExample_StreamOutput}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_Instancing "${FilesExample_Instancing}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_PostProcessing "${FilesExample_PostProcessing}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_MultiRenderer "${FilesExample_MultiRenderer}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_ShadowMapping "${FilesExample_ShadowMapping}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_UnorderedAccess "${FilesExample_UnorderedAccess}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_MultiThreading "${FilesExample_MultiThreading}" "${EXAMPLE_PROJECT_LIBS}")
        ADD_TEST_PROJECT(Example_PBR "${FilesExample_PBR}" "${EXAMPLE_PROJECT_LIBS}")
    endif()
else()
    if(LLGL_BUILD_TESTS)
        message(SEND_ERROR "LLGL_BUILD_TESTS is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
    if(LLGL_BUILD_EXAMPLES)
        message(SEND_ERROR "LLGL_BUILD_EXAMPLES is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
endif()

# Wrapper: C#
if(WIN32 AND LLGL_BUILD_WRAPPER_CSHARP)
    add_subdirectory(Wrapper/CSharp)
endif()

# Summary Information
message("~~~ Build Summary ~~~")

message("Target Platform: ${SUMMARY_TARGET_PLATFORM}")
message("Target Architecture: ${SUMMARY_TARGET_ARCH}")
message("Target Library: ${SUMMARY_LIBRARY_TYPE}")

set(RENDERER_COUNT 0)

if(LLGL_BUILD_RENDERER_OPENGL AND OpenGL_FOUND)
    message("Build Renderer: OpenGL")
    math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
endif()

if(LLGL_BUILD_RENDERER_OPENGLES3 AND OPENGLES_FOUND)
    message("Build Renderer: OpenGL ES 3")
    math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
endif()

if(LLGL_BUILD_RENDERER_VULKAN AND VULKAN_FOUND)
    math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
    message("Build Renderer: Vulkan")
endif()

if(LLGL_BUILD_RENDERER_METAL)
    math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
    message("Build Renderer: Metal")
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D11)
    math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
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
    math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
    message("Build Renderer: Direct3D 12.0")
endif()

if(WIN32 AND LLGL_BUILD_WRAPPER_CSHARP)
    message("Build Wrapper: C#")
endif()

if(LLGL_BUILD_STATIC_LIB AND NOT(${RENDERER_COUNT} EQUAL 1))
    message(SEND_ERROR "Static library only supports one single render backend, but multiple are specified!")
endif()

if(LLGL_ENABLE_SPIRV_REFLECT)
    message("Including Submodule: SPIRV")
endif()


