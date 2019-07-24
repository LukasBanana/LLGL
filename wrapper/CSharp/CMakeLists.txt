
# CMake lists for C# wrapper

# === Preprocessor definitions ===

# Disable "/EHsc" flag, that is incompatible with "/CLR"
#list(REMOVE_ITEM CMAKE_CXX_FLAGS "/EHsc")
set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS /W3 /GR /wd4251")

# Disable "/RTC1" flag, that is incompatible with "/CLR"
#list(REMOVE_ITEM CMAKE_CXX_FLAGS_DEBUG "/RTC1")
set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MDd /Zi /Ob0 /Od /wd4251")


# === Global files ===

file(GLOB FilesWrapperCSharp ${PROJECT_SOURCE_DIR}/wrapper/CSharp/*.*)

source_group("Wrapper\\CSharp" FILES ${FilesWrapperCSharp})


# === Executable ===

# Wrapper library
add_library(LLGL_CSharp SHARED ${FilesWrapperCSharp})
set_target_properties(LLGL_CSharp PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(LLGL_CSharp PROPERTIES COMPILE_FLAGS "/CLR")
set_target_properties(LLGL_CSharp PROPERTIES OUTPUT_NAME "SharpLLGL")
target_link_libraries(LLGL_CSharp ${LLGL_DEPENDENCIES})
target_compile_features(LLGL_CSharp PRIVATE cxx_range_for)

