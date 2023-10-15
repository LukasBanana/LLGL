#
# CMakeLists.txt file for LLGL example projects
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

cmake_minimum_required(VERSION 3.7)

project(LLGL_Examples)


# === Source files ===

# Example project files
find_source_files(FilesExampleBase CXX "${EXAMPLEBASE_PROJECT_DIR}")

if(LLGL_ANDROID_PLATFORM)
    set(ANDROID_APP_GLUE_DIR "$ENV{ANDROID_NDK_ROOT}/sources/android/native_app_glue")
    set(
        FilesAndroidNativeAppGlue
        "${ANDROID_APP_GLUE_DIR}/android_native_app_glue.c"
        "${ANDROID_APP_GLUE_DIR}/android_native_app_glue.h"
    )
elseif(LLGL_IOS_PLATFORM)
    find_source_files(FilesExampleBaseIOS OBJC "${EXAMPLEBASE_PROJECT_DIR}/iOS")
endif()

set(FilesExampleBaseAll ${FilesExampleBase})

if(LLGL_IOS_PLATFORM)
    set(FilesExampleBaseAll ${FilesExampleBaseAll} ${FilesExampleBaseIOS})
endif()

if(APPLE)
    set_source_files_properties("${EXAMPLEBASE_PROJECT_DIR}/ExampleBase.cpp" PROPERTIES COMPILE_FLAGS -xobjective-c++)
endif()

find_project_source_files( FilesExample_Animation        "${EXAMPLE_CPP_PROJECTS_DIR}/Animation"        )
find_project_source_files( FilesExample_ClothPhysics     "${EXAMPLE_CPP_PROJECTS_DIR}/ClothPhysics"     )
find_project_source_files( FilesExample_ComputeShader    "${EXAMPLE_CPP_PROJECTS_DIR}/ComputeShader"    )
find_project_source_files( FilesExample_Fonts            "${EXAMPLE_CPP_PROJECTS_DIR}/Fonts"            )
find_project_source_files( FilesExample_HelloTriangle    "${EXAMPLE_CPP_PROJECTS_DIR}/HelloTriangle"    )
find_project_source_files( FilesExample_Instancing       "${EXAMPLE_CPP_PROJECTS_DIR}/Instancing"       )
find_project_source_files( FilesExample_Mapping          "${EXAMPLE_CPP_PROJECTS_DIR}/Mapping"          )
find_project_source_files( FilesExample_MultiContext     "${EXAMPLE_CPP_PROJECTS_DIR}/MultiContext"     )
find_project_source_files( FilesExample_MultiRenderer    "${EXAMPLE_CPP_PROJECTS_DIR}/MultiRenderer"    )
find_project_source_files( FilesExample_MultiThreading   "${EXAMPLE_CPP_PROJECTS_DIR}/MultiThreading"   )
find_project_source_files( FilesExample_PBR              "${EXAMPLE_CPP_PROJECTS_DIR}/PBR"              )
find_project_source_files( FilesExample_PostProcessing   "${EXAMPLE_CPP_PROJECTS_DIR}/PostProcessing"   )
find_project_source_files( FilesExample_Queries          "${EXAMPLE_CPP_PROJECTS_DIR}/Queries"          )
find_project_source_files( FilesExample_RenderTarget     "${EXAMPLE_CPP_PROJECTS_DIR}/RenderTarget"     )
find_project_source_files( FilesExample_ResourceBinding  "${EXAMPLE_CPP_PROJECTS_DIR}/ResourceBinding"  )
find_project_source_files( FilesExample_ShadowMapping    "${EXAMPLE_CPP_PROJECTS_DIR}/ShadowMapping"    )
find_project_source_files( FilesExample_StencilBuffer    "${EXAMPLE_CPP_PROJECTS_DIR}/StencilBuffer"    )
find_project_source_files( FilesExample_StreamOutput     "${EXAMPLE_CPP_PROJECTS_DIR}/StreamOutput"     )
find_project_source_files( FilesExample_Tessellation     "${EXAMPLE_CPP_PROJECTS_DIR}/Tessellation"     )
find_project_source_files( FilesExample_Texturing        "${EXAMPLE_CPP_PROJECTS_DIR}/Texturing"        )
find_project_source_files( FilesExample_VolumeRendering  "${EXAMPLE_CPP_PROJECTS_DIR}/VolumeRendering"  )

find_project_source_files( FilesExampleC99_HelloTriangle "${EXAMPLE_C99_PROJECTS_DIR}/HelloTriangle"    )

