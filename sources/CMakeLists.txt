
# === CMake lists for the LLGL - (09/07/2015) ===

cmake_minimum_required(VERSION 2.8)
project(LLGL)


# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../include")


# === Global files ===

file(GLOB_RECURSE FilesAll		${PROJECT_SOURCE_DIR}/*.*)

file(GLOB FilesInclude			${PROJECT_INCLUDE_DIR}/*.*)
file(GLOB FilesIncludeDetails	${PROJECT_INCLUDE_DIR}/details/*.*)
file(GLOB FilesCommon			${PROJECT_SOURCE_DIR}/Common/*.*)


# === Source group folders ===

# --- Dyancmis ---

source_group("Include" FILES ${FilesInclude})
source_group("Include\\Details" FILES ${FilesIncludeDetails})
source_group("Common" FILES ${FilesCommon})


# === Include directories ===

include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_INCLUDE_DIR}")


# === Binaries ===

add_library(LLGL SHARED ${FilesAll} ${FilesInclude} ${FilesIncludeDetails})

set_target_properties("LLGL" PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")


