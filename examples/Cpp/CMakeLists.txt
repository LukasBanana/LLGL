#
# CMakeLists.txt file for LLGL C++ example projects
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

if (NOT DEFINED CMAKE_MINIMUM_REQUIRED_VERSION)
    cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
endif()

project(LLGL_ExamplesCpp)


# === Source files ===

# Example project files
find_source_files(FilesExampleBase CXX "${EXAMPLE_CPP_PROJECTS_DIR}/ExampleBase")

if(APPLE)
    if(LLGL_IOS_PLATFORM)
        find_source_files(FilesExampleBaseIOS OBJC "${SHARED_PLATFORM_DIR}/iOS")
    else()
        find_source_files(FilesExampleBaseMacOS OBJC "${SHARED_PLATFORM_DIR}/macOS")
    endif()
elseif(LLGL_ANDROID_PLATFORM)
    find_source_files(FilesExampleBaseAndroid CXX "${SHARED_PLATFORM_DIR}/Android")
endif()

set(FilesExampleBaseAll ${FilesExampleBase})
if(APPLE)
    if(LLGL_IOS_PLATFORM)
        list(APPEND FilesExampleBaseAll ${FilesExampleBaseIOS})
    else()
        list(APPEND FilesExampleBaseAll ${FilesExampleBaseMacOS})
    endif()
    set_source_files_properties("${EXAMPLE_CPP_PROJECTS_DIR}/ExampleBase/ExampleBase.cpp" PROPERTIES COMPILE_FLAGS -xobjective-c++)
elseif(LLGL_ANDROID_PLATFORM)
    list(APPEND FilesExampleBaseAll ${FilesExampleBaseAndroid})
endif()

find_project_source_files( FilesExample_Animation           "${EXAMPLE_CPP_PROJECTS_DIR}/Animation"        )
find_project_source_files( FilesExample_ClothPhysics        "${EXAMPLE_CPP_PROJECTS_DIR}/ClothPhysics"     )
find_project_source_files( FilesExample_Fonts               "${EXAMPLE_CPP_PROJECTS_DIR}/Fonts"            )
find_project_source_files( FilesExample_HelloGame           "${EXAMPLE_CPP_PROJECTS_DIR}/HelloGame"        )
find_project_source_files( FilesExample_HelloTriangle       "${EXAMPLE_CPP_PROJECTS_DIR}/HelloTriangle"    )
find_project_source_files( FilesExample_Instancing          "${EXAMPLE_CPP_PROJECTS_DIR}/Instancing"       )
find_project_source_files( FilesExample_IndirectDraw        "${EXAMPLE_CPP_PROJECTS_DIR}/IndirectDraw"     )
find_project_source_files( FilesExample_MultiContext        "${EXAMPLE_CPP_PROJECTS_DIR}/MultiContext"     )
find_project_source_files( FilesExample_MultiRenderer       "${EXAMPLE_CPP_PROJECTS_DIR}/MultiRenderer"    )
find_project_source_files( FilesExample_MultiThreading      "${EXAMPLE_CPP_PROJECTS_DIR}/MultiThreading"   )
find_project_source_files( FilesExample_PBR                 "${EXAMPLE_CPP_PROJECTS_DIR}/PBR"              )
find_project_source_files( FilesExample_PostProcessing      "${EXAMPLE_CPP_PROJECTS_DIR}/PostProcessing"   )
find_project_source_files( FilesExample_Queries             "${EXAMPLE_CPP_PROJECTS_DIR}/Queries"          )
find_project_source_files( FilesExample_RenderTarget        "${EXAMPLE_CPP_PROJECTS_DIR}/RenderTarget"     )
find_project_source_files( FilesExample_ResourceBinding     "${EXAMPLE_CPP_PROJECTS_DIR}/ResourceBinding"  )
find_project_source_files( FilesExample_ShadowMapping       "${EXAMPLE_CPP_PROJECTS_DIR}/ShadowMapping"    )
find_project_source_files( FilesExample_StencilBuffer       "${EXAMPLE_CPP_PROJECTS_DIR}/StencilBuffer"    )
find_project_source_files( FilesExample_Tessellation        "${EXAMPLE_CPP_PROJECTS_DIR}/Tessellation"     )
find_project_source_files( FilesExample_Texturing           "${EXAMPLE_CPP_PROJECTS_DIR}/Texturing"        )
find_project_source_files( FilesExample_VolumeRendering     "${EXAMPLE_CPP_PROJECTS_DIR}/VolumeRendering"  )

