
# Custom CMake module for finding "LLGL" files
# Written by Lukas Hermanns on 14/07/2016

# Macros

macro(_LLGL_APPEND_LIBRARIES _list _release)
	set(_debug ${_release}_DEBUG)
	if(${_debug})
		set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
	else()
		set(${_list} ${${_list}} ${${_release}})
	endif()
endmacro()

# Find library

find_path(LLGL_INCLUDE_DIR LLGL/LLGL.h)

find_library(LLGL_LIBRARY NAMES LLGL)
find_library(LLGL_LIBRARY_DEBUG NAMES LLGLD)

# Setup package handle

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(
	LLGL
	DEFAULT_MSG
	LLGL_INCLUDE_DIR
    LLGL_LIBRARY
    LLGL_LIBRARY_DEBUG
)

if(LLGL_FOUND)
	_LLGL_APPEND_LIBRARIES(LLGL_LIBRARIES LLGL_LIBRARY)
endif(LLGL_FOUND)
