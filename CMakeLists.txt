#
# CMakeLists.txt file for LLGL project
#
# Copyright (c) 2015 Lukas Hermanns. All rights reserved.
# Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
#

# Originally authored on 09/07/2015

cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(LLGL)

# === Target platforms ===

if("${CMAKE_GENERATOR_PLATFORM}" STREQUAL "x64")
    set(LLGL_BUILD_64BIT ON)
else()
    set(LLGL_BUILD_64BIT OFF)
endif()

set(LLGL_UWP_PLATFORM OFF)
set(LLGL_IOS_PLATFORM OFF)
set(LLGL_ANDROID_PLATFORM OFF)
set(LLGL_WASM_PLATFORM OFF)

if(NOT DEFINED LLGL_TARGET_PLATFORM)
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "Android")
        set(LLGL_TARGET_PLATFORM "Android")
        if(NOT DEFINED ENV{ANDROID_NDK_ROOT})
            message(FATAL_ERROR "Cannot build for Android platform: Missing environment variable 'ANDROID_NDK_ROOT'")
        endif()
        set(LLGL_ANDROID_PLATFORM ON)
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore")
        set(LLGL_TARGET_PLATFORM "${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_VERSION}")
        set(LLGL_UWP_PLATFORM ON)
    elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
        set(LLGL_TARGET_PLATFORM "WebAssembly")
        set(LLGL_WASM_PLATFORM ON)
    elseif(WIN32)
        if(LLGL_BUILD_64BIT)
            set(LLGL_TARGET_PLATFORM "Win64")
        else()
            set(LLGL_TARGET_PLATFORM "Win32")
        endif()
    elseif(APPLE)
        if("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS")
            set(LLGL_TARGET_PLATFORM "iOS")
            set(LLGL_IOS_PLATFORM ON)
        else()
            set(LLGL_TARGET_PLATFORM "macOS")
        endif()
    elseif(UNIX)
        set(LLGL_TARGET_PLATFORM "Linux")
    endif()
endif()


# === Project ===

if(LLGL_UWP_PLATFORM)
    set(CMAKE_CXX_STANDARD 17)
else()
    set(CMAKE_CXX_STANDARD 11)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# XCODE_SCHEME_WORKING_DIRECTORY (used per target) requires CMake 3.17.5
