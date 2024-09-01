#
# CMakeLists.txt file for LLGL C99 example projects
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_ExamplesC99)


# === Source files ===

# Example project files
find_source_files(FilesExampleBaseC99 C "${EXAMPLE_C99_PROJECTS_DIR}/ExampleBase")

if(LLGL_ANDROID_PLATFORM)
    find_source_files(FilesExampleBaseC99Android C "${SHARED_PLATFORM_DIR}/Android")
endif()

set(FilesExampleBaseAllC99 ${FilesExampleBaseC99})
if(LLGL_ANDROID_PLATFORM)
    list(APPEND FilesExampleBaseAllC99 ${FilesExampleBaseC99Android})
endif()

find_project_source_files( FilesExampleC99_HelloTriangle    "${EXAMPLE_C99_PROJECTS_DIR}/HelloTriangle" )
find_project_source_files( FilesExampleC99_Offscreen        "${EXAMPLE_C99_PROJECTS_DIR}/Offscreen"     )
find_project_source_files( FilesExampleC99_Texturing        "${EXAMPLE_C99_PROJECTS_DIR}/Texturing"     )


# === Source group folders ===

source_group("Sources" FILES ${FilesExampleBaseC99})

if(LLGL_ANDROID_PLATFORM)
    source_group("Sources\\Android" FILES ${FilesExampleBaseC99Android})
endif()


# === Include directories ===

include_directories("${EXTERNAL_INCLUDE_DIR}") # <stb/stb_image.h>
include_directories("${EXAMPLE_C99_PROJECTS_DIR}/ExampleBase")
include_directories("${SHARED_PLATFORM_DIR}")


# === Projects ===

# Example Projects
if(LLGL_BUILD_EXAMPLES AND LLGL_BUILD_WRAPPER_C99)
    # Base project (static lib) for all examples
    add_library(ExampleBase_C99 STATIC ${FilesExampleBaseAllC99})
    set_target_properties(ExampleBase_C99 PROPERTIES LINKER_LANGUAGE C DEBUG_POSTFIX "D")
    if(WIN32)
        target_link_libraries(ExampleBase_C99 "${LLGL_MODULE_LIBS}")
    else()
        target_link_libraries(ExampleBase_C99 "${LLGL_MODULE_LIBS}" m)
    endif()
    set_target_properties(ExampleBase_C99 PROPERTIES FOLDER "Examples (C99)")

    # Native "app glue" for Android examples
    if(LLGL_ANDROID_PLATFORM)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
        target_link_libraries(ExampleBase_C99 android_native_app_glue log)
    endif(LLGL_ANDROID_PLATFORM)

    set(EXAMPLE_C99_PROJECT_LIBS ExampleBase_C99 ${LLGL_MODULE_LIBS})

    # C99 wrapper examples
    if (NOT LLGL_ANDROID_PLATFORM)
        add_llgl_example_project(Example_C99_HelloTriangle  C "${FilesExampleC99_HelloTriangle}"    "${EXAMPLE_C99_PROJECT_LIBS}")
    endif()
    add_llgl_example_project(Example_C99_Offscreen      C "${FilesExampleC99_Offscreen}"        "${EXAMPLE_C99_PROJECT_LIBS}")
    add_llgl_example_project(Example_C99_Texturing      C "${FilesExampleC99_Texturing}"        "${EXAMPLE_C99_PROJECT_LIBS}")
endif()


