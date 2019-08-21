/*
 * ShaderProgramFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SHADER_PROGRAM_FLAGS_H
#define LLGL_SHADER_PROGRAM_FLAGS_H


#include "ForwardDecls.h"
#include "VertexFormat.h"
#include "StreamOutputFormat.h"
#include "ResourceFlags.h"
#include "BufferFlags.h"
#include "PipelineLayoutFlags.h"
#include <vector>
#include <string>
#include <cstdint>


namespace LLGL
{


//! Shader uniform location type, as zero-based index in 32-bit signed integer format.
using UniformLocation = std::int32_t;


/* ----- Enumerations ----- */

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
    Float2x3,       //!< float2x3/ mat2x3 uniform.
    Float2x4,       //!< float2x4/ mat2x4 uniform.
    Float3x2,       //!< float3x2/ mat3x2 uniform.
    Float3x3,       //!< float3x3/ mat3 uniform.
    Float3x4,       //!< float3x4/ mat3x4 uniform.
    Float4x2,       //!< float4x2/ mat4x2 uniform.
    Float4x3,       //!< float4x3/ mat4x3 uniform.
    Float4x4,       //!< float4x4/ mat4 uniform.
    Double2x2,      //!< double2x2/ dmat2 uniform.
    Double2x3,      //!< double2x3/ dmat2x3 uniform.
    Double2x4,      //!< double2x4/ dmat2x4 uniform.
    Double3x2,      //!< double3x2/ dmat3x2 uniform.
    Double3x3,      //!< double3x3/ dmat3 uniform.
    Double3x4,      //!< double3x4/ dmat3x4 uniform.
    Double4x2,      //!< double4x2/ dmat4x2 uniform.
    Double4x3,      //!< double4x3/ dmat4x3 uniform.
    Double4x4,      //!< double4x4/ dmat4 uniform.

    /* ----- Resources ----- */
    Sampler,        //!< Sampler uniform (e.g. "sampler2D").
    Image,          //!< Image uniform (e.g. "image2D").
    AtomicCounter,  //!< Atomic counter uniform (e.g. "atomic_uint").
};


/* ----- Structures ----- */

/**
\brief Descriptor structure for shader programs.
\see RenderSystem::CreateShaderProgram
\see RenderSystem::CreateShader
*/
struct ShaderProgramDescriptor
{
    /**
    \brief Vertex format list. This may also be empty, if the vertex shader has no input attributes or only a compute shader is specified.
    \see VertexFormat
    */
    std::vector<VertexFormat>   vertexFormats;

    //TODO: add FragmentFormat structure to provide CPU side fragment binding for OpenGL
    #if 0
    FragmentFormat              fragmentFormat;
    #endif

    /**
    \brief Specifies the vertex shader.
    \remarks Each graphics shader program must have at least a vertex shader.
    For a compute shader program, only a compute shader must be specified.
    With OpenGL, this shader may also have a stream output.
    \see ShaderDescriptor::streamOutput
    */
    Shader*                     vertexShader            = nullptr;

    /**
    \brief Specifies the tessellation-control shader (also referred to as "Hull Shader").
    \remarks If this is used, the counter part must also be specified (i.e. \c tessEvaluationShader).
    \see tessEvaluationShader
    */
    Shader*                     tessControlShader       = nullptr;

    /**
    \brief Specifies the tessellation-evaluation shader (also referred to as "Domain Shader").
    \remarks If this is used, the counter part must also be specified (i.e. \c tessControlShader).
    \see tessControlShader
    */
    Shader*                     tessEvaluationShader    = nullptr;

    /**
    \brief Specifies an optional geometry shader.
    \remarks This shader may also have a stream output.
    \see ShaderDescriptor::streamOutput
    */
    Shader*                     geometryShader          = nullptr;

    /**
    \brief Specifies an optional fragment shader (also referred to as "Pixel Shader").
    \remarks If no fragment shader is specified, generated fragments are discarded by the output merger
    and only the stream-output functionality is used by either the vertex or geometry shader.
    */
    Shader*                     fragmentShader          = nullptr;

    /**
    \brief Specifies the compute shader.
    \remarks This shader cannot be used in conjunction with any other shaders.
    */
    Shader*                     computeShader           = nullptr;
};

/**
\brief Shader reflection resource structure.
\see ShaderReflection::resources
\see BindingDescriptor
*/
struct ShaderResource
{
    /**
    \brief Binding descriptor with resource name, binding slot, flags, and array size.
    \remarks Although the \c name attribute in the BindingDescriptor structure is optional for pipeline layouts,
    the shader reflection always queries this attribute as well.
    */
    BindingDescriptor   binding;

    /**
    \brief Specifies the size (in bytes) for a constant buffer resource.
    \remarks Additional attribute exclusively used for constant buffer resources.
    For all other resources, i.e. when 'type' is not equal to 'ResourceType::ConstantBuffer', this attribute is zero.
    \see ResourceType::ConstantBuffer
    */
    std::uint32_t       constantBufferSize  = 0;

    /**
    \brief Specifies the sub-type of a storage buffer resource.
    \remarks Additional attribute exclusively used for storage buffer resources.
    */
    StorageBufferType   storageBufferType   = StorageBufferType::Undefined;
};

/**
\brief Shader reflection uniform structure.
\see ShaderReflection::uniforms
*/
struct ShaderUniform
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

/**
\brief Shader reflection structure.
\remarks Contains all information of resources and attributes that can be queried from a shader program.
This is not a "descriptor", because it is only used as output from an interface rather than a description to create something.
\see ShaderProgram::QueryReflection
*/
struct ShaderReflection
{
    //! List of all vertex input attributes.
    std::vector<VertexAttribute>        vertexAttributes;

    //! List of all stream-output attributes.
    std::vector<StreamOutputAttribute>  streamOutputAttributes;

    #if 0//TODO: enable this
    //! List of all fragment output attributes.
    std::vector<FragmentAttribute>      fragmentAttributes;
    #endif

    //! List of all shader reflection resource views.
    std::vector<ShaderResource>         resources;

    /**
    \brief List of all uniforms (a.k.a. shader constants).
    \note Only supported with: OpenGL, Vulkan.
    */
    std::vector<ShaderUniform>          uniforms;
};


} // /namespace LLGL


#endif



// ================================================================================
