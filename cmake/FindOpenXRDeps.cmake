#
# FindOpenXRDeps.cmake
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#
# Resolves the OpenXR SDK dependency. Prefers a system-installed OpenXR via
# find_package(OpenXR CONFIG); if that fails and FetchContent is permitted,
# the official Khronos OpenXR-SDK is downloaded at configure time.
#
# Outputs:
#   OPENXR_FOUND                  : TRUE if a usable OpenXR target was resolved.
#   OPENXR_HEADER_TARGET          : Imported target providing the OpenXR headers.
#   OPENXR_LOADER_TARGET          : Imported target providing the OpenXR loader to link against.
#   OPENXR_SOURCE                 : "system" or "fetch", for summary reporting.
#

set(OPENXR_FOUND FALSE)

# OpenXR SDK version to fetch when no system install is found.
# Pinned for reproducibility; bump intentionally.
set(LLGL_OPENXR_FETCH_TAG "release-1.1.41" CACHE STRING "Git tag of the Khronos OpenXR-SDK used when fetched at configure time.")

option(LLGL_OPENXR_ALLOW_FETCH "Allow fetching Khronos OpenXR-SDK at configure time if not found locally." ON)

# --- 1) Try a system-installed OpenXR via its config package ---
find_package(OpenXR CONFIG QUIET)
if(OpenXR_FOUND)
    if(TARGET OpenXR::headers AND TARGET OpenXR::openxr_loader)
        set(OPENXR_HEADER_TARGET OpenXR::headers)
        set(OPENXR_LOADER_TARGET OpenXR::openxr_loader)
        set(OPENXR_FOUND TRUE)
        set(OPENXR_SOURCE "system")
    endif()
endif()

# --- 2) Fall back to FetchContent if allowed ---
if(NOT OPENXR_FOUND AND LLGL_OPENXR_ALLOW_FETCH)
    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.14")
        include(FetchContent)

        # Suppress the SDK's own samples/tests/layers - we only need headers + loader.
        set(BUILD_API_LAYERS         OFF CACHE BOOL "" FORCE)
        set(BUILD_TESTS              OFF CACHE BOOL "" FORCE)
        set(BUILD_CONFORMANCE_TESTS  OFF CACHE BOOL "" FORCE)
        set(BUILD_LOADER             ON  CACHE BOOL "" FORCE)

        # Choose a shared vs static loader based on how LLGL itself is built.
        #
        # When LLGL is built as SHARED libraries (the default), each renderer backend is a separate module DLL
        # (LLGL_Vulkan, LLGL_Direct3D12, ...) and the XR graphics bindings live inside those DLLs, while the XR
        # frontend that calls xrCreateInstance lives elsewhere. A *static* loader would be linked into each of
        # those binaries separately, giving every module its own private copy of the loader's global instance
        # table -- so an XrInstance created by one module is rejected as "No active XrInstance handle"
        # (XR_ERROR_HANDLE_INVALID) when another module calls xrGetInstanceProcAddr on it. A single shared loader
        # DLL is imported by all modules, so they share one loader instance. The DLL is deployed next to the LLGL
        # binaries below, and its own output directory is pinned so the SDK's xcopy POST_BUILD step keeps working
        # even when a consuming project redirects binaries via a global CMAKE_RUNTIME_OUTPUT_DIRECTORY.
        #
        # When LLGL is built STATIC (LLGL_BUILD_STATIC_LIB), every module is linked into the single consuming
        # binary, so the static loader has exactly one copy and the duplication problem cannot occur. Keep it
        # static there: it stays embeddable with no loader-DLL deployment burden and skips the xcopy step entirely
        # (which is what motivated building the loader statically on desktop in the first place).
        #
        # Android always ships the dynamic loader (libopenxr_loader.so) in the APK (see BuildAndroid.sh).
        if(ANDROID OR NOT LLGL_BUILD_STATIC_LIB)
            set(DYNAMIC_LOADER       ON  CACHE BOOL "" FORCE)
        else()
            set(DYNAMIC_LOADER       OFF CACHE BOOL "" FORCE)
        endif()

        FetchContent_Declare(
            openxr_sdk
            GIT_REPOSITORY "https://github.com/KhronosGroup/OpenXR-SDK.git"
            GIT_TAG        "${LLGL_OPENXR_FETCH_TAG}"
            GIT_SHALLOW    TRUE
        )
        FetchContent_MakeAvailable(openxr_sdk)

        if(TARGET headers AND TARGET openxr_loader)
            set(OPENXR_HEADER_TARGET headers)
            set(OPENXR_LOADER_TARGET openxr_loader)
            set(OPENXR_FOUND TRUE)
            set(OPENXR_SOURCE "fetch")
        endif()
    else()
        message(WARNING "LLGL_OPENXR_ALLOW_FETCH requires CMake 3.14+; cannot fetch OpenXR-SDK")
    endif()
endif()

if(OPENXR_FOUND)
    # Group fetched OpenXR targets into a folder so they don't clutter the IDE root.
    foreach(_oxr_target headers openxr_loader generate_openxr_header xr_global_generated_files)
        if(TARGET ${_oxr_target})
            set_target_properties(${_oxr_target} PROPERTIES FOLDER "External/OpenXR")
        endif()
    endforeach()

    # For the fetched SHARED loader on desktop (LLGL built as shared libraries), pin its output directory and
    # deploy the DLL next to the LLGL binaries. Skipped for Android (the APK bundles libopenxr_loader.so), for a
    # system-installed loader (already on the loader search path), and for static LLGL builds (which keep the
    # static loader and have no loader DLL).
    if(OPENXR_SOURCE STREQUAL "fetch" AND NOT ANDROID AND NOT LLGL_BUILD_STATIC_LIB AND TARGET openxr_loader AND TARGET LLGL)
        # Pin the loader DLL to its own build directory (overriding any global CMAKE_RUNTIME_OUTPUT_DIRECTORY a
        # consuming project may set), so the SDK's POST_BUILD xcopy -- which copies from the loader's own dir into
        # the SDK sample dirs -- always finds its source and does not fail the build.
        set_target_properties(openxr_loader PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "${openxr_sdk_BINARY_DIR}/src/loader"
        )

        # Deploy the loader DLL next to the LLGL binaries so applications can load it at runtime. The copy is
        # attached to the core LLGL target because add_custom_command(TARGET ...) must be issued in the directory
        # that created the target, and openxr_loader is created in the FetchContent sub-build; force the loader to
        # build before LLGL so its DLL exists when the copy runs.
        add_dependencies(LLGL openxr_loader)
        add_custom_command(
            TARGET LLGL POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:openxr_loader>" "$<TARGET_FILE_DIR:LLGL>"
            COMMENT "Deploying OpenXR loader DLL next to LLGL binaries"
            VERBATIM
        )
    endif()
endif()