if(APPLE AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.5")
    # Once Xcode schemes are generated for example, we also have to enable them globally or the main projects will disappear
    set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
endif()


# === Build path ===

set(OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/build)
set(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)
set(LIBRARY_OUTPUT_PATH ${OUTPUT_DIR} CACHE PATH "Build directory" FORCE)

set( EXTERNAL_MODULE_DIR            "${PROJECT_SOURCE_DIR}/cmake"                   )
set( PROJECT_INCLUDE_DIR            "${PROJECT_SOURCE_DIR}/include"                 )
set( EXTERNAL_INCLUDE_DIR           "${PROJECT_SOURCE_DIR}/external"                )
set( EXAMPLE_PROJECTS_ROOT_DIR      "${PROJECT_SOURCE_DIR}/examples"                )
set( EXAMPLE_CPP_PROJECTS_DIR       "${EXAMPLE_PROJECTS_ROOT_DIR}/Cpp"              )
set( EXAMPLE_C99_PROJECTS_DIR       "${EXAMPLE_PROJECTS_ROOT_DIR}/C99"              )
set( EXAMPLE_CSHARP_PROJECTS_DIR    "${EXAMPLE_PROJECTS_ROOT_DIR}/CSharp"           )
set( SHARED_PLATFORM_DIR            "${EXAMPLE_PROJECTS_ROOT_DIR}/Shared/Platform"  )
set( SHARED_ASSETS_DIR              "${EXAMPLE_PROJECTS_ROOT_DIR}/Shared/Assets"    )
set( TEST_PROJECTS_DIR              "${PROJECT_SOURCE_DIR}/tests"                   )
set( BACKEND_INCLUDE_DIR            "${PROJECT_INCLUDE_DIR}/LLGL/Backend"           )


# === Macros ===

if(LLGL_IOS_PLATFORM OR LLGL_ANDROID_PLATFORM OR LLGL_WASM_PLATFORM)
    set(LLGL_MOBILE_PLATFORM ON)
else()
    set(LLGL_MOBILE_PLATFORM OFF)
endif()

macro(ADD_DEFINE IDENT)
    add_definitions("-D${IDENT}")
endmacro()

macro(ADD_PROJECT_DEFINE TARGET_NAME IDENT)
    target_compile_definitions(${TARGET_NAME} PRIVATE "-D${IDENT}")
endmacro()

macro(ADD_DEBUG_DEFINE IDENT)
    # Requires CMake 3.12
    add_compile_definitions("$<$<CONFIG:Debug>:${IDENT}>")
endmacro()


# === Functions ===

# Filters sources files with a variadic argument for all directores:
# Example:
#   find_source_files(OutputFiles "*.h;*.cpp" ${MyProjectDir}/Base ${MyProjectDir}/Extensions)
function(find_source_files OUTPUT_LIST FILTERS)
    # Substitute predefined filters
    if("${FILTERS}" STREQUAL "C")
        set(FILTERS "*.c;*.h;*.inl")
    elseif("${FILTERS}" STREQUAL "CXX")
        set(FILTERS "*.c;*.cpp;*.h;*.inl")
    elseif("${FILTERS}" STREQUAL "OBJC")
        set(FILTERS "*.c;*.cpp;*.h;*.inl;*.m;*.mm")
    elseif("${FILTERS}" STREQUAL "RES")
        set(FILTERS "*.c;*.cpp;*.h;*.inl;*.m;*.mm;*.metal;*.vert;*.frag;*.tesc;*.tese")
    elseif("${FILTERS}" STREQUAL "INC")
        set(FILTERS "*.h;*.hpp;*.inl")
    endif()

    # Collect all input directories for each filter
    set(FilteredProjectDirs "")
    set(ProjectDirs "${ARGN}")
    foreach(ProjectSubdir ${ProjectDirs})
        foreach(Filter ${FILTERS})
            list(APPEND FilteredProjectDirs "${ProjectSubdir}/${Filter}")
        endforeach()
    endforeach()
    
    # Find all source files
    file(GLOB FilteredProjectFiles ${FilteredProjectDirs})
    
    # Write filtered files to output variable
    set(${OUTPUT_LIST} ${${OUTPUT_LIST}} ${FilteredProjectFiles} PARENT_SCOPE)
endfunction()

# Adds a (variadic) list of resource files to the output project files.
# The resource files must be located in either the shared assets folder,
# i.e. either under "examples/Shared/Assets/".
# Example:
#   add_project_resource_files(MyProjectFiles "Models/ModelAssets-*.obj" "Textures/TextureAsset-1.jpg")
function(add_project_resource_files OUTPUT_LIST SRC_FOLDER)
    # Iterate over all input filenames and expand their paths depending on their file extensions
    set(ResourceFiles "")
    set(InputFilenames "${ARGN}")
    foreach(InputFile ${InputFilenames})
        if("${InputFile}" MATCHES ".+\\.(png|jpg|dds|tga|obj|txt|map|icns)")
            file(GLOB SharedResourceFiles "${SHARED_ASSETS_DIR}/${InputFile}")
            file(GLOB ProjectResourceFiles "${SRC_FOLDER}/${InputFile}")
            list(APPEND ResourceFiles "${SharedResourceFiles}" "${ProjectResourceFiles}")
        endif()
    endforeach()
    
    # Write expanded resource filenames to output variable
    set(${OUTPUT_LIST} ${${OUTPUT_LIST}} ${ResourceFiles} PARENT_SCOPE)
endfunction()

# Bundles input source files together with platform specific resource files, e.g. macOS and iOS bundles.
function(bundle_project_files OUTPUT_LIST SRC_FILES)
    if(APPLE)
        if (LLGL_IOS_PLATFORM)
            find_source_files(BundledProjectFiles "AppIcon.png;LaunchScreen.storyboard" ${SHARED_PLATFORM_DIR}/iOS)
        else()
            find_source_files(BundledProjectFiles "AppIcon.icns" ${SHARED_PLATFORM_DIR}/macOS)
        endif()
        list(APPEND BundledProjectFiles "${SRC_FILES}")
    else()
        set(BundledProjectFiles "${SRC_FILES}")
    endif()
    set(${OUTPUT_LIST} ${BundledProjectFiles} PARENT_SCOPE)
endfunction()

# Main function to find and bundle project source files.
# @param SRC_FILES: can either be a directory or a list of specific source files if it sends with ".cpp", ".c", ".h", ".m", or ".mm".
function(find_project_source_files OUTPUT_LIST SRC_FILES)
    if("${SRC_FILES}" MATCHES ".+\\.(cpp|c|h|m|mm|cs)")
        set(ProjectFiles "${SRC_FILES}")
    else()
        # Find all source files for project
        if(APPLE)
            find_source_files(ProjectFiles RES ${SRC_FILES})
        else()
            find_source_files(ProjectFiles CXX ${SRC_FILES})
        endif()
    endif()
    
    # Bundle source files and write to output variable
    bundle_project_files(OutputProjectFiles "${ProjectFiles}")

    # Also bundle potential assets for certain platforms
    if(APPLE)
        # Try to find an *.assets.txt file for current project
        file(GLOB AssetsListFilename "${SRC_FILES}/*.assets.txt")
        if(NOT "${AssetsListFilename}" STREQUAL "")
            # Read *.assets.txt file and convert its content into a ';'-separated list to append project assets to bundle
            file(READ "${AssetsListFilename}" AssetsListContents)
            if(NOT "${AssetsListContents}" STREQUAL "")
                string(REGEX REPLACE ";" "\\\\;" AssetsListContents "${AssetsListContents}")
                string(REGEX REPLACE "\n" ";" AssetsListContents "${AssetsListContents}")
                add_project_resource_files(OutputProjectFiles "${SRC_FILES}" ${AssetsListContents})
            endif()
        endif()
    endif()

    # Write final project file list to output parameter
    set(${OUTPUT_LIST} ${OutputProjectFiles} PARENT_SCOPE)
endfunction()

function(set_llgl_module_properties MODULE_NAME)
    set_target_properties(
        ${MODULE_NAME} PROPERTIES
            LINKER_LANGUAGE CXX
            DEBUG_POSTFIX   "D"
            FOLDER          "LLGL"
    )
endfunction()

function(append_global_property PROP_NAME OUTPUT_LIST NEW_ITEM)
    get_property(PropertyList GLOBAL PROPERTY ${PROP_NAME})

    # Link base libs first to avoid wrong order in GCC.
    # For Clang, this order can cause duplicate linking entries, but shouldn't hurt.
    # For now it's not worth the trouble having to distinguish between compiler toolchains (see CMAKE_CXX_COMPILER_ID).
    list(APPEND PropertyList ${NEW_ITEM})

    set_property(GLOBAL PROPERTY ${PROP_NAME} "${PropertyList}")
    set(${OUTPUT_LIST} "${PropertyList}" PARENT_SCOPE)
endfunction()

function(add_llgl_module MODULE_NAME DEFINE_NAME SRC_FILES)
    # Add module as either static or shared library
    if(LLGL_BUILD_STATIC_LIB)
        add_library(${MODULE_NAME} STATIC ${SRC_FILES})
    else()
        add_library(${MODULE_NAME} SHARED ${SRC_FILES})
    endif()
    
    # Keep track of all active module names
    append_global_property(LLGL_GLOBAL_MODULE_LIST None ${MODULE_NAME})
    
    # Add basic link libraries and properties
    set_llgl_module_properties(${MODULE_NAME})
    
    ADD_PROJECT_DEFINE(LLGL ${DEFINE_NAME})
endfunction()

# Sets the working directory for startup for the specified project.
function(set_project_working_dir PROJECT_NAME WORKING_DIR)
    if(WIN32)
        # VS_DEBUGGER_WORKING_DIRECTORY requires CMake 3.8.2
        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.8.2")
            set_target_properties(
                ${PROJECT_NAME} PROPERTIES
                VS_DEBUGGER_WORKING_DIRECTORY "${WORKING_DIR}"
            )
        endif()
    elseif(APPLE)
        if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.17.5")
            # XCODE_SCHEME_WORKING_DIRECTORY requires CMake 3.17.5
            set_target_properties(
                ${PROJECT_NAME} PROPERTIES
                XCODE_GENERATE_SCHEME           TRUE
                XCODE_SCHEME_WORKING_DIRECTORY  "${WORKING_DIR}"
            )
        endif()
    endif()
endfunction()

function(add_llgl_example_project PROJECT_NAME LINKER_LANG SRC_FILES LIB_FILES)
    if(APPLE)
        # Project configurations
        add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC_FILES})
        target_link_libraries(${PROJECT_NAME} ${LIB_FILES})

        # Configure bundle name and ID
        if (${PROJECT_NAME} MATCHES "Example_+")
            string(LENGTH "Example_" NAME_PREFIX_LEN)
            string(SUBSTRING ${PROJECT_NAME} ${NAME_PREFIX_LEN} -1 PROJECT_BUNDLE_NAME)
        elseif (${PROJECT_NAME} MATCHES "Test_+")
            string(LENGTH "Test_" NAME_PREFIX_LEN)
            string(SUBSTRING ${PROJECT_NAME} ${NAME_PREFIX_LEN} -1 PROJECT_BUNDLE_NAME)
        else()
            set(PROJECT_BUNDLE_NAME ${PROJECT_NAME})
        endif()

        # macOS/iOS bundle names must not have "_" characters
        string(REPLACE "_" "-" PROJECT_BUNDLE_NAME "${PROJECT_BUNDLE_NAME}")

        # Filter resource files into "Resources" folder
        set(RES_FILES "${SRC_FILES}")
        list(FILTER RES_FILES INCLUDE REGEX ".+\\.(png|jpg|dds|tga|obj|txt|map|icns)")
        source_group("Resources" FILES ${RES_FILES})

        # Don't compile these resource types, just keep them in the project solution
        set_source_files_properties(${RES_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

        # Filter shader files into "Shaders" folder
        set(SHADER_FILES "${SRC_FILES}")
        list(FILTER SHADER_FILES INCLUDE REGEX ".+\\.(metal|vert|frag|tesc|tese)")
        source_group("Shaders" FILES ${SHADER_FILES})

        # Set app icon for macOS and iOS
        set(BundleResourceFiles "${RES_FILES};${SHADER_FILES}")
        if(LLGL_IOS_PLATFORM)
            set(BundleInfoPlistFile "${SHARED_PLATFORM_DIR}/iOS/IOSBundleInfo.plist.in")
            set(BundleIconFile "AppIcon.png")
        else()
            set(BundleInfoPlistFile "${SHARED_PLATFORM_DIR}/macOS/MacOSXBundleInfo.plist.in")
            set(BundleIconFile "AppIcon.icns")
        endif()

        # Configure Info.plist file for NSBundle
        set_target_properties(
            ${PROJECT_NAME} PROPERTIES
            OUTPUT_NAME                         "${PROJECT_NAME}"
            MACOSX_BUNDLE_BUNDLE_NAME           "${PROJECT_BUNDLE_NAME}"
            MACOSX_BUNDLE_GUI_IDENTIFIER        "LLGL.Examples.${PROJECT_BUNDLE_NAME}"
            MACOSX_BUNDLE_LONG_VERSION_STRING   "LLGL-1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING  "1.0"
            MACOSX_BUNDLE_BUNDLE_VERSION        1.0
            MACOSX_BUNDLE_ICON_FILE             "${BundleIconFile}"
            MACOSX_BUNDLE                       ON
            MACOSX_BUNDLE_INFO_PLIST            "${BundleInfoPlistFile}"
            RESOURCE                            "${BundleResourceFiles}"
        )
    elseif(LLGL_ANDROID_PLATFORM)
        # Android project configurations
        add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
        target_link_libraries(${PROJECT_NAME} ${LIB_FILES})
    else()
        add_executable(${PROJECT_NAME} ${SRC_FILES})
        target_link_libraries(${PROJECT_NAME} ${LIB_FILES})
    endif()

    # Configure linker settings
    if(LLGL_ANDROID_PLATFORM)
        # When linking the libExample*.so library for Android, we link with C++ semantics.
        # Otherwise, the libc++_shared.so library will be unintentionally included,
        # since the static dependencies like libLLGL.so must be build with C++, regardingless of the example project's language
        set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX DEBUG_POSTFIX "D")
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE ${LINKER_LANG} DEBUG_POSTFIX "D")
    endif()
    
    # Configure working directory and project solution folder
    if(${PROJECT_NAME} MATCHES "Example_C99_+")
        string(SUBSTRING ${PROJECT_NAME} 12 -1 PROJECT_BASE_NAME)
        set_project_working_dir(${PROJECT_NAME} "${EXAMPLE_C99_PROJECTS_DIR}/${PROJECT_BASE_NAME}")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples (C99)")
    elseif(${PROJECT_NAME} MATCHES "Example_CSharp_+")
        string(SUBSTRING ${PROJECT_NAME} 15 -1 PROJECT_BASE_NAME)
        set_project_working_dir(${PROJECT_NAME} "${EXAMPLE_CSHARP_PROJECTS_DIR}/${PROJECT_BASE_NAME}")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples (CSharp)")
    elseif(${PROJECT_NAME} MATCHES "Example_+")
        string(SUBSTRING ${PROJECT_NAME} 8 -1 PROJECT_BASE_NAME)
        set_project_working_dir(${PROJECT_NAME} "${EXAMPLE_CPP_PROJECTS_DIR}/${PROJECT_BASE_NAME}")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Examples")
    elseif(${PROJECT_NAME} MATCHES "Test_+")
        set_project_working_dir(${PROJECT_NAME} "${TEST_PROJECTS_DIR}")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")
    elseif(${PROJECT_NAME} MATCHES "Testbed")
        set_project_working_dir(${PROJECT_NAME} "${TEST_PROJECTS_DIR}/Testbed")
        set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tests")
    endif()
