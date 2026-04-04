# FindWebGPU.cmake
#
# Finding the WebGPU (Dawn) implementation.
#
# Inputs:
#   WEBGPU_ROOT - (Optional) Path to the WebGPU SDK root
#
# Result Variables:
#   WebGPU_FOUND        - True if the system was found
#   WebGPU_INCLUDE_DIRS - The include directories
#   WebGPU_LIBRARIES    - The libraries to link against

# Define search hints
set(_WEBGPU_HINTS
    ${WEBGPU_ROOT}
    $ENV{WEBGPU_ROOT}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/dawn
)

# Look for the header
find_path(WebGPU_INCLUDE_DIR
    NAMES webgpu/webgpu.h
    PATHS ${_WEBGPU_HINTS}
    PATH_SUFFIXES include
)

# Look for the library (Dawn version)
find_library(WebGPU_LIBRARY
    NAMES webgpu_dawn
    PATHS ${_WEBGPU_HINTS}
    PATH_SUFFIXES lib bin
)

# Standard package handling
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WebGPU
    REQUIRED_VARS WebGPU_LIBRARY WebGPU_INCLUDE_DIR
)

if(WebGPU_FOUND)
    set(WebGPU_INCLUDE_DIRS ${WebGPU_INCLUDE_DIR})
    set(WebGPU_LIBRARIES ${WebGPU_LIBRARY})

    # Create the imported target
    if(NOT TARGET WebGPU::WebGPU)
        add_library(WebGPU::WebGPU UNKNOWN IMPORTED)
        set_target_properties(WebGPU::WebGPU PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${WebGPU_INCLUDE_DIRS}"
            IMPORTED_LOCATION "${WebGPU_LIBRARIES}"
        )
    endif()
endif()

mark_as_advanced(WebGPU_INCLUDE_DIR WebGPU_LIBRARY)