find_project_source_files( FilesExample_HelloUWP            "${EXAMPLE_CPP_PROJECTS_DIR}/HelloUWP"         )


# === Source group folders ===

source_group("Sources" FILES ${FilesExampleBase})

if(APPLE)
    if(LLGL_IOS_PLATFORM)
        source_group("Sources\\iOS" FILES ${FilesExampleBaseIOS})
    else()
        source_group("Sources\\macOS" FILES ${FilesExampleBaseMacOS})
    endif()
elseif(LLGL_ANDROID_PLATFORM)
    source_group("Sources\\Android" FILES ${FilesExampleBaseAndroid})
endif()


# === Include directories ===

include_directories("${EXTERNAL_INCLUDE_DIR}") # <stb/stb_image.h>
include_directories("${EXAMPLE_CPP_PROJECTS_DIR}/ExampleBase")
include_directories("${SHARED_PLATFORM_DIR}")

if(GaussLib_INCLUDE_DIR)
    include_directories(${GaussLib_INCLUDE_DIR})
elseif(LLGL_BUILD_EXAMPLES)
    message(FATAL_ERROR "LLGL_BUILD_EXAMPLES failed: missing 'GaussLib_INCLUDE_DIR' path")
endif()


# === Projects ===

# Example Projects
if(LLGL_BUILD_EXAMPLES)
    # Base project (static lib) for all examples
    add_library(ExampleBase STATIC ${FilesExampleBaseAll})
    set_target_properties(ExampleBase PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
    target_link_libraries(ExampleBase "${LLGL_MODULE_LIBS}")
    set_target_properties(ExampleBase PROPERTIES FOLDER "Examples")
    
    # Native "app glue" for Android examples
    if(LLGL_ANDROID_PLATFORM)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
        target_link_libraries(ExampleBase android_native_app_glue log)
    endif(LLGL_ANDROID_PLATFORM)
    
    set(EXAMPLE_PROJECT_LIBS ExampleBase ${LLGL_MODULE_LIBS})

    # Common examples
    if(LLGL_UWP_PLATFORM)
        add_llgl_example_project(Example_HelloUWP           CXX "${FilesExample_HelloUWP}"          "${EXAMPLE_PROJECT_LIBS}")
    else()
        add_llgl_example_project(Example_Animation          CXX "${FilesExample_Animation}"         "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_ClothPhysics       CXX "${FilesExample_ClothPhysics}"      "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_IndirectDraw       CXX "${FilesExample_IndirectDraw}"      "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_Fonts              CXX "${FilesExample_Fonts}"             "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_HelloGame          CXX "${FilesExample_HelloGame}"         "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_HelloTriangle      CXX "${FilesExample_HelloTriangle}"     "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_Instancing         CXX "${FilesExample_Instancing}"        "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_MultiThreading     CXX "${FilesExample_MultiThreading}"    "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_PBR                CXX "${FilesExample_PBR}"               "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_PostProcessing     CXX "${FilesExample_PostProcessing}"    "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_Queries            CXX "${FilesExample_Queries}"           "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_RenderTarget       CXX "${FilesExample_RenderTarget}"      "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_ResourceBinding    CXX "${FilesExample_ResourceBinding}"   "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_ShadowMapping      CXX "${FilesExample_ShadowMapping}"     "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_StencilBuffer      CXX "${FilesExample_StencilBuffer}"     "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_Tessellation       CXX "${FilesExample_Tessellation}"      "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_Texturing          CXX "${FilesExample_Texturing}"         "${EXAMPLE_PROJECT_LIBS}")
        add_llgl_example_project(Example_VolumeRendering    CXX "${FilesExample_VolumeRendering}"   "${EXAMPLE_PROJECT_LIBS}")

        # Desktop specific examples
        if(NOT LLGL_MOBILE_PLATFORM)
            add_llgl_example_project(Example_MultiContext   CXX "${FilesExample_MultiContext}"      "${EXAMPLE_PROJECT_LIBS}")
            add_llgl_example_project(Example_MultiRenderer  CXX "${FilesExample_MultiRenderer}"     "${EXAMPLE_PROJECT_LIBS}")
        endif()
    endif()
endif(LLGL_BUILD_EXAMPLES)


