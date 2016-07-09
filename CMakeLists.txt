
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
endif(WIN32)


# === Global files ===

file(GLOB FilesInclude		${PROJECT_INCLUDE_DIR}/LLGL/*.*)
file(GLOB FilesCore			${PROJECT_SOURCE_DIR}/sources/Core/*.*)
file(GLOB FilesPlatformBase	${PROJECT_SOURCE_DIR}/sources/Platform/*.*)

if(WIN32)
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/sources/Platform/Win32/*.*)
elseif(APPLE)
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/sources/Platform/MacOS/*.*)
elseif(UNIX)
	file(GLOB FilesPlatform ${PROJECT_SOURCE_DIR}/sources/Platform/Linux/*.*)
endif()

set(FilesTest1 ${PROJECT_SOURCE_DIR}/test/test1.cpp)


# === Source group folders ===

# --- Dyancmis ---

source_group("Include" FILES ${FilesInclude})
source_group("Sources\\Core" FILES ${FilesCommon})
source_group("Sources\\Platform" FILES ${FilesPlatformBase} ${FilesPlatform})


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")


# === Binaries ===

set(
	FilesAll
	${FilesInclude}
	${FilesCore}
	${FilesPlatformBase}
	${FilesPlatform}
)

add_library(LLGL SHARED ${FilesAll})
set_target_properties("LLGL" PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")

add_executable(test1 ${FilesTest1})
target_link_libraries(test1 LLGL)
set_target_properties("test1" PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")


