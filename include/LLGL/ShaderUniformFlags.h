/*
 * ShaderUniformFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_UNIFORM_FLAGS_H
#define LLGL_SHADER_UNIFORM_FLAGS_H


#include <string>
#include <cstdint>


namespace LLGL
{


/**
\brief Shader uniform type enumeration.
\remarks Because "Bool" is a reserved identifier for an Xlib macro on GNU/Linux,
all scalar types also have a component index (e.g. "Bool1" instead of "Bool").
*/
enum class UniformType
{
    Undefined,      //!< Undefined uniform type.

    /* ----- Scalars & Vectors ----- */
    Float1,         //!< float uniform.
    Float2,         //!< float2/ vec2 uniform.
    Float3,         //!< float3/ vec3 uniform.
    Float4,         //!< float4/ vec4 uniform.
    Double1,        //!< double uniform.
    Double2,        //!< double2/ dvec2 uniform.
    Double3,        //!< double3/ dvec3 uniform.
    Double4,        //!< double4/ dvec4 uniform.
    Int1,           //!< int uniform.
    Int2,           //!< int2/ ivec2 uniform.
    Int3,           //!< int3/ ivec3 uniform.
    Int4,           //!< int4/ ivec4 uniform.
    UInt1,          //!< uint uniform.
    UInt2,          //!< uint2/ uvec2 uniform.
    UInt3,          //!< uint3/ uvec3 uniform.
    UInt4,          //!< uint4/ uvec4 uniform.
    Bool1,          //!< bool uniform.
    Bool2,          //!< bool2/ bvec2 uniform.
    Bool3,          //!< bool3/ bvec3 uniform.
    Bool4,          //!< bool4/ bvec4 uniform.

    /* ----- Matrices ----- */
    Float2x2,       //!< float2x2/ mat2 uniform.
    Float3x3,       //!< float3x3/ mat3 uniform.
    Float4x4,       //!< float4x4/ mat4 uniform.
    Float2x3,       //!< float2x3/ mat2x3 uniform.
    Float2x4,       //!< float2x4/ mat2x4 uniform.
    Float3x2,       //!< float3x2/ mat3x2 uniform.
    Float3x4,       //!< float3x4/ mat3x4 uniform.
    Float4x2,       //!< float4x2/ mat4x2 uniform.
    Float4x3,       //!< float4x3/ mat4x3 uniform.
    Double2x2,      //!< double2x2/ dmat2 uniform.
    Double3x3,      //!< double3x3/ dmat3 uniform.
    Double4x4,      //!< double4x4/ dmat4 uniform.
    Double2x3,      //!< double2x3/ dmat2x3 uniform.
    Double2x4,      //!< double2x4/ dmat2x4 uniform.
    Double3x2,      //!< double3x2/ dmat3x2 uniform.
    Double3x4,      //!< double3x4/ dmat3x4 uniform.
    Double4x2,      //!< double4x2/ dmat4x2 uniform.
    Double4x3,      //!< double4x3/ dmat4x3 uniform.

    /* ----- Resources ----- */
    Sampler,        //!< Sampler uniform (e.g. "sampler2D").
    Image,          //!< Image uniform (e.g. "image2D").
    AtomicCounter,  //!< Atomic counter uniform (e.g. "atomic_uint").
};


//! Shader uniform location type, as zero-based index in 32-bit signed integer format.
using UniformLocation = std::int32_t;

/**
\brief Shader uniform descriptor structure.
\see ShaderReflectionDescriptor::uniforms
*/
struct UniformDescriptor
{
    //! Name of the uniform inside the shader.
    std::string     name;

    //! Data type of the uniform. By default UniformType::Undefined.
    UniformType     type        = UniformType::Undefined;

    //! Internal location of the uniform within a shader program.
    UniformLocation location    = 0;

    //! Array size of the uniform.
    std::uint32_t   size        = 0;
};


} // /namespace LLGL


#endif



// ================================================================================
