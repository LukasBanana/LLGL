/*
 * ShaderReflectionFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SHADER_REFLECTION_H
#define LLGL_SHADER_REFLECTION_H


#include <LLGL/ForwardDecls.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/ShaderFlags.h>
#include <vector>
#include <string>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Storage buffer type enumeration for shader reflection.
\note Only supported with: Direct3D 11, Direct3D 12.
\see ShaderResourceReflection::storageBufferType
*/
enum class StorageBufferType
{
    //! Undefined storage buffer type.
    Undefined,

    /**
    \brief Typed buffer, e.g. \c Buffer<float4> in HLSL.
    \see BufferDescriptor::format
    */
    TypedBuffer,

    /**
    \brief Structured buffer, e.g. \c StructuredBuffer<MyStruct> in HLSL.
    \see BufferDescriptor::stride
    */
    StructuredBuffer,

    /**
    \brief Byte-address buffer, e.g. \c ByteAddressBuffer in HLSL.
    \see BufferDescriptor::format
    */
    ByteAddressBuffer,

    /**
    \brief Typed read/write buffer, e.g. \c RWBuffer<float4> in HLSL.
    \see BufferDescriptor::format
    */
    RWTypedBuffer,

    /**
    \brief Read/write structured buffer, e.g. \c RWStructuredBuffer<MyStruct> in HLSL.
    \see BufferDescriptor::stride
    */
    RWStructuredBuffer,

    /**
    \brief Read/write byte-address buffer, e.g. \c RWByteAddressBuffer in HLSL.
    \see BufferDescriptor::stride
    */
    RWByteAddressBuffer,

    /**
    \brief Append structured buffer, e.g. \c AppendStructuredBuffer<MyStruct> in HLSL.
    \see BufferDescriptor::stride
    \see MiscFlags::Append
    */
    AppendStructuredBuffer,

    /**
    \brief Consume structured buffer, e.g. \c ConsumeStructuredBuffer<MyStruct> in HLSL.
    \see BufferDescriptor::stride
    \see MiscFlags::Append
    */
    ConsumeStructuredBuffer,
};


/* ----- Structures ----- */

/**
\brief Shader reflection resource structure.
\see ShaderReflection::resources
\see BindingDescriptor
*/
struct ShaderResourceReflection
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
    For all other resources, i.e. when \c type is not equal to ShaderResourceType::ConstantBuffer, this attribute is zero.
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
\brief Shader reflection structure.
\remarks Contains all information of resources and attributes that can be queried from a shader program.
This is not a "descriptor", because it is only used as output from an interface rather than a description to create something.
\see Shader::Reflect
*/
struct ShaderReflection
{
    //! List of all shader reflection resource views.
    std::vector<ShaderResourceReflection>   resources;

    /**
    \brief List of all uniforms (a.k.a. shader constants).
    \note Only supported with: OpenGL, Vulkan.
    \todo Add support to D3D11 and D3D12 for global constants.
    */
    std::vector<UniformDescriptor>          uniforms;

    /**
    \brief Reflection data that is specificly for the vertex shader.
    \remarks The shader reflection only considers the following members of the VertexAttribute structure,
    for both \c inputAttribs and \c outputAttribs in VertexShaderAttributes:
    - \c name
    - \c format
    - \c location
    - \c semanticIndex
    - \c systemValue
    */
    VertexShaderAttributes                  vertex;

    //! Reflection data that is specificly for the fragment shader.
    FragmentShaderAttributes                fragment;

    //! Reflection data that is specificly for the compute shader.
    ComputeShaderAttributes                 compute;
};


} // /namespace LLGL


#endif



// ================================================================================