# Append extra resources for mobile platforms
if(LLGL_IOS_PLATFORM)
    add_project_resource_files(FilesExample_Fonts
        "FontAtlas_*.png"
        )
    add_project_resource_files(FilesExample_Texturing
        "Crate-DXT1-MipMapped.dds"
        "Crate.jpg"
        )
    add_project_resource_files(FilesExample_PostProcessing
        "WiredBox.obj"
        )
    add_project_resource_files(FilesExample_ClothPhysics
        "Logo_LLGL.png"
        )
    add_project_resource_files(FilesExample_RenderTarget
        "Crate.jpg"
        )
    add_project_resource_files(FilesExample_ResourceBinding
        "Crate.jpg"
        "TilesGray512.jpg"
        "TilesBlue512.jpg"
        "UVSphere.obj"
        )
    add_project_resource_files(FilesExample_StencilBuffer
        "Portal-*.obj"
        "WiredBox.obj"
        "Pyramid.obj"
        )
    add_project_resource_files(FilesExample_ShadowMapping
        "SimpleRoom.obj"
        "WiredBox.obj"
        )
    add_project_resource_files(FilesExample_Animation
        "PenroseStairs-*.obj"
        "IcoSphere.obj"
        "TilesGray512.jpg"
        )
    add_project_resource_files(FilesExample_VolumeRendering
        "Suzanne.obj"
        )
    add_project_resource_files(FilesExample_Instancing
        "Grass.jpg"
        "Plants_*.png"
        )
endif()


# === Source group folders ===

source_group("Sources" FILES ${FilesExampleBase})

if(LLGL_ANDROID_PLATFORM)
    source_group("android_native_app_glue" FILES ${FilesAndroidNativeAppGlue})
elseif(LLGL_IOS_PLATFORM)
    source_group("Sources\\iOS" FILES ${FilesExampleBaseIOS})
endif()


# === Include directories ===

include_directories("${EXAMPLEBASE_PROJECT_DIR}")

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
        add_library(android_native_app_glue STATIC ${FilesAndroidNativeAppGlue})
        target_link_libraries(ExampleBase android_native_app_glue)
        include_directories(${ANDROID_APP_GLUE_DIR})
    endif(LLGL_ANDROID_PLATFORM)
    
    set(EXAMPLE_PROJECT_LIBS ExampleBase ${LLGL_MODULE_LIBS})
    
    if(LLGL_ANDROID_PLATFORM)
        # Android specific examples
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")
    endif()

    # Common examples
    add_llgl_example_project(Example_Animation          CXX "${FilesExample_Animation}"         "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_ClothPhysics       CXX "${FilesExample_ClothPhysics}"      "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_ComputeShader      CXX "${FilesExample_ComputeShader}"     "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_Fonts              CXX "${FilesExample_Fonts}"             "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_HelloTriangle      CXX "${FilesExample_HelloTriangle}"     "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_Instancing         CXX "${FilesExample_Instancing}"        "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_Mapping            CXX "${FilesExample_Mapping}"           "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_MultiContext       CXX "${FilesExample_MultiContext}"      "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_MultiRenderer      CXX "${FilesExample_MultiRenderer}"     "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_MultiThreading     CXX "${FilesExample_MultiThreading}"    "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_PBR                CXX "${FilesExample_PBR}"               "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_PostProcessing     CXX "${FilesExample_PostProcessing}"    "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_Queries            CXX "${FilesExample_Queries}"           "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_RenderTarget       CXX "${FilesExample_RenderTarget}"      "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_ResourceBinding    CXX "${FilesExample_ResourceBinding}"   "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_ShadowMapping      CXX "${FilesExample_ShadowMapping}"     "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_StencilBuffer      CXX "${FilesExample_StencilBuffer}"     "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_StreamOutput       CXX "${FilesExample_StreamOutput}"      "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_Tessellation       CXX "${FilesExample_Tessellation}"      "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_Texturing          CXX "${FilesExample_Texturing}"         "${EXAMPLE_PROJECT_LIBS}")
    add_llgl_example_project(Example_VolumeRendering    CXX "${FilesExample_VolumeRendering}"   "${EXAMPLE_PROJECT_LIBS}")

    if (LLGL_BUILD_WRAPPER_C99)
        # C99 wrapper examples
        add_llgl_example_project(Example_C99_HelloTriangle C "${FilesExampleC99_HelloTriangle}" "${EXAMPLE_PROJECT_LIBS}")
    endif(LLGL_BUILD_WRAPPER_C99)
endif(LLGL_BUILD_EXAMPLES)


