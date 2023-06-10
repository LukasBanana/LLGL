/*
 * PipelineLayoutFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
\remarks All uniform types have WORD size, i.e. they are a multiple of 32-bits.
\remarks Because "Bool" is a reserved identifier for an Xlib macro on GNU/Linux,
all scalar types have a component index (i.e. \c Bool1 instead of \c Bool).
\see LLGL::Parse
*/
enum class UniformType
{
    Undefined,      //!< Undefined uniform type.

    /* ----- Scalars & Vectors ----- */
    Float1,         //!< Single scalar of a 32-bit floating-point. Use \c "float1" or \c "float" for parsing.
    Float2,         //!< Two component vector of 32-bit floating-points. Use \c "float2" for parsing.
    Float3,         //!< Three component vector of 32-bit floating-points. Use \c "float3" for parsing.
    Float4,         //!< Four component vector of 32-bit floating-points. Use \c "float4" for parsing.
    Double1,        //!< Single scalar of a 64-bit floating-point. Use \c "double1" or \c "double" for parsing.
    Double2,        //!< Two component vector of 64-bit floating-points. Use \c "double2" for parsing.
    Double3,        //!< Three component vector of 64-bit floating-points. Use \c "double3" for parsing.
    Double4,        //!< Four component vector of 64-bit floating-points. Use \c "double4" for parsing.
    Int1,           //!< Single scalar of a 32-bit signed integer. Use \c "int1" or \c "int" for parsing.
    Int2,           //!< Two component vector of 32-bit signed integers. Use \c "int2" for parsing.
    Int3,           //!< Three component vector of 32-bit signed integers. Use \c "int3" for parsing.
    Int4,           //!< Four component vector of 32-bit signed integers. Use \c "int4" for parsing.
    UInt1,          //!< Single scalar of a 32-bit unsigned integer. Use \c "int1" or \c "int" for parsing.
    UInt2,          //!< Two component vector of 32-bit unsigned integers. Use \c "int2" for parsing.
    UInt3,          //!< Three component vector of 32-bit unsigned integers. Use \c "int3" for parsing.
    UInt4,          //!< Four component vector of 32-bit unsigned integers. Use \c "int4" for parsing.
    Bool1,          //!< Single scalar of a 32-bit boolean. Use \c "bool1" or \c "bool" for parsing.
    Bool2,          //!< Two component vector of 32-bit booleans. Use \c "bool2" for parsing.
    Bool3,          //!< Three component vector of 32-bit booleans. Use \c "bool3" for parsing.
    Bool4,          //!< Four component vector of 32-bit booleans. Use \c "bool4" for parsing.

    /* ----- Matrices ----- */
    Float2x2,       //!< 2x2 matrix of 32-bit floating-points. Use \c "float2x2" for parsing.
    Float2x3,       //!< 2x3 matrix of 32-bit floating-points. Use \c "float2x3" for parsing.
    Float2x4,       //!< 2x4 matrix of 32-bit floating-points. Use \c "float2x4" for parsing.
    Float3x2,       //!< 3x2 matrix of 32-bit floating-points. Use \c "float3x2" for parsing.
    Float3x3,       //!< 3x3 matrix of 32-bit floating-points. Use \c "float3x3" for parsing.
    Float3x4,       //!< 3x4 matrix of 32-bit floating-points. Use \c "float3x4" for parsing.
    Float4x2,       //!< 4x2 matrix of 32-bit floating-points. Use \c "float4x2" for parsing.
    Float4x3,       //!< 4x3 matrix of 32-bit floating-points. Use \c "float4x3" for parsing.
    Float4x4,       //!< 4x4 matrix of 32-bit floating-points. Use \c "float4x4" for parsing.
    Double2x2,      //!< 2x2 matrix of 64-bit floating-points. Use \c "double2x2" for parsing.
    Double2x3,      //!< 2x3 matrix of 64-bit floating-points. Use \c "double2x3" for parsing.
    Double2x4,      //!< 2x4 matrix of 64-bit floating-points. Use \c "double2x4" for parsing.
    Double3x2,      //!< 3x2 matrix of 64-bit floating-points. Use \c "double3x2" for parsing.
    Double3x3,      //!< 3x3 matrix of 64-bit floating-points. Use \c "double3x3" for parsing.
    Double3x4,      //!< 3x4 matrix of 64-bit floating-points. Use \c "double3x4" for parsing.
    Double4x2,      //!< 4x2 matrix of 64-bit floating-points. Use \c "double4x2" for parsing.
    Double4x3,      //!< 4x3 matrix of 64-bit floating-points. Use \c "double4x3" for parsing.
    Double4x4,      //!< 4x4 matrix of 64-bit floating-points. Use \c "double4x4" for parsing.

