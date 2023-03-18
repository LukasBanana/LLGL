/*
 * PipelineLayoutFlags.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PIPELINE_LAYOUT_FLAGS_H
#define LLGL_PIPELINE_LAYOUT_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/BufferFlags.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/Container/StringView.h>
#include <vector>


namespace LLGL
{


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
\brief Layout structure for a single binding point of the pipeline layout descriptor.
\see PipelineLayoutDescriptor::bindings
*/
struct BindingDescriptor
{
    BindingDescriptor() = default;
    BindingDescriptor(const BindingDescriptor&) = default;

    //! Constructors with all primary attributes and a default value for a uniform array.
    inline BindingDescriptor(
        ResourceType    type,
        long            bindFlags,
        long            stageFlags,
        std::uint32_t   slot,
        std::uint32_t   arraySize = 0)
    :
        type       { type       },
        bindFlags  { bindFlags  },
        stageFlags { stageFlags },
        slot       { slot       },
        arraySize  { arraySize  }
    {
    }

    //! Constructors with all attributes.
    inline BindingDescriptor(
        const StringView&   name,
        ResourceType        type,
        long                bindFlags,
        long                stageFlags,
        std::uint32_t       slot,
        std::uint32_t       arraySize = 0)
    :
        name       { name.begin(), name.end() },
        type       { type                     },
        bindFlags  { bindFlags                },
        stageFlags { stageFlags               },
        slot       { slot                     },
        arraySize  { arraySize                }
    {
    }

    /**
    \brief Optional name for shading languages that do not support binding slots within the shader.
    \remarks This is only used for the OpenGL backend, when the GLSL version does not support <a href="https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Binding_points">explicit binding points</a>.
    If GLSL 420 or later is supported, this can be ignored and the binding points can be specified like this:
    \code
    #version 420
    layout(binding = 1) uniform sampler2D mySampler;
    \endcode
    Otherwise, the name of the resource must be included in this binding descriptor, e.g. <code>"mySampler"</code>.
    */
    std::string     name;

    //! Resource view type for this layout binding. By default ResourceType::Undefined.
    ResourceType    type        = ResourceType::Undefined;

    /**
    \brief Specifies to which kind of resource slot the resource will be bound. By default 0.
    \remarks Input and output binding flags cannot be used together when a resource is bound, e.g. a texture cannot be sampled while it is written to.
    \remarks When a Buffer is bound to a constant buffer slot for instance, the binding flag BindFlags::ConstantBuffer is required.
    When a Texture is bound to a sampled texture slot, the binding flag BindFlags::Sampled is required and so on.
    \see BindFlags
    */
    long            bindFlags   = 0;

    /**
    \brief Specifies which shader stages can access this resource binding. By default 0.
    \remarks This can be a bitwise OR combination of the StageFlags bitmasks.
    \see StageFlags
    */
    long            stageFlags  = 0;

    /**
    \brief Specifies the zero-based binding slot. By default 0.
    \note For Vulkan, each binding must have a unique slot within the same pipeline layout.
    */
    std::uint32_t   slot        = 0;

    /**
    \brief Specifies the number of binding slots for an array resource. By default 0.
    \remarks This can only be used for heap bindings, not for individual bindings.
    \note For Vulkan, this number specifies the size of an array of resources (e.g. an array of uniform buffers).
    \see PipelineLayoutDescriptor::heapBindings
    */
    std::uint32_t   arraySize   = 0;
};

/**
\brief Static sampler state pipeline layout descriptor.
\remarks Static samplers are part of a pipeline layout rather than a pipeline state.
This is the equivalent of a static sampler in a root siganture in Direct3D 12.
\see PipelineLayoutDescriptor::staticSamplers
*/
struct StaticSamplerDescriptor
{
    StaticSamplerDescriptor() = default;
    StaticSamplerDescriptor(const StaticSamplerDescriptor&) = default;

    //! Initializes the static sampler with stage flags, binding slot, and sampler state.
    inline StaticSamplerDescriptor(
        long                        stageFlags,
        std::uint32_t               slot,
        const SamplerDescriptor&    sampler)
    :
        stageFlags { stageFlags },
        slot       { slot       },
        sampler    { sampler    }
    {
    }

    //! Initializes the static sampler with a name, stage flags, binding slot, and sampler state.
    inline StaticSamplerDescriptor(
        const StringView&           name,
        long                        stageFlags,
        std::uint32_t               slot,
        const SamplerDescriptor&    sampler)
    :
        name       { name.begin(), name.end() },
        stageFlags { stageFlags               },
        slot       { slot                     },
        sampler    { sampler                  }
    {
    }

