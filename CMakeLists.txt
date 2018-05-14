
# === CMake lists for the LLGL - (09/07/2015) ===

cmake_minimum_required(VERSION 2.8)
project(LLGL)


# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")


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


# === Options ===

option(LLGL_ENABLE_CHECKED_CAST "Enable dynamic checked cast (only in Debug mode)" ON)
option(LLGL_ENABLE_DEBUG_LAYER "Enable renderer debug layer (for both Debug and Release mode)" ON)
option(LLGL_ENABLE_UTILITY "Enable utility functions (LLGL/Utility.h)" ON)
option(LLGL_ENABLE_BACKWARDS_COMPATIBILITY "Enable various functions for backwards compatibility (C++14 only)" OFF)
option(LLGL_ENABLE_SPIRV_REFLECT "Enable shader reflection of SPIR-V modules (requires the SPIRV submodule)" OFF)

option(LLGL_GL_ENABLE_EXT_PLACEHOLDERS "Enable OpenGL extension placeholders" ON)
option(LLGL_GL_ENABLE_VENDOR_EXT "Enable vendor specific OpenGL extensions (e.g. GL_NV_..., GL_AMD_... etc.)" ON)
option(LLGL_GL_ENABLE_DSA_EXT "Enable OpenGL direct state access (DSA) extension if available" ON)
option(LLGL_GL_INCLUDE_EXTERNAL "Include additional OpenGL header files from 'external' folder" ON)

option(LLGL_BUILD_STATIC_LIB "Build LLGL as static lib (Only allows a single render system!)" OFF)
option(LLGL_BUILD_TESTS "Include test projects" OFF)
option(LLGL_BUILD_TUTORIALS "Include tutorial projects" OFF)

if(MOBILE_PLATFORM)
	option(LLGL_BUILD_RENDERER_OPENGLES3 "Include OpenGL ES 3 renderer project" ON)
else()
	option(LLGL_BUILD_RENDERER_OPENGL "Include OpenGL renderer project" ON)
endif()

if(NOT APPLE)
	option(LLGL_BUILD_RENDERER_VULKAN "Include Vulkan renderer project (experimental)" OFF)
endif()

if(WIN32)
	option(LLGL_BUILD_RENDERER_DIRECT3D11 "Include Direct3D11 renderer project" ON)
	option(LLGL_BUILD_RENDERER_DIRECT3D12 "Include Direct3D12 renderer project (experimental)" OFF)
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

if(LLGL_ENABLE_BACKWARDS_COMPATIBILITY)
    ADD_DEFINE(LLGL_ENABLE_BACKWARDS_COMPATIBILITY)
endif()

if(LLGL_ENABLE_SPIRV_REFLECT)
    ADD_DEFINE(LLGL_ENABLE_SPIRV_REFLECT)
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


# === Global files ===