    /* ----- Resources ----- */
    Sampler,        //!< Sampler uniform (e.g. \c sampler2D). Not supported for parsing.
    Image,          //!< Image uniform (e.g. \c image2D). Not supported for parsing.
    AtomicCounter,  //!< Atomic counter uniform (e.g. \c atomic_uint). Not supported for parsing.
};


/* ----- Structures ----- */

/**
\brief Resource binding slot structure.
\remarks This is used to unify the description of resource binding slots and sets.
\see BindingDescriptor::slot
*/
struct BindingSlot
{
    BindingSlot() = default;
    BindingSlot(const BindingSlot&) = default;

    //! Constructs the binding slot with an index and an optional set (for Vulkan).
    inline BindingSlot(std::uint32_t index, std::uint32_t set = 0) :
        index { index },
        set   { set   }
    {
    }

    /**
    \brief Specifies the zero-based binding index. By default 0.
    \remarks For Vulkan, each binding must have a unique slot within the same pipeline layout unless they are in different descriptor sets.
    \see set
    */
    std::uint32_t index = 0;

    /**
    \brief Specifies the zero-based descriptor set.
    \remarks For Vulkan, each binding must have a unique slot within the same pipeline layout unless they are in different descriptor sets.
    LLGL will also re-assign these descriptor set indices according to the internal binding layout for the Vulkan backend,
    i.e. modify <code>OpDecorate ID DescriptorSet SET</code> SPIR-V instructions.
    \remarks This field is silently ignored by backends that do not support binding sets, aka. register spaces.
    \note Only supported with: Vulkan, Direct3D 12.
    \see slot
    */
    std::uint32_t set   = 0;
};

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
        ResourceType        type,
        long                bindFlags,
        long                stageFlags,
        const BindingSlot&  slot,
        std::uint32_t       arraySize = 0)
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
        const BindingSlot&  slot,
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

    //! Specifies the binding slot for the resource.
    BindingSlot     slot;

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
        const BindingSlot&          slot,
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
        const BindingSlot&          slot,
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

    //! Specifies the binding slot of the static sampler.
    BindingSlot         slot;

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
    \brief Specifies the name of an individual shader uniform. This <b>must not</b> be empty.
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
    \remarks In Direct3D 12 they are called "descriptor tables" and in Vulkan they are called "descriptor sets".
    \see CommandBuffer::SetResourceHeap
    \see ResourceViewDescriptor
    \see ResourceHeap::GetNumDescriptorSets
    */
    std::vector<BindingDescriptor>          heapBindings;

    /**
    \brief List of individual layout resource bindings.
    \remarks These bindings refer to individual resource descriptors that are bound separately from a ResourceHeap.
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
    \remarks Uniforms are not described by their binding slot, but solely by their name and type.
    They represent a small range within one or more constant buffers or push constant ranges and therefore don't have individual binding slots.
    Their offests within these buffers are determined by LLGL when a PipelineState object (PSO) is created with a PipelineLayout that contains uniform descriptors.
    This process is highly dependent on the backend, the shaders used within the PSO, and other factors.
    It is advised to keep the number of uniforms small as there is only a very limited amount of space for uniforms:
    - Vulkan only guarantees a minimum of 128 bytes for the push constant ranges which is shared across all shader stages within a single PSO.
    That's just enough for two UniformType::Float4x4 matrices.
    - Metal has a limit of 4KB of on-demand block data in its command encoders which is utilized for uniforms in this backend.
    - Direct3D 12 has a fixed limit of 64 DWORDS (= 256 bytes) per root signature, which are claimed by all root parameters, including root constants (aka. uniforms).
    Only static samplers are not counted towards root parameters.
    \see CommandBuffer::SetUniforms
    */
    std::vector<UniformDescriptor>          uniforms;
};


/* ----- Operators ----- */

//! Returns true if the specified binding slots are equal, i.e. \c index and \c set are equal.
inline bool operator == (const BindingSlot& lhs, const BindingSlot& rhs)
{
    return (lhs.index == rhs.index && lhs.set == rhs.set);
}

//! Returns true if the specified binding slots are not equal, i.e. \c index and \c set are not equal.
inline bool operator != (const BindingSlot& lhs, const BindingSlot& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL


#endif



// ================================================================================