    /**
    \brief Optional name for shading languages that do not support binding slots within the shader.
    \remarks This is only used for the OpenGL backend, when the GLSL version does not support <a href="https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Binding_points">explicit binding points</a>.
    If GLSL 420 or later is supported, this can be ignored and the binding points can be specified like this:
    \code
    #version 420
    layout(binding = 1) uniform sampler2D mySampler;
    \endcode
    Otherwise, the name of the resource must be included in this binding descriptor, e.g. <code>"mySampler"</code>.
    */
    std::string         name;

    /**
    \brief Specifies which shader stages can access this static sampler. By default 0.
    \remarks This can be a bitwise OR combination of the StageFlags bitmasks.
    \see StageFlags
    */
    long                stageFlags  = 0;

    /**
    \brief Specifies the zero-based binding slot. By default 0.
    \note For Vulkan, each binding must have a unique slot within the same pipeline layout.
    */
    std::uint32_t       slot        = 0;

    /**
    \brief Specifies the static sampler state.
    \remarks Static samplers can only use one of three predefined border colors:
    - Transparent black: <code>{0,0,0,0}</code>
    - Opaque black: <code>{0,0,0,1}</code>
    - Opaque white: <code>{1,1,1,1}</code>
    */
    SamplerDescriptor   sampler;
};

/**
\brief Shader uniform pipeline layout descriptor.
\remarks Uniforms describe individual shader constants that have the same (i.e. uniform) value across all shader invocations for each draw and dispatch command.
\remarks For shader uniforms, no binding slot is specified. The backend determines the binding slot automatically via shader reflection for each pipeline state object (PSO).
\see PipelineLayoutDescriptor::uniforms
*/
struct UniformDescriptor
{
    UniformDescriptor() = default;
    UniformDescriptor(const UniformDescriptor&) = default;

    //! Initializes the uniform descriptor with a name, type, and optional array size.
    inline UniformDescriptor(
        const StringView&   name,
        UniformType         type,
        std::uint32_t       arraySize = 0)
    :
        name      { name.begin(), name.end() },
        type      { type                     },
        arraySize { arraySize                }
    {
    }

    /**
    \brief Specifies the name of an individual shader uniform.
    \remarks This describes the name of the constant itself and not its encloding constant buffer.
    */
    std::string     name;

    /**
    \brief Specifies the data type of the shader uniform. By default UniformType::Undefined.
    \remarks This describes the shader constant as scalar, vector, or matrix constant.
    When the pipeline layout is created, this field must have a valid uniform type, i.e. it must \e not be UniformType::Undefined.
    */
    UniformType     type        = UniformType::Undefined;

    //! Specifies the array size of the uniform. If this is 0, the uniform does \e not describe an array. By default 0.
    std::uint32_t   arraySize   = 0;
};

/**
\brief Pipeline layout descritpor structure.
\remarks Contains all layout bindings that will be used by graphics and compute pipelines.
\see RenderSystem::CreatePipelineLayout
*/
struct PipelineLayoutDescriptor
{
    /**
    \brief List of layout resource heap bindings.
    \remarks These bindings refer to the resource descriptors that are all bound at once with a single ResourceHeap.
    \remarks Only heap bindings can have subresource views as opposed to individual bindings that can only bind the entire resource.
    \remarks Only heap bindings can contain sampler states as opposed to individual bindings. This is due to a restriction of root parameters in Direct3D 12.
    \remarks In Direct3D 12 they are called "descriptor tables" and in Vulkan they are called "descriptor sets".
    \see CommandBuffer::SetResourceHeap
    \see ResourceViewDescriptor
    \see ResourceHeap::GetNumDescriptorSets
    */
    std::vector<BindingDescriptor>          heapBindings;

    /**
    \brief List of individual layout resource bindings.
    \remarks These bindings refer to individual resource descriptor that are bound separately.
    \remarks Individual bindings are limited to binding the entire resource as opposed to heap bindigs that can also bind a subresource view.
    \see CommandBuffer::SetResource
    */
    std::vector<BindingDescriptor>          bindings;

    /**
    \brief List of static sampler states with their binding points.
    \remarks These sampler states are immutable and are automatically bound with each pipeline state.
    */
    std::vector<StaticSamplerDescriptor>    staticSamplers;

    /**
    \brief List of shader uniforms that can be written dynamically.
    \remarks In Vulkan they are called "push constants", in OpenGL they are called "uniforms", and in Direct3D they are called "shader constants".
    \see CommandBuffer::SetUniform
    \see CommandBuffer::SetUniforms
    */
    std::vector<UniformDescriptor>          uniforms;
};


} // /namespace LLGL


#endif



// ================================================================================
