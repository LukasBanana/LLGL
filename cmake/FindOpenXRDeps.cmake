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

        # Build the loader statically on desktop, dynamically on Android.
        #
        # The SDK's loader CMakeLists adds an unconditional Visual Studio POST_BUILD step that
        # xcopies the built loader DLL from the loader target's *own* output dir into the SDK's
        # sample/test dirs (hello_xr/loader_test/conformance). We never build those, and any
        # consuming project that sets a global CMAKE_RUNTIME_OUTPUT_DIRECTORY redirects the DLL
        # elsewhere, so that xcopy fails the whole build with "File not found - openxr_loader*.dll".
        # A static loader skips that block entirely and removes the DLL-deployment burden on
        # desktop consumers (the loader still discovers the active XR runtime via the registry).
        # Android keeps the dynamic loader because the APK ships libopenxr_loader.so (see BuildAndroid.sh).
        if(ANDROID)
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
endif()
