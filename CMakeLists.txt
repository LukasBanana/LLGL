
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

macro(ADD_TEST_PROJECT TEST_NAME TEST_FILES)
	if(APPLE)
		add_executable(${TEST_NAME} MACOSX_BUNDLE ${TEST_FILES})
	else()
		add_executable(${TEST_NAME} ${TEST_FILES})
	endif()

	target_link_libraries(${TEST_NAME} LLGL)
	set_target_properties(${TEST_NAME} PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
	target_compile_features(${TEST_NAME} PRIVATE cxx_range_for)
endmacro()

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


# === Global files ===

# Common files
file(GLOB FilesInclude						${PROJECT_INCLUDE_DIR}/LLGL/*.*)
file(GLOB FilesCore							${PROJECT_SOURCE_DIR}/sources/Core/*.*)
file(GLOB FilesPlatformBase					${PROJECT_SOURCE_DIR}/sources/Platform/*.*)
file(GLOB FilesRenderer						${PROJECT_SOURCE_DIR}/sources/Renderer/*.*)

if(WIN32)
	file(GLOB FilesPlatform					${PROJECT_SOURCE_DIR}/sources/Platform/Win32/*.*)
elseif(APPLE)
	set(CMAKE_MACOSX_RPATH 1)
	file(GLOB FilesPlatform					${PROJECT_SOURCE_DIR}/sources/Platform/MacOS/*.*)
elseif(UNIX)
	file(GLOB FilesPlatform					${PROJECT_SOURCE_DIR}/sources/Platform/Linux/*.*)
endif()

# OpenGL renderer files
file(GLOB FilesRendererOpenGL				${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/*.*)
file(GLOB FilesRendererOpenGLShader			${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Shader/*.*)

if(WIN32)
	file(GLOB FilesRendererOpenGLPlatform	${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Win32/*.*)
elseif(APPLE)
	file(GLOB FilesRendererOpenGLPlatform	${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/MacOS/*.*)
elseif(UNIX)
	file(GLOB FilesRendererOpenGLPlatform	${PROJECT_SOURCE_DIR}/sources/Renderer/OpenGL/Platform/Linux/*.*)
endif()

# Direct3D12 renderer files
file(GLOB FilesRendererDirect3D12			${PROJECT_SOURCE_DIR}/sources/Renderer/Direct3D12/*.*)

# Vulkan renderer files
file(GLOB FilesRendererVulkan				${PROJECT_SOURCE_DIR}/sources/Renderer/Vulkan/*.*)

# Test files
set(FilesTest1 ${PROJECT_SOURCE_DIR}/test/Test1_Window.cpp)
set(FilesTest2 ${PROJECT_SOURCE_DIR}/test/Test2_OpenGL.cpp)


# === Source group folders ===

source_group("Include" FILES ${FilesInclude})
source_group("Sources\\Core" FILES ${FilesCore})

source_group(
	"Sources\\Platform" FILES
	${FilesPlatformBase} ${FilesPlatform}
)

source_group(
	"Sources\\Renderer" FILES
	${FilesRenderer}
	${FilesRendererOpenGL}
	${FilesRendererDirect3D12}
	${FilesRendererVulkan}
)

source_group(
	"Sources\\Renderer\\Platform" FILES
	${FilesRendererOpenGLPlatform}
)

source_group(
	"Sources\\Renderer\\Shader" FILES
	${FilesRendererOpenGLShader}
)


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")

find_path(GaussianLib_INCLUDE_DIR NAMES Gauss/Gauss.h)
include_directories(${GaussianLib_INCLUDE_DIR})


# === Projects ===

set(
	FilesLLGL
	${FilesInclude}
	${FilesCore}
	${FilesPlatformBase}
	${FilesPlatform}
	${FilesRenderer}
)

# Base project
add_library(LLGL SHARED ${FilesLLGL})

if(APPLE)
	find_library(COCOA_LIBRARY Cocoa)
	target_link_libraries(LLGL ${COCOA_LIBRARY})
elseif(UNIX)
	target_link_libraries(LLGL X11)
endif()

set_target_properties(LLGL PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
target_compile_features(LLGL PRIVATE cxx_range_for)

# OpenGL Renderer
find_package(OpenGL)
if(OpenGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIR})
	add_library(LLGL_OpenGL SHARED ${FilesRendererOpenGL} ${FilesRendererOpenGLPlatform} ${FilesRendererOpenGLShader})
	set_target_properties(LLGL_OpenGL PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
	target_link_libraries(LLGL_OpenGL LLGL ${OPENGL_LIBRARIES})
	target_compile_features(LLGL_OpenGL PRIVATE cxx_range_for)
else()
	message("Missing OpenGL -> LLGL_OpenGL renderer will be excluded from project")
endif()

# Test Projects
ADD_TEST_PROJECT(Test1_Window ${FilesTest1})
ADD_TEST_PROJECT(Test2_OpenGL ${FilesTest2})


