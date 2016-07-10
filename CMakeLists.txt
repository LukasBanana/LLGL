
# === CMake lists for the LLGL - (09/07/2015) ===

cmake_minimum_required(VERSION 2.8)
project(LLGL)


# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")


# === Preprocessor definitions ===

macro(add_define IDENT)
	if(MSVC)
		add_definitions(/D${IDENT})
	else(MSVC)
		add_definitions(-D${IDENT})
	endif(MSVC)
endmacro()

if(WIN32)
	add_define(_CRT_SECURE_NO_WARNINGS)
	add_define(_SCL_SECURE_NO_WARNINGS)
	add_define(UNICODE)
	add_define(NOMINMAX)
	if(MSVC)
		# Disable some warnings for MSVC compiler
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4251 /wd4290 /wd4103")
	endif(MSVC)
endif(WIN32)


# === Global files ===

file(GLOB FilesInclude		${PROJECT_INCLUDE_DIR}/LLGL/*.*)
file(GLOB FilesCore			${PROJECT_SOURCE_DIR}/sources/Core/*.*)
file(GLOB FilesPlatformBase	${PROJECT_SOURCE_DIR}/sources/Platform/*.*)

if(WIN32)
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/sources/Platform/Win32/*.*)
elseif(APPLE)
	set(CMAKE_MACOSX_RPATH 1)
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/sources/Platform/MacOS/*.*)
elseif(UNIX)
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/sources/Platform/Linux/*.*)
endif()

set(FilesTest1 ${PROJECT_SOURCE_DIR}/test/test1.cpp)


# === Source group folders ===

# --- Dyancmis ---

source_group("Include" FILES ${FilesInclude})
source_group("Sources\\Core" FILES ${FilesCore})
source_group("Sources\\Platform" FILES ${FilesPlatformBase} ${FilesPlatform})


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")

find_path(GaussianLib_INCLUDE_DIR NAMES Gauss/Gauss.h)
include_directories(${GaussianLib_INCLUDE_DIR})


# === Binaries ===

set(
	FilesAll
	${FilesInclude}
	${FilesCore}
	${FilesPlatformBase}
	${FilesPlatform}
)

add_library(LLGL SHARED ${FilesAll})

if(APPLE)
	find_library(COCOA_LIBRARY Cocoa)
	target_link_libraries(LLGL ${COCOA_LIBRARY})
elseif(UNIX)
	target_link_libraries(LLGL X11)
endif()

set_target_properties("LLGL" PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
target_compile_features(LLGL PRIVATE cxx_range_for)

if(APPLE)
	add_executable(test1 MACOSX_BUNDLE ${FilesTest1})
else()
	add_executable(test1 ${FilesTest1})
endif()

target_link_libraries(test1 LLGL)
set_target_properties("test1" PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
target_compile_features(test1 PRIVATE cxx_range_for)


