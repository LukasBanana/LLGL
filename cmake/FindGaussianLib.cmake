
# Custom CMake module for finding "GaussianLib" files
# Written by Lukas Hermanns on 24/08/2015

# === Options ===

option(GaussLib_REAL_DOUBLE "Use double precision floating-points for real types" OFF)
option(GaussLib_ASSERT_EXCEPTION "Enable runtime exceptions for assertions" OFF)
option(GaussLib_ENABLE_SWIZZLE_OPERATOR "Enable swizzle operator for Vector class" OFF)
option(GaussLib_ENABLE_INVERSE_OPERATOR "Enable inverse matrix operator (A ^ -1)" OFF)
option(GaussLib_DISABLE_AUTO_INIT "Disable automatic initialization" OFF)
option(GaussLib_ROW_MAJOR_STORAGE "Use row-major storage (column-major storage otherwise)" OFF)
option(GaussLib_ROW_VECTORS "Use row-vectors (column-vectors otherwise)" OFF)


# === Macros ===

if(GaussLib_REAL_DOUBLE)
	add_definitions(-DGS_REAL_DOUBLE)
endif()

if(GaussLib_ASSERT_EXCEPTION)
	add_definitions(-DGS_ASSERT_EXCEPTION)
endif()

if(GaussLib_ENABLE_SWIZZLE_OPERATOR)
	add_definitions(-DGS_ENABLE_SWIZZLE_OPERATOR)
endif()

if(GaussLib_ENABLE_INVERSE_OPERATOR)
	add_definitions(-DGS_ENABLE_INVERSE_OPERATOR)
endif()

if(GaussLib_DISABLE_AUTO_INIT)
	add_definitions(-DGS_DISABLE_AUTO_INIT)
endif()

if(GaussLib_ROW_MAJOR_STORAGE)
	add_definitions(-DGS_ROW_MAJOR_STORAGE)
endif()

if(GaussLib_ROW_VECTORS)
	add_definitions(-DGS_ROW_VECTORS)
endif()


# === Find library ===

find_path(GaussLib_INCLUDE_DIR Gauss/Gauss.h)

if(GaussLib_INCLUDE_DIR)
    include_directories(${GaussLib_INCLUDE_DIR})
endif()

