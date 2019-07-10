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
    \remarks If this is used, the counter part must also be specified (i.e. <code>tessEvaluationShader</code>).
    \see tessEvaluationShader
    */
    Shader*                     tessControlShader       = nullptr;

    /**
    \brief Specifies the tessellation-evaluation shader (also referred to as "Domain Shader").
    \remarks If this is used, the counter part must also be specified (i.e. <code>tessControlShader</code>).
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
\brief Shader reflection descriptor structure.
\remarks Contains all information of resources and attributes that can be queried from a shader program.
\see ShaderProgram::QueryReflectionDesc
*/
struct ShaderReflectionDescriptor
{
    /**
    \brief Shader reflection resource view structure.
    \remarks A mapping between this structure and a binding descriptor may look like this:
    \code
    auto myShaderReflectionDesc = myShaderProgram->QueryReflectionDesc();
    LLGL::PipelineLayoutDescriptor myPipelineLayoutDesc;
    for (const auto& myResourceView : myShaderReflectionDesc) {
        BindingDescriptor myBindingDesc;
        myBindingDesc.type          = myResourceView.type;
        myBindingDesc.stageFlags    = myResourceView.stageFlags;
        myBindingDesc.slot          = myResourceView.slot;
        myBindingDesc.arraySize     = myResourceView.arraySize;
        myPipelineLayoutDesc.bindings.push_back(myBindingDesc);
    }
    \endcode
    \see ShaderReflectionDescriptor::resourceViews
    \see BindingDescriptor
    */
    struct ResourceView
    {
        //! Name of the shader resource, i.e. the identifier used in the shader.
        std::string         name;

        //! Resource view type for this layout binding. By default ResourceType::Undefined.
        ResourceType        type                = ResourceType::Undefined;

        /**
        \brief Specifies to which kind of resource slot the resource is bound. By default 0.
        \remarks For a constant buffer for instance, the flags will contain the BindFlags::ConstantBuffer flag.
        For a 2D texture for instance, the flags will contain the BindFlags::SampleBuffer flag.
        \see BindFlags
        */
        long                bindFlags           = 0;

        /**
        \brief Specifies in which shader stages the resource is located. By default 0.
        \remarks This can be a bitwise OR combination of the StageFlags bitmasks.
        \see StageFlags
        */
        long                stageFlags          = 0;

        /**
        \brief Specifies the zero-based binding slot. By default 0.
        \remarks If the binding slot could be not queried by the shader reflection, the value is Constants::invalidSlot.
        \see Constants::invalidSlot
        */
        std::uint32_t       slot                = 0;

        /**
        \brief Specifies the number of binding slots for an array resource. By default 1.
        \note For Vulkan, this number specifies the size of an array of resources (e.g. an array of uniform buffers).
        */
        std::uint32_t       arraySize           = 1;

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
    \brief Shader uniform descriptor structure.
    \see ShaderReflectionDescriptor::uniforms
    */
    struct Uniform
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

    //! List of all vertex attributes.
    std::vector<VertexAttribute>        vertexAttributes;

    //! List of all stream-output attributes.
    std::vector<StreamOutputAttribute>  streamOutputAttributes;

    //! List of all shader reflection resource views.
    std::vector<ResourceView>           resourceViews;

    /**
    \brief List of all uniforms (a.k.a. shader constants).
    \note Only supported with: OpenGL, Vulkan.
    */
    std::vector<Uniform>                uniforms;
};


} // /namespace LLGL


#endif



// ================================================================================