# Common files
file(GLOB FilesInclude						${PROJECT_INCLUDE_DIR}/LLGL/*.*)
file(GLOB FilesIncludePlatformBase			${PROJECT_INCLUDE_DIR}/LLGL/Platform/*.*)
file(GLOB FilesCore							${PROJECT_SOURCE_DIR}/sources/Core/*.*)
file(GLOB FilesPlatformBase					${PROJECT_SOURCE_DIR}/sources/Platform/*.*)
file(GLOB FilesRenderer						${PROJECT_SOURCE_DIR}/sources/Renderer/*.*)

if(LLGL_ENABLE_DEBUG_LAYER)
	file(GLOB FilesRendererDbg				${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/*.*)
endif()

if(WIN32)
	file(GLOB FilesPlatform					${PROJECT_SOURCE_DIR}/sources/Platform/Win32/*.*)
elseif(APPLE)
	set(CMAKE_MACOSX_RPATH 1)
	if(MOBILE_PLATFORM)
		file(GLOB FilesPlatform				${PROJECT_SOURCE_DIR}/sources/Platform/IOS/*.*)
	else()
		file(GLOB FilesPlatform				${PROJECT_SOURCE_DIR}/sources/Platform/MacOS/*.*)
	endif()
elseif(UNIX)
	file(GLOB FilesPlatform					${PROJECT_SOURCE_DIR}/sources/Platform/Linux/*.*)
endif()

# OpenGL common renderer files
file(GLOB FilesRendererGLCommon				${PROJECT_SOURCE_DIR}/sources/Renderer/GLCommon/*.*)
file(GLOB FilesRendererGLCommonTexture		${PROJECT_SOURCE_DIR}/sources/Renderer/GLCommon/Texture/*.*)

# OpenGL renderer files
file(GLOB FilesRendererGL					${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/*.*)
file(GLOB FilesRendererGLBuffer				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Buffer/*.*)
file(GLOB FilesRendererGLExt				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Ext/*.*)
file(GLOB FilesRendererGLPlatformBase		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/*.*)
file(GLOB FilesRendererGLRenderState		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/RenderState/*.*)
file(GLOB FilesRendererGLShader				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/*.*)
file(GLOB FilesRendererGLTexture			${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Texture/*.*)

if(WIN32)
	file(GLOB FilesRendererGLPlatform		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Win32/*.*)
	file(GLOB FilesIncludePlatform			${PROJECT_INCLUDE_DIR}/LLGL/Platform/Win32/*.*)
	set(SUMMARY_TARGET_PLATFORM "Win32")
elseif(APPLE)
	if(MOBILE_PLATFORM)
		file(GLOB FilesRendererGLPlatform	${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGLES/Platform/IOS/*.*)
		file(GLOB FilesIncludePlatform		${PROJECT_INCLUDE_DIR}/LLGL/Platform/IOS/*.*)
		set(SUMMARY_TARGET_PLATFORM "iOS")
	else()
		file(GLOB FilesRendererGLPlatform	${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/MacOS/*.*)
		file(GLOB FilesIncludePlatform		${PROJECT_INCLUDE_DIR}/LLGL/Platform/MacOS/*.*)
		set(SUMMARY_TARGET_PLATFORM "macOS")
	endif()
elseif(UNIX)
	file(GLOB FilesRendererGLPlatform		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Linux/*.*)
	file(GLOB FilesIncludePlatform			${PROJECT_INCLUDE_DIR}/LLGL/Platform/Linux/*.*)
	set(SUMMARY_TARGET_PLATFORM "Linux")
endif()

# OpenGLES3 renderer files
file(GLOB FilesRendererGLES3				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGLES3/*.*)

# SPIR-V renderer files
file(GLOB FilesRendererSPIRV                ${PROJECT_SOURCE_DIR}/sources/Renderer/SPIRV/*.*)

# Vulkan renderer files
file(GLOB FilesRendererVK					${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/*.*)
file(GLOB FilesRendererVKBuffer				${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Buffer/*.*)
file(GLOB FilesRendererVKExt				${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Ext/*.*)
file(GLOB FilesRendererVKMemory				${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Memory/*.*)
file(GLOB FilesRendererVKRenderState		${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/RenderState/*.*)
file(GLOB FilesRendererVKShader				${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Shader/*.*)
file(GLOB FilesRendererVKTexture			${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/Texture/*.*)

# Direct3D common renderer files
file(GLOB FilesRendererDXCommon				${PROJECT_SOURCE_DIR}/sources/Renderer/DXCommon/*.*)

# Direct3D12 renderer files
file(GLOB FilesRendererD3D12				${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/*.*)
file(GLOB FilesRendererD3D12Buffer			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Buffer/*.*)
file(GLOB FilesRendererD3D12RenderState		${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/RenderState/*.*)
file(GLOB FilesRendererD3D12Shader			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Shader/*.*)
file(GLOB FilesRendererD3D12Texture			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Texture/*.*)

# Direct3D11 renderer files
file(GLOB FilesRendererD3D11				${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/*.*)
file(GLOB FilesRendererD3D11Buffer			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Buffer/*.*)
file(GLOB FilesRendererD3D11RenderState		${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/RenderState/*.*)
file(GLOB FilesRendererD3D11Shader			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Shader/*.*)
file(GLOB FilesRendererD3D11Texture			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Texture/*.*)

# Vulkan renderer files
#file(GLOB FilesRendererVulkan				${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/*.*)

# Test files
set(FilesTest1 ${PROJECT_SOURCE_DIR}/test/Test1_Window.cpp)
set(FilesTest2 ${PROJECT_SOURCE_DIR}/test/Test2_OpenGL.cpp)
set(FilesTest3 ${PROJECT_SOURCE_DIR}/test/Test3_Direct3D12.cpp)
set(FilesTest4 ${PROJECT_SOURCE_DIR}/test/Test4_Compute.cpp)
set(FilesTest5 ${PROJECT_SOURCE_DIR}/test/Test5_Vulkan.cpp)

# Tutorial files
set(FilesTutorial01 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial01_HelloTriangle/main.cpp)
set(FilesTutorial02 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial02_Tessellation/main.cpp)
set(FilesTutorial03 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial03_Texturing/main.cpp)
set(FilesTutorial04 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial04_Query/main.cpp)
set(FilesTutorial05 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial05_RenderTarget/main.cpp)
set(FilesTutorial06 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial06_MultiContext/main.cpp)
set(FilesTutorial07 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial07_Array/main.cpp)
set(FilesTutorial08 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial08_Compute/main.cpp)
set(FilesTutorial09 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial09_StreamOutput/main.cpp)
set(FilesTutorial10 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial10_Instancing/main.cpp)
set(FilesTutorial11 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial11_PostProcessing/main.cpp)
set(FilesTutorial12 ${PROJECT_SOURCE_DIR}/tutorial/Tutorial12_MultiRenderer/main.cpp)


# === Source group folders ===

source_group("Include" FILES ${FilesInclude})
source_group("Sources\\Core" FILES ${FilesCore})
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

source_group("Sources\\DXCommon" FILES ${FilesRendererDXCommon})

source_group("Sources\\Direct3D11" FILES ${FilesRendererD3D11})
source_group("Sources\\Direct3D11\\Buffer" FILES ${FilesRendererD3D11Buffer})
source_group("Sources\\Direct3D11\\RenderState" FILES ${FilesRendererD3D11RenderState})
source_group("Sources\\Direct3D11\\Shader" FILES ${FilesRendererD3D11Shader})
source_group("Sources\\Direct3D11\\Texture" FILES ${FilesRendererD3D11Texture})

source_group("Sources\\Direct3D12" FILES ${FilesRendererD3D12})
source_group("Sources\\Direct3D12\\Buffer" FILES ${FilesRendererD3D12Buffer})
source_group("Sources\\Direct3D12\\RenderState" FILES ${FilesRendererD3D12RenderState})
source_group("Sources\\Direct3D12\\Shader" FILES ${FilesRendererD3D12Shader})
source_group("Sources\\Direct3D12\\Texture" FILES ${FilesRendererD3D12Texture})


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")

include(cmake/FindGaussianLib.cmake)


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

if(LLGL_ENABLE_DEBUG_LAYER)
	set(FilesLLGL ${FilesLLGL} ${FilesRendererDbg})
endif()

set(
	FilesGL
	${FilesRendererGL}
	${FilesRendererGLBuffer}
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

if(LLGL_ENABLE_SPIRV_REFLECT)
    set(FilesVK ${FilesVK} ${FilesRendererSPIRV})
endif()

set(
	FilesD3D12
	${FilesRendererD3D12}
	${FilesRendererD3D12Buffer}
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
	target_link_libraries(LLGL X11 pthread)
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
    
    if(LLGL_ENABLE_SPIRV_REFLECT)
        # SPIRV Submodule
        include_directories("${PROJECT_SOURCE_DIR}/external/SPIRV/include")
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
		
		set_target_properties(LLGL_Direct3D12 PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
		target_link_libraries(LLGL_Direct3D12 LLGL d3d12 dxgi D3DCompiler)
		target_compile_definitions(LLGL_Direct3D12 PUBLIC -DLLGL_DX_ENABLE_D3D12)
		ENABLE_CXX11(LLGL_Direct3D12)
	endif()
endif()

if(GaussLib_INCLUDE_DIR)
    # Test Projects
    if(LLGL_BUILD_TESTS)
        ADD_TEST_PROJECT(Test1_Window ${FilesTest1} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Test2_OpenGL ${FilesTest2} ${TEST_PROJECT_LIBS})
        if(WIN32)
            ADD_TEST_PROJECT(Test3_Direct3D12 ${FilesTest3} ${TEST_PROJECT_LIBS})
        endif()
        ADD_TEST_PROJECT(Test4_Compute ${FilesTest4} ${TEST_PROJECT_LIBS})
        if(LLGL_BUILD_RENDERER_VULKAN AND VULKAN_FOUND)
            ADD_TEST_PROJECT(Test5_Vulkan ${FilesTest5} ${TEST_PROJECT_LIBS})
        endif()
    endif()

    # Tutorial Projects
    if(LLGL_BUILD_TUTORIALS)
        ADD_TEST_PROJECT(Tutorial01_HelloTriangle ${FilesTutorial01} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial02_Tessellation ${FilesTutorial02} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial03_Texturing ${FilesTutorial03} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial04_Query ${FilesTutorial04} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial05_RenderTarget ${FilesTutorial05} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial06_MultiContext ${FilesTutorial06} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial07_Array ${FilesTutorial07} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial08_Compute ${FilesTutorial08} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial09_StreamOutput ${FilesTutorial09} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial10_Instancing ${FilesTutorial10} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial11_PostProcessing ${FilesTutorial11} ${TEST_PROJECT_LIBS})
        ADD_TEST_PROJECT(Tutorial12_MultiRenderer ${FilesTutorial12} ${TEST_PROJECT_LIBS})
    endif()
else()
    if(LLGL_BUILD_TESTS)
        message(SEND_ERROR "LLGL_BUILD_TESTS is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
    if(LLGL_BUILD_TUTORIALS)
        message(SEND_ERROR "LLGL_BUILD_TUTORIALS is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
endif()

# Summary Information
message("~~~ Build Summary ~~~")

message("Target Platform: ${SUMMARY_TARGET_PLATFORM}")
message("Library Type: ${SUMMARY_LIBRARY_TYPE}")

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
    message("Build Renderer: Vulkan")
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D11)
	message("Build Renderer: Direct3D 11")
	math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D12)
	message("Build Renderer: Direct3D 12")
	math(EXPR RENDERER_COUNT "${RENDERER_COUNT}+1")
endif()

if(LLGL_BUILD_STATIC_LIB AND NOT(${RENDERER_COUNT} EQUAL 1))
	message(SEND_ERROR "Static library only supports one single render backend, but multiple are specified!")
endif()

if(LLGL_ENABLE_SPIRV_REFLECT)
    message("Including Submodule: SPIRV")
endif()


