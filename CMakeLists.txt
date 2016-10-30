
# === CMake lists for the LLGL - (09/07/2015) ===

cmake_minimum_required(VERSION 2.8)
project(LLGL)


# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")


# === Preprocessor definitions ===

macro(ADD_DEFINE IDENT)
	if(MSVC)
		add_definitions(/D${IDENT})
	else(MSVC)
		add_definitions(-D${IDENT})
	endif(MSVC)
endmacro()

macro(ADD_DEBUG_DEFINE IDENT)
	if(MSVC)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /D${IDENT}")
	else(MSVC)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D${IDENT}")
	endif(MSVC)
endmacro()

macro(ENABLE_CXX11 PROJECT_NAME)
	if(DEFINED IOS_PLATFORM)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else()
		target_compile_features(${PROJECT_NAME} PRIVATE cxx_range_for)
	endif()
endmacro()

macro(ADD_TEST_PROJECT TEST_NAME TEST_FILES)
	if(APPLE)
		add_executable(${TEST_NAME} MACOSX_BUNDLE ${TEST_FILES})
	else()
		add_executable(${TEST_NAME} ${TEST_FILES})
	endif()

	target_link_libraries(${TEST_NAME} LLGL)
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


# === Options ===

option(LLGL_ENABLE_CHECKED_CAST "Enable Dynamic Checked Cast (only in Debug mode)" ON)
option(LLGL_ENABLE_DEBUG_LAYER "Enable Renderer Debug Layer (for both Debug and Release mode)" ON)
option(LLGL_ENABLE_UTILITY "Enable Utility Functions (LLGL/Utility.h)" ON)

option(LLGL_GL_ENABLE_VENDOR_EXT "Enable Vendor Specific OpenGL Extensions (e.g. GL_NV_..., GL_AMD_... etc.)" ON)
option(LLGL_GL_ENABLE_EXT_PLACEHOLDERS "Enable OpenGL Extension Placeholders" ON)

option(LLGL_BUILD_TESTS "Include Test Projects" ON)
option(LLGL_BUILD_TUTORIALS "Include Tutorial Projects" ON)

if(DEFINED IOS_PLATFORM)
	option(LLGL_BUILD_RENDERER_OPENGLES2 "Include OpenGL ES 2 Renderer Project" ON)
else()
	option(LLGL_BUILD_RENDERER_OPENGL "Include OpenGL Renderer Project" ON)
endif()

if(WIN32)
	option(LLGL_BUILD_RENDERER_DIRECT3D11 "Include Direct3D11 Renderer Project" ON)
	option(LLGL_BUILD_RENDERER_DIRECT3D12 "Include Direct3D12 Renderer Project (Experimental)" OFF)
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

if(LLGL_GL_ENABLE_VENDOR_EXT)
	ADD_DEFINE(LLGL_GL_ENABLE_VENDOR_EXT)
endif()

if(LLGL_GL_ENABLE_EXT_PLACEHOLDERS)
	ADD_DEFINE(LLGL_GL_ENABLE_EXT_PLACEHOLDERS)
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

