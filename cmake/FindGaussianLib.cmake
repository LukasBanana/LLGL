
# Custom CMake module for finding "GaussianLib" files
# Written by Lukas Hermanns on 24/08/2015

# === Options ===

option(GaussLib_ENABLE_SSE "Enable SSE support" OFF)
option(GaussLib_ENABLE_SSE2 "Enable SSE2 support" OFF)

if(GaussLib_ENABLE_SSE)
	add_definitions(-DGS_ENABLE_SSE)
endif()

if(GaussLib_ENABLE_SSE2)
	add_definitions(-DGS_ENABLE_SSE2)
endif()

if(UNIX AND (GaussLib_ENABLE_SSE OR GaussLib_ENABLE_SSE2))
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1")
endif()


# === Find library ===

find_path(GaussLib_INCLUDE_DIR Gauss/Gauss.h)

include_directories(${GaussLib_INCLUDE_DIR})