endfunction()


# === Preprocessor definitions ===

if(WIN32)
    ADD_DEFINE(_CRT_SECURE_NO_WARNINGS)
    ADD_DEFINE(_SCL_SECURE_NO_WARNINGS)
    ADD_DEFINE(UNICODE)
    ADD_DEFINE(NOMINMAX)
    ADD_DEFINE(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
    if(MSVC)
        # Disable some warnings for MSVC compiler
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd4250 /wd4251 /wd4290 /wd4103")
    endif(MSVC)
endif(WIN32)

ADD_DEBUG_DEFINE(LLGL_DEBUG)

if(LLGL_MOBILE_PLATFORM)
    ADD_DEFINE(LLGL_MOBILE_PLATFORM)
endif()


# === Initialize summary variables ===

set(SUMMARY_LIBRARY_TYPE "Unknown")
set(SUMMARY_TARGET_ARCH "Unknown")
set(SUMMARY_FLAGS "")


# === Options ===

if(APPLE)
    set(CMAKE_MACOSX_RPATH 1)
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
    set(LLGL_BUILD_64BIT 1)
else()
    set(LLGL_BUILD_64BIT 0)
endif()

option(LLGL_ENABLE_CHECKED_CAST "Enable dynamic checked cast (only in Debug mode)" ON)
option(LLGL_ENABLE_DEBUG_LAYER "Enable renderer debug layer (for both Debug and Release mode)" ON)
option(LLGL_ENABLE_EXCEPTIONS "Enable C++ exceptions" OFF)

option(LLGL_PREFER_STL_CONTAINERS "Prefers C++ STL containers over custom containers, e.g. std::vector over SmallVector<T>" OFF)

option(LLGL_BUILD_STATIC_LIB "Build LLGL as static library" OFF)
option(LLGL_BUILD_TESTS "Include test projects" OFF)
option(LLGL_BUILD_EXAMPLES "Include example projects" OFF)

option(LLGL_BUILD_RENDERER_NULL "Include Null renderer project" ON)

if(NOT LLGL_UWP_PLATFORM)
    if(EMSCRIPTEN)
        option(LLGL_BUILD_RENDERER_WEBGL "Include WebGL renderer project" ON)
        option(LLGL_ENABLE_EMSCRIPTEN_PTHREADS "Build for Wasm platform with pthreads (USE_PTHREADS). This limits browser availability!" OFF)
    elseif(LLGL_MOBILE_PLATFORM)
        option(LLGL_BUILD_RENDERER_OPENGLES3 "Include OpenGLES 3 renderer project" ON)
    else()
        option(LLGL_BUILD_RENDERER_OPENGL "Include OpenGL renderer project" ON)
    endif()
endif()

if(APPLE)
    option(LLGL_MACOS_ENABLE_COREVIDEO "Enable CoreVideo framework (for refresh rate of built-in displays)" ON)
    option(LLGL_BUILD_RENDERER_METAL "Include Metal renderer project" ON)
elseif(NOT LLGL_UWP_PLATFORM)
    option(LLGL_BUILD_RENDERER_VULKAN "Include Vulkan renderer project (experimental)" OFF)
endif()

if(WIN32)
    option(LLGL_BUILD_RENDERER_DIRECT3D11 "Include Direct3D11 renderer project" ON)
    option(LLGL_BUILD_RENDERER_DIRECT3D12 "Include Direct3D12 renderer project" ON)
endif()

option(LLGL_BUILD_WRAPPER_C99 "Include wrapper for C99" OFF)
option(LLGL_BUILD_WRAPPER_CSHARP "Include wrapper for C# (requires LLGL_BUILD_WRAPPER_C99)" OFF)

if(NOT LLGL_BUILD_WRAPPER_C99)
    if(LLGL_BUILD_WRAPPER_CSHARP)
        message(SEND_ERROR "LLGL_BUILD_WRAPPER_CSHARP is enabled but not LLGL_BUILD_WRAPPER_C99; The C# wrapper depends on the C99 wrapper!")
    endif()
endif()

if(LLGL_ENABLE_CHECKED_CAST)
    ADD_DEBUG_DEFINE(LLGL_ENABLE_CHECKED_CAST)
endif()

if(LLGL_ENABLE_DEBUG_LAYER)
    ADD_DEFINE(LLGL_ENABLE_DEBUG_LAYER)
endif()

if(LLGL_ENABLE_EXCEPTIONS)
    ADD_DEFINE(LLGL_ENABLE_EXCEPTIONS)
endif()

if(LLGL_BUILD_STATIC_LIB)
    ADD_DEFINE(LLGL_BUILD_STATIC_LIB)
endif()

if(LLGL_PREFER_STL_CONTAINERS)
    ADD_DEFINE(LLGL_PREFER_STL_CONTAINERS)
endif()

if(LLGL_MACOS_ENABLE_COREVIDEO)
    ADD_DEFINE(LLGL_MACOS_ENABLE_COREVIDEO)
endif()

if(LLGL_WASM_PLATFORM)
    set(SUMMARY_TARGET_ARCH "wasm")
elseif(LLGL_MOBILE_PLATFORM)
    if("${ANDROID_ABI}" STREQUAL "x86_64")
        set(ARCH_AMD64 ON)
        set(SUMMARY_TARGET_ARCH "x86-64")
    elseif("${ANDROID_ABI}" STREQUAL "x86")
        set(ARCH_IA32 ON)
        set(SUMMARY_TARGET_ARCH "x86")
    else()
        set(ARCH_ARM64 ON)
        set(SUMMARY_TARGET_ARCH "arm64")
    endif()
elseif(APPLE OR LLGL_BUILD_64BIT)
    set(ARCH_AMD64 ON)
    set(SUMMARY_TARGET_ARCH "x86-64")
else()
    set(ARCH_IA32 ON)
    set(SUMMARY_TARGET_ARCH "x86")
endif()

if(EMSCRIPTEN)
    # When USE_PTHREADS is enabled, HTML5 pages cannot be opened in Chrome unless launched with '--enable-features=SharedArrayBuffer'
    if(LLGL_ENABLE_EMSCRIPTEN_PTHREADS)
        add_compile_options("SHELL:-s USE_PTHREADS")
        add_link_options("SHELL:-s USE_PTHREADS")
        set(SUMMARY_FLAGS ${SUMMARY_FLAGS} "pthreads")
    endif()

    # TODO: Emscripten file system pulls in a large amount of code. Consider limiting file system support to examples and tests.
    add_link_options("SHELL:-s FORCE_FILESYSTEM") # for examples

    # LLGL needs at least WebGL 2.0
    add_link_options("SHELL:-s MIN_WEBGL_VERSION=2")
    add_link_options("SHELL:-s MAX_WEBGL_VERSION=2")
    add_link_options("SHELL:-s OFFSCREEN_FRAMEBUFFER") # required for explicit swap control
    add_link_options("SHELL:-s ASSERTIONS")
endif()


# === Global files ===

set(FilesMsvcNatvis ${PROJECT_SOURCE_DIR}/LLGL.natvis)

# Common files
find_source_files(FilesInclude                      "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL")
find_source_files(FilesIncludeBackend               "*.inl" "${PROJECT_INCLUDE_DIR}/LLGL/Backend")
find_source_files(FilesIncludeContainer             "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL/Container")
find_source_files(FilesIncludeUtils                 "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL/Utils")
find_source_files(FilesIncludePlatformBase          "*.h"   "${PROJECT_INCLUDE_DIR}/LLGL/Platform")
find_source_files(FilesCore                         CXX     "${PROJECT_SOURCE_DIR}/sources/Core")
find_source_files(FilesPlatformBase                 CXX     "${PROJECT_SOURCE_DIR}/sources/Platform")
find_source_files(FilesRenderer                     CXX     "${PROJECT_SOURCE_DIR}/sources/Renderer")

if(LLGL_ENABLE_DEBUG_LAYER)
    find_source_files(FilesRendererDbg              CXX     "${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer")
    find_source_files(FilesRendererDbgBuffer        CXX     "${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/Buffer")
    find_source_files(FilesRendererDbgRenderState   CXX     "${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/RenderState")
    find_source_files(FilesRendererDbgShader        CXX     "${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/Shader")
    find_source_files(FilesRendererDbgTexture       CXX     "${PROJECT_SOURCE_DIR}/sources/Renderer/DebugLayer/Texture")
endif()

if(EMSCRIPTEN)
    find_source_files(FilesPlatform                 CXX     "${PROJECT_SOURCE_DIR}/sources/Platform/Wasm")
    find_source_files(FilesIncludePlatform          CXX     "${PROJECT_INCLUDE_DIR}/LLGL/Platform/Wasm")
elseif(WIN32)
    if(LLGL_UWP_PLATFORM)
        find_source_files(FilesPlatform             CXX     "${PROJECT_SOURCE_DIR}/sources/Platform/UWP")
        find_source_files(FilesIncludePlatform      CXX     "${PROJECT_INCLUDE_DIR}/LLGL/Platform/UWP")
    else()
        find_source_files(FilesPlatform             CXX     "${PROJECT_SOURCE_DIR}/sources/Platform/Win32")
        find_source_files(FilesIncludePlatform      CXX     "${PROJECT_INCLUDE_DIR}/LLGL/Platform/Win32")
    endif()
else()
    set(PlatformPosixDir "${PROJECT_SOURCE_DIR}/sources/Platform/POSIX")
    if(APPLE)
        if(LLGL_MOBILE_PLATFORM)
            find_source_files(FilesPlatform         OBJC    "${PROJECT_SOURCE_DIR}/sources/Platform/IOS" "${PlatformPosixDir}")
            find_source_files(FilesIncludePlatform  OBJC    "${PROJECT_INCLUDE_DIR}/LLGL/Platform/IOS")
        else()
            find_source_files(FilesPlatform         OBJC    "${PROJECT_SOURCE_DIR}/sources/Platform/MacOS" "${PlatformPosixDir}")
            find_source_files(FilesIncludePlatform  OBJC    "${PROJECT_INCLUDE_DIR}/LLGL/Platform/MacOS")
        endif()
    elseif(UNIX)
        if(LLGL_ANDROID_PLATFORM)
            find_source_files(FilesPlatform         CXX     "${PROJECT_SOURCE_DIR}/sources/Platform/Android")
            find_source_files(FilesIncludePlatform  CXX     "${PROJECT_INCLUDE_DIR}/LLGL/Platform/Android")
        else()
            find_source_files(FilesPlatform         CXX     "${PROJECT_SOURCE_DIR}/sources/Platform/Linux" "${PlatformPosixDir}")
            find_source_files(FilesIncludePlatform  CXX     "${PROJECT_INCLUDE_DIR}/LLGL/Platform/Linux")
        endif()
    endif()
endif()

if(LLGL_ANDROID_PLATFORM)
    set(ANDROID_APP_GLUE_DIR "$ENV{ANDROID_NDK_ROOT}/sources/android/native_app_glue")
    set(
        FilesAndroidNativeAppGlue
        "${ANDROID_APP_GLUE_DIR}/android_native_app_glue.c"
        "${ANDROID_APP_GLUE_DIR}/android_native_app_glue.h"
    )
endif()


# === Source group folders ===

source_group("NatVis" FILES ${FilesMsvcNatvis})

source_group("Include"              FILES ${FilesInclude})
source_group("Include\\Backend"     FILES ${FilesIncludeBackend})
source_group("Include\\Container"   FILES ${FilesIncludeContainer})
source_group("Include\\Utils"       FILES ${FilesIncludeUtils})
source_group("Include\\Platform"    FILES ${FilesIncludePlatformBase} ${FilesIncludePlatform})

source_group("Sources\\Core"        FILES ${FilesCore})
source_group("Sources\\Platform"    FILES ${FilesPlatformBase} ${FilesPlatform})
source_group("Sources\\Renderer"    FILES ${FilesRenderer})

if(LLGL_ENABLE_DEBUG_LAYER)
    source_group("Sources\\Renderer\\DebugLayer"                FILES ${FilesRendererDbg})
    source_group("Sources\\Renderer\\DebugLayer\\Buffer"        FILES ${FilesRendererDbgBuffer})
    source_group("Sources\\Renderer\\DebugLayer\\RenderState"   FILES ${FilesRendererDbgRenderState})
    source_group("Sources\\Renderer\\DebugLayer\\Shader"        FILES ${FilesRendererDbgShader})
    source_group("Sources\\Renderer\\DebugLayer\\Texture"       FILES ${FilesRendererDbgTexture})
endif()

if(LLGL_ANDROID_PLATFORM)
    source_group("native_app_glue" FILES ${FilesAndroidNativeAppGlue})
endif()


# === Include directories ===

include_directories("${PROJECT_INCLUDE_DIR}")

include("${EXTERNAL_MODULE_DIR}/FindGaussianLib.cmake")

if(LLGL_ANDROID_PLATFORM)
    include_directories(${ANDROID_APP_GLUE_DIR})
endif(LLGL_ANDROID_PLATFORM)


# === Projects ===

set(
    FilesLLGL
    ${FilesInclude}
    ${FilesIncludeBackend}
    ${FilesIncludeContainer}
    ${FilesIncludeUtils}
    ${FilesIncludePlatformBase}
    ${FilesIncludePlatform}
    ${FilesCore}
    ${FilesPlatformBase}
    ${FilesPlatform}
    ${FilesRenderer}
)

if(MSVC)
    list(APPEND FilesLLGL ${FilesMsvcNatvis})
endif()

if(LLGL_ENABLE_DEBUG_LAYER)
    list(
        APPEND FilesLLGL
        ${FilesRendererDbg}
        ${FilesRendererDbgBuffer}
        ${FilesRendererDbgRenderState}
        ${FilesRendererDbgShader}
        ${FilesRendererDbgTexture}
    )
endif()

# Wrapper: C99
if(LLGL_BUILD_WRAPPER_C99)
    find_source_files(FilesWrapperC99 CXX "${PROJECT_SOURCE_DIR}/wrapper/C99")
    find_source_files(FilesIncludeC99 "*.h" "${PROJECT_INCLUDE_DIR}/LLGL-C")
    source_group("Wrapper\\C99\\Include" FILES ${FilesIncludeC99})
    source_group("Wrapper\\C99\\Sources" FILES ${FilesWrapperC99})
    list(APPEND FilesLLGL ${FilesIncludeC99} ${FilesWrapperC99})
endif()

# Base project
if(LLGL_BUILD_STATIC_LIB)
    set(SUMMARY_LIBRARY_TYPE "Static")
    add_library(LLGL STATIC ${FilesLLGL})
else()
    set(SUMMARY_LIBRARY_TYPE "Shared")
    add_library(LLGL SHARED ${FilesLLGL})
endif()

include(GNUInstallDirs)
target_include_directories(LLGL INTERFACE
                           $<BUILD_INTERFACE:${PROJECT_INCLUDE_DIR}>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(LLGL_ANDROID_PLATFORM)
    add_library(android_native_app_glue STATIC ${FilesAndroidNativeAppGlue})
    set_target_properties(android_native_app_glue PROPERTIES LINKER_LANGUAGE C)
    target_link_libraries(LLGL android log android_native_app_glue)
elseif(LLGL_IOS_PLATFORM)
    target_link_libraries(LLGL PUBLIC "-framework Foundation -framework UIKit")
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    target_link_libraries(LLGL ${COCOA_LIBRARY})
    if(LLGL_MACOS_ENABLE_COREVIDEO)
        target_link_libraries(LLGL "-framework CoreVideo")
    endif()
elseif(EMSCRIPTEN)
    target_link_libraries(LLGL embind)
elseif(UNIX)
    target_link_libraries(LLGL X11 pthread Xrandr)
#elseif(LLGL_UWP_PLATFORM)
#    set_target_properties(LLGL PROPERTIES VS_WINRT_REFERENCES "Windows.Foundation.UniversalApiContract")
endif()

set_property(GLOBAL PROPERTY LLGL_GLOBAL_MODULE_LIST LLGL)

set_llgl_module_properties(LLGL)

if(LLGL_ANDROID_PLATFORM)
    append_global_property(LLGL_GLOBAL_MODULE_LIST None android_native_app_glue)
endif()

if(LLGL_BUILD_RENDERER_NULL)
    add_subdirectory(sources/Renderer/Null)
endif()

if(LLGL_BUILD_RENDERER_OPENGL OR LLGL_BUILD_RENDERER_OPENGLES3 OR LLGL_BUILD_RENDERER_WEBGL)
    add_subdirectory(sources/Renderer/OpenGL)
endif()

if(LLGL_BUILD_RENDERER_VULKAN)
    add_subdirectory(sources/Renderer/Vulkan)
endif()

if(LLGL_BUILD_RENDERER_METAL)
    add_subdirectory(sources/Renderer/Metal)
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D11 OR LLGL_BUILD_RENDERER_DIRECT3D12)
    add_subdirectory(sources/Renderer/DXCommon)
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D11)
    add_subdirectory(sources/Renderer/Direct3D11)
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D12)
    add_subdirectory(sources/Renderer/Direct3D12)