file(GLOB FilesRendererDXCommon				${PROJECT_SOURCE_DIR}/sources/Renderer/DXCommon/*.*)

if(WIN32)
	file(GLOB FilesPlatform					${PROJECT_SOURCE_DIR}/sources/Platform/Win32/*.*)
elseif(APPLE)
	set(CMAKE_MACOSX_RPATH 1)
	if(DEFINED IOS_PLATFORM)
		file(GLOB FilesPlatform				${PROJECT_SOURCE_DIR}/sources/Platform/IOS/*.*)
	else()
		file(GLOB FilesPlatform				${PROJECT_SOURCE_DIR}/sources/Platform/MacOS/*.*)
	endif()
elseif(UNIX)
	file(GLOB FilesPlatform					${PROJECT_SOURCE_DIR}/sources/Platform/Linux/*.*)
endif()

# OpenGL renderer files
file(GLOB FilesRendererGL					${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/*.*)
file(GLOB FilesRendererGLShader				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/*.*)
file(GLOB FilesRendererGLExt				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Ext/*.*)
file(GLOB FilesRendererGLBuffer				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Buffer/*.*)
file(GLOB FilesRendererGLTexture			${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Texture/*.*)
file(GLOB FilesRendererGLRenderState		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/RenderState/*.*)
file(GLOB FilesRendererGLPlatformBase		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/*.*)

if(WIN32)
	file(GLOB FilesRendererGLPlatform		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Win32/*.*)
	file(GLOB FilesIncludePlatform			${PROJECT_INCLUDE_DIR}/LLGL/Platform/Win32/*.*)
elseif(APPLE)
	if(DEFINED IOS_PLATFORM)
		file(GLOB FilesRendererGLPlatform		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGLES/Platform/IOS/*.*)
		file(GLOB FilesIncludePlatform			${PROJECT_INCLUDE_DIR}/LLGL/Platform/IOS/*.*)
	else()
		file(GLOB FilesRendererGLPlatform		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/MacOS/*.*)
		file(GLOB FilesIncludePlatform			${PROJECT_INCLUDE_DIR}/LLGL/Platform/MacOS/*.*)
	endif()
elseif(UNIX)
	file(GLOB FilesRendererGLPlatform		${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Linux/*.*)
	file(GLOB FilesIncludePlatform			${PROJECT_INCLUDE_DIR}/LLGL/Platform/Linux/*.*)
endif()

# Direct3D12 renderer files
file(GLOB FilesRendererD3D12				${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/*.*)
file(GLOB FilesRendererD3D12Shader			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Shader/*.*)
file(GLOB FilesRendererD3D12Buffer			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Buffer/*.*)
file(GLOB FilesRendererD3D12Texture			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/Texture/*.*)
file(GLOB FilesRendererD3D12RenderState		${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/RenderState/*.*)

# Direct3D11 renderer files
file(GLOB FilesRendererD3D11				${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/*.*)
file(GLOB FilesRendererD3D11Shader			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Shader/*.*)
file(GLOB FilesRendererD3D11Buffer			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Buffer/*.*)
file(GLOB FilesRendererD3D11Texture			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/Texture/*.*)
file(GLOB FilesRendererD3D11RenderState		${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D11/RenderState/*.*)

# Vulkan renderer files
file(GLOB FilesRendererVulkan				${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/*.*)

# Test files
set(FilesTest1 ${PROJECT_SOURCE_DIR}/test/Test1_Window.cpp)
set(FilesTest2 ${PROJECT_SOURCE_DIR}/test/Test2_OpenGL.cpp)
set(FilesTest3 ${PROJECT_SOURCE_DIR}/test/Test3_Direct3D12.cpp)
set(FilesTest4 ${PROJECT_SOURCE_DIR}/test/Test4_Compute.cpp)

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


# === Source group folders ===

source_group("Include" FILES ${FilesInclude})
source_group("Include\\Platform" FILES ${FilesIncludePlatformBase} ${FilesIncludePlatform})
source_group("Sources\\Core" FILES ${FilesCore})

source_group(
	"Sources\\Platform" FILES
	${FilesPlatformBase} ${FilesPlatform}
)

source_group(
	"Sources\\Renderer" FILES
	${FilesRenderer}
	${FilesRendererGL}
	${FilesRendererD3D12}
	${FilesRendererD3D11}
	${FilesRendererVulkan}
)

if(LLGL_ENABLE_DEBUG_LAYER)
	source_group(
		"Sources\\Renderer\\DebugLayer" FILES
		${FilesRendererDbg}
	)
endif()

source_group(
	"Sources\\Renderer\\DXCommon" FILES
	${FilesRendererDXCommon}
)

source_group(
	"Sources\\Renderer\\Platform" FILES
	${FilesRendererGLPlatformBase}
	${FilesRendererGLPlatform}
)

source_group(
	"Sources\\Renderer\\Ext" FILES
	${FilesRendererGLExt}
)

source_group(
	"Sources\\Renderer\\Shader" FILES
	${FilesRendererGLShader}
	${FilesRendererD3D12Shader}
	${FilesRendererD3D11Shader}
)

source_group(
	"Sources\\Renderer\\Buffer" FILES
	${FilesRendererGLBuffer}
	${FilesRendererD3D12Buffer}
	${FilesRendererD3D11Buffer}
)

source_group(
	"Sources\\Renderer\\Texture" FILES
	${FilesRendererGLTexture}
	${FilesRendererD3D12Texture}
	${FilesRendererD3D11Texture}
)

source_group(
	"Sources\\Renderer\\RenderState" FILES
	${FilesRendererGLRenderState}
	${FilesRendererD3D12RenderState}
	${FilesRendererD3D11RenderState}
)


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")

find_path(GaussianLib_INCLUDE_DIR NAMES Gauss/Gauss.h)
include_directories(${GaussianLib_INCLUDE_DIR})


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
	${FilesRendererGLExt}
	${FilesRendererGLPlatformBase}
	${FilesRendererGLPlatform}
	${FilesRendererGLShader}
	${FilesRendererGLBuffer}
	${FilesRendererGLTexture}
	${FilesRendererGLRenderState}
)

#TODO: replace ...GL variables by ...GLES2 variables
set(
	FilesGLES2
	${FilesRendererGL}
	${FilesRendererGLExt}
	${FilesRendererGLPlatformBase}
	${FilesRendererGLPlatform}
	${FilesRendererGLShader}
	${FilesRendererGLBuffer}
	${FilesRendererGLTexture}
	${FilesRendererGLRenderState}
)

set(
	FilesD3D12
	${FilesRendererD3D12}
	${FilesRendererD3D12Shader}
	${FilesRendererD3D12Buffer}
	${FilesRendererD3D12Texture}
	${FilesRendererD3D12RenderState}
	${FilesRendererDXCommon}
)

set(
	FilesD3D11
	${FilesRendererD3D11}
	${FilesRendererD3D11Shader}
	${FilesRendererD3D11Buffer}
	${FilesRendererD3D11Texture}
	${FilesRendererD3D11RenderState}
	${FilesRendererDXCommon}
)

# Base project
add_library(LLGL SHARED ${FilesLLGL})

if(APPLE)
	if(DEFINED IOS_PLATFORM)
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

if(LLGL_BUILD_RENDERER_OPENGLES2)
		# OpenGLES Renderer
		include(cmake/FindOpenGLES.cmake)
		if(OPENGLES_FOUND)
			ADD_DEFINE(LLGL_GL_OPENGLES)
			
			include_directories(${OPENGLES_INCLUDE_DIR})
			add_library(LLGL_OpenGLES2 SHARED ${FilesGLES2})
			set_target_properties(LLGL_OpenGLES2 PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
			target_link_libraries(LLGL_OpenGLES2 LLGL ${OPENGLES_LIBRARIES})
			
			if(DEFINED IOS_PLATFORM)
				ADD_FRAMEWORK(LLGL_OpenGLES2 UIKit)
				ADD_FRAMEWORK(LLGL_OpenGLES2 QuartzCore)
				ADD_FRAMEWORK(LLGL_OpenGLES2 OpenGLES)
			else()
				ENABLE_CXX11(LLGL_OpenGLES2)
			endif()
		else()
			message("Missing OpenGL ES -> LLGL_OpenGLES2 renderer will be excluded from project")
		endif()
endif()

if(LLGL_BUILD_RENDERER_OPENGL)
	# OpenGL Renderer
	find_package(OpenGL)
	if(OpenGL_FOUND)
		include_directories(${OPENGL_INCLUDE_DIR})
		add_library(LLGL_OpenGL SHARED ${FilesGL})
		set_target_properties(LLGL_OpenGL PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
		target_link_libraries(LLGL_OpenGL LLGL ${OPENGL_LIBRARIES})
		ENABLE_CXX11(LLGL_OpenGL)
	else()
		message("Missing OpenGL -> LLGL_OpenGL renderer will be excluded from project")
	endif()
endif()

if(WIN32)
	if(LLGL_BUILD_RENDERER_DIRECT3D11)
		# Direct3D 11 Renderer
		add_library(LLGL_Direct3D11 SHARED ${FilesD3D11})
		set_target_properties(LLGL_Direct3D11 PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
		target_link_libraries(LLGL_Direct3D11 LLGL d3d11 dxgi D3DCompiler)
		ENABLE_CXX11(LLGL_Direct3D11)
	endif()
	
	if(LLGL_BUILD_RENDERER_DIRECT3D12)
		# Direct3D 12 Renderer
		add_library(LLGL_Direct3D12 SHARED ${FilesD3D12})
		set_target_properties(LLGL_Direct3D12 PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
		target_link_libraries(LLGL_Direct3D12 LLGL d3d12 dxgi D3DCompiler)
		ENABLE_CXX11(LLGL_Direct3D12)
	endif()
endif()

# Test Projects
if(LLGL_BUILD_TESTS)
	ADD_TEST_PROJECT(Test1_Window ${FilesTest1})
	ADD_TEST_PROJECT(Test2_OpenGL ${FilesTest2})
	if(WIN32)
		ADD_TEST_PROJECT(Test3_Direct3D12 ${FilesTest3})
	endif()
	ADD_TEST_PROJECT(Test4_Compute ${FilesTest4})
endif()

# Tutorial Projects
if(LLGL_BUILD_TUTORIALS)
	ADD_TEST_PROJECT(Tutorial01_HelloTriangle ${FilesTutorial01})
	ADD_TEST_PROJECT(Tutorial02_Tessellation ${FilesTutorial02})
	ADD_TEST_PROJECT(Tutorial03_Texturing ${FilesTutorial03})
	ADD_TEST_PROJECT(Tutorial04_Query ${FilesTutorial04})
	ADD_TEST_PROJECT(Tutorial05_RenderTarget ${FilesTutorial05})
	ADD_TEST_PROJECT(Tutorial06_MultiContext ${FilesTutorial06})
	ADD_TEST_PROJECT(Tutorial07_Array ${FilesTutorial07})
	ADD_TEST_PROJECT(Tutorial08_Compute ${FilesTutorial08})
	ADD_TEST_PROJECT(Tutorial09_StreamOutput ${FilesTutorial09})
	ADD_TEST_PROJECT(Tutorial10_Instancing ${FilesTutorial10})
endif()


