/*
 * Format.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_FORMAT_H
#define LLGL_FORMAT_H


#include "Export.h"


namespace LLGL
{


/* ----- Enumerations ----- */

//! Renderer data types enumeration.
enum class DataType
{
    Int8,   //!< 8-bit signed integer (char).
    UInt8,  //!< 8-bit unsigned integer (unsigned char).

    Int16,  //!< 16-bit signed integer (short).
    UInt16, //!< 16-bit unsigned integer (unsigned short).

    Int32,  //!< 32-bit signed integer (int).
    UInt32, //!< 32-bit unsigned integer (unsiged int).
    
    Float,  //!< 32-bit floating-point (float).
    Double, //!< 64-bit real type (double).
};

//! Renderer vector types enumeration.
enum class VectorType
{
    Float,      //!< 1-Dimensional single precision floating-point vector (float in GLSL, float in HLSL).
    Float2,     //!< 2-Dimensional single precision floating-point vector (vec2 in GLSL, float2 in HLSL).
    Float3,     //!< 3-Dimensional single precision floating-point vector (vec3 in GLSL, float3 in HLSL).
    Float4,     //!< 4-Dimensional single precision floating-point vector (vec4 in GLSL, float4 in HLSL).
    Double,     //!< 1-Dimensional double precision floating-point vector (double in GLSL, double in HLSL).
    Double2,    //!< 2-Dimensional double precision floating-point vector (dvec2 in GLSL, double2 in HLSL).
    Double3,    //!< 3-Dimensional double precision floating-point vector (dvec3 in GLSL, double3 in HLSL).
    Double4,    //!< 4-Dimensional double precision floating-point vector (dvec4 in GLSL, double4 in HLSL).
    Int,        //!< 1-Dimensional signed integer vector (int in GLSL, int in HLSL).
    Int2,       //!< 2-Dimensional signed integer vector (ivec2 in GLSL, int2 in HLSL).
    Int3,       //!< 3-Dimensional signed integer vector (ivec3 in GLSL, int3 in HLSL).
    Int4,       //!< 4-Dimensional signed integer vector (ivec4 in GLSL, int4 in HLSL).
    UInt,       //!< 1-Dimensional unsigned integer vector (uint in GLSL, uint in HLSL).
    UInt2,      //!< 2-Dimensional unsigned integer vector (uvec2 in GLSL, uint2 in HLSL).
    UInt3,      //!< 3-Dimensional unsigned integer vector (uvec3 in GLSL, uint3 in HLSL).
    UInt4,      //!< 4-Dimensional unsigned integer vector (uvec4 in GLSL, uint4 in HLSL).
};

/*
//! Renderer matrix types enumeration.
enum class MatrixType
{
    Float2x2,   //!< 2x2 single precision floating-point matrix (mat2 in GLSL, float2x2 in HLSL).
    Float3x3,   //!< 3x3 single precision floating-point matrix (mat3 in GLSL, float3x3 in HLSL).
    Float4x4,   //!< 4x4 single precision floating-point matrix (mat4 in GLSL, float4x4 in HLSL).
    Float2x3,   //!< 2x3 single precision floating-point matrix (mat2x3 in GLSL, float2x3 in HLSL).
    Float2x4,   //!< 2x4 single precision floating-point matrix (mat2x4 in GLSL, float2x4 in HLSL).
    Float3x2,   //!< 3x2 single precision floating-point matrix (mat3x2 in GLSL, float3x2 in HLSL).
    Float3x4,   //!< 3x4 single precision floating-point matrix (mat3x4 in GLSL, float3x4 in HLSL).
    Float4x2,   //!< 4x2 single precision floating-point matrix (mat4x2 in GLSL, float4x2 in HLSL).
    Float4x3,   //!< 4x3 single precision floating-point matrix (mat4x3 in GLSL, float4x3 in HLSL).

    Double2x2,  //!< 2x2 double precision floating-point matrix (dmat2 in GLSL, double2x2 in HLSL).
    Double3x3,  //!< 3x3 double precision floating-point matrix (dmat3 in GLSL, double3x3 in HLSL).
    Double4x4,  //!< 4x4 double precision floating-point matrix (dmat4 in GLSL, double4x4 in HLSL).
    Double2x3,  //!< 2x3 double precision floating-point matrix (dmat2x3 in GLSL, double2x3 in HLSL).
    Double2x4,  //!< 2x4 double precision floating-point matrix (dmat2x4 in GLSL, double2x4 in HLSL).
    Double3x2,  //!< 3x2 double precision floating-point matrix (dmat3x2 in GLSL, double3x2 in HLSL).
    Double3x4,  //!< 3x4 double precision floating-point matrix (dmat3x4 in GLSL, double3x4 in HLSL).
    Double4x2,  //!< 4x2 double precision floating-point matrix (dmat4x2 in GLSL, double4x2 in HLSL).
    Double4x3,  //!< 4x3 double precision floating-point matrix (dmat4x3 in GLSL, double4x3 in HLSL).
};*/


/* ----- Functions ----- */

//! Returns the size (in bytes) of the specified data type.
LLGL_EXPORT unsigned int DataTypeSize(const DataType dataType);

//! Returns the size (in bytes) of the specified vector type.
LLGL_EXPORT unsigned int VectorTypeSize(const VectorType vectorType);

/**
\brief Retrieves the format of the specified vector type.
\param[in] vectorType Specifies the vector type whose format is to be retrieved.
\param[out] dataType Specifies the output parameter for the resulting data type.
\param[out] components Specifiefs the output parameter for the resulting number of vector components.
*/
LLGL_EXPORT void VectorTypeFormat(const VectorType vectorType, DataType& dataType, unsigned int& components);


} // /namespace LLGL


#endif



// ================================================================================