endif()

# Static libs must all be linked to the final apps (LLGL_BUILD_STATIC_LIB).
# Also UWP apps need references to all loaded modules (LLGL_UWP_PLATFORM).
if(LLGL_BUILD_STATIC_LIB OR LLGL_UWP_PLATFORM)
    get_property(LLGL_MODULE_LIBS GLOBAL PROPERTY LLGL_GLOBAL_MODULE_LIST)
else()
    set(LLGL_MODULE_LIBS LLGL)
endif()

if(GaussLib_INCLUDE_DIR)
    if(LLGL_BUILD_TESTS AND NOT LLGL_MOBILE_PLATFORM)
        add_subdirectory(tests)
    endif()
    if(LLGL_BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif(LLGL_BUILD_EXAMPLES)
else(GaussLib_INCLUDE_DIR)
    # If GaussianLib is not available, tests and examples cannot be enabled
    if(LLGL_BUILD_TESTS)
        message(SEND_ERROR "LLGL_BUILD_TESTS is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
    if(LLGL_BUILD_EXAMPLES)
        message(SEND_ERROR "LLGL_BUILD_EXAMPLES is enabled but 'GaussLib_INCLUDE_DIR' path is missing")
    endif()
endif(GaussLib_INCLUDE_DIR)

# Wrapper: C#
if(LLGL_BUILD_WRAPPER_CSHARP)
    add_subdirectory(wrapper/CSharp)
endif()

# Install targets, headers, and CMake config files
get_property(LLGL_ALL_TARGETS GLOBAL PROPERTY LLGL_GLOBAL_MODULE_LIST)
if(LLGL_BUILD_RENDERER_DIRECT3D11 OR LLGL_BUILD_RENDERER_DIRECT3D12)
    list(APPEND LLGL_ALL_TARGETS LLGL_DXCommon)
endif()

install(TARGETS ${LLGL_ALL_TARGETS} EXPORT LLGLTargets RUNTIME LIBRARY ARCHIVE)
install(DIRECTORY "${PROJECT_INCLUDE_DIR}/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(EXPORT LLGLTargets NAMESPACE LLGL:: DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/LLGL")
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/LLGLConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/LLGL
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/LLGLConfig.cmake" DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/LLGL")

# Summary Information
message(STATUS "~~~ Build Summary ~~~")

message(STATUS "Target Platform: ${LLGL_TARGET_PLATFORM}")
message(STATUS "Target Architecture: ${SUMMARY_TARGET_ARCH}")
message(STATUS "Target Library: ${SUMMARY_LIBRARY_TYPE}")

if(LLGL_BUILD_RENDERER_NULL)
    message(STATUS "Build Renderer: Null")
endif()

if(LLGL_BUILD_RENDERER_OPENGL)
    message(STATUS "Build Renderer: OpenGL")
endif()

if(LLGL_BUILD_RENDERER_OPENGLES3)
    message(STATUS "Build Renderer: ${LLGL_GL_ENABLE_OPENGLES}")
endif()

if(LLGL_BUILD_RENDERER_WEBGL)
    message(STATUS "Build Renderer: WebGL")
endif()

if(LLGL_BUILD_RENDERER_VULKAN)
    message(STATUS "Build Renderer: Vulkan")
endif()

if(LLGL_BUILD_RENDERER_METAL)
    message(STATUS "Build Renderer: Metal")
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D11)
    message(STATUS "Build Renderer: ${LLGL_D3D11_ENABLE_FEATURELEVEL}")
endif()

if(LLGL_BUILD_RENDERER_DIRECT3D12)
    message(STATUS "Build Renderer: ${LLGL_D3D12_ENABLE_FEATURELEVEL}")
endif()

if(LLGL_BUILD_WRAPPER_C99)
    message(STATUS "Build Wrapper: C99")
endif()

if(LLGL_BUILD_WRAPPER_CSHARP)
    message(STATUS "Build Wrapper: C#")
endif()

if(LLGL_BUILD_EXAMPLES)
    message(STATUS "Build Examples")
endif()

if(LLGL_BUILD_TESTS)
    message(STATUS "Build Tests")
endif()

if(LLGL_VK_ENABLE_SPIRV_REFLECT)
    message(STATUS "Including Submodule: SPIRV-Headers")
endif()

if(NOT "${SUMMARY_FLAGS}" STREQUAL "")
    message(STATUS "Options: ${SUMMARY_FLAGS}")
endif()

message(STATUS "~~~~~~~~~~~~~~~~~~~~~")

