#
# CMakeLists.txt file for LLGL C99 example projects
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

cmake_minimum_required(VERSION 3.7)

project(LLGL_ExamplesC99)


# === Source files ===

# Example project files
find_source_files(FilesExampleBaseC99 C "${EXAMPLE_C99_PROJECTS_DIR}/ExampleBase")

find_project_source_files( FilesExampleC99_HelloTriangle    "${EXAMPLE_C99_PROJECTS_DIR}/HelloTriangle" )
find_project_source_files( FilesExampleC99_Offscreen        "${EXAMPLE_C99_PROJECTS_DIR}/Offscreen" )
find_project_source_files( FilesExampleC99_Texturing        "${EXAMPLE_C99_PROJECTS_DIR}/Texturing"     )


# === Source group folders ===

source_group("Sources" FILES ${FilesExampleBaseC99})


# === Include directories ===

include_directories("${EXTERNAL_INCLUDE_DIR}") # <stb/stb_image.h>
include_directories("${EXAMPLE_C99_PROJECTS_DIR}/ExampleBase")

if(GaussLib_INCLUDE_DIR)
    include_directories(${GaussLib_INCLUDE_DIR})
elseif(LLGL_BUILD_EXAMPLES)
    message(FATAL_ERROR "LLGL_BUILD_EXAMPLES failed: missing 'GaussLib_INCLUDE_DIR' path")
endif()


# === Projects ===

# Example Projects
if(LLGL_BUILD_EXAMPLES AND LLGL_BUILD_WRAPPER_C99)
    # Base project (static lib) for all examples
    add_library(ExampleBase_C99 STATIC ${FilesExampleBaseC99})
    set_target_properties(ExampleBase_C99 PROPERTIES LINKER_LANGUAGE C DEBUG_POSTFIX "D")
    if(WIN32)
        target_link_libraries(ExampleBase_C99 "${LLGL_MODULE_LIBS}")
    else()
        target_link_libraries(ExampleBase_C99 "${LLGL_MODULE_LIBS}" m)
    endif()
    set_target_properties(ExampleBase_C99 PROPERTIES FOLDER "Examples (C99)")
    set(EXAMPLE_C99_PROJECT_LIBS ExampleBase_C99 ${LLGL_MODULE_LIBS})
    
    # C99 wrapper examples
    add_llgl_example_project(Example_C99_HelloTriangle  C "${FilesExampleC99_HelloTriangle}"    "${EXAMPLE_C99_PROJECT_LIBS}")
    add_llgl_example_project(Example_C99_Offscreen      C "${FilesExampleC99_Offscreen}"        "${EXAMPLE_C99_PROJECT_LIBS}")
    add_llgl_example_project(Example_C99_Texturing      C "${FilesExampleC99_Texturing}"        "${EXAMPLE_C99_PROJECT_LIBS}")
endif()


