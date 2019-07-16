/*
 * ResourceFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_FLAGS_H
#define LLGL_RESOURCE_FLAGS_H


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Hardware resource type enumeration.
\remarks This is primarily used to describe the source type for a layout binding (see BindingDescriptor),
which is why all buffer types are enumerated but not the texture types.
\see BindingDescriptor::type
*/
enum class ResourceType
{
    //! Undefined resource type.
    Undefined,

    /**
    \brief Buffer resource.
    \see Buffer
    \see RenderSystem::CreateBuffer
    */
    Buffer,

    /**
    \brief Texture resource.
    \see Texture
    \see TextureType
    \see RenderSystem::CreateTexture
    */
    Texture,

    /**
    \brief Sampler state resource.
    \see Sampler
    \see RenderSystem::CreateSampler
    */
    Sampler,
};

/**
\brief Flags for Buffer and Texture resources that describe for which purposes they will be used.
\see BufferDescriptor::bindFlags
\see TextureDescriptor::bindFlags
\see BindingDescriptor::bindFlags
\see ShaderReflectionDescriptor::ResourceView::bindFlags
*/
struct BindFlags
{
    enum
    {
        /**
        \brief The resource can be used to bind a stream of vertices.
        \remarks This can only be used for Buffer resources.
        \see CommandBuffer::SetVertexBuffer
        */
        VertexBuffer            = (1 << 0),

        /**
        \brief The resource can be used to bind a stream of indices.
        \remarks This can only be used for Buffer resources.
        \see CommandBuffer::SetIndexBuffer
        */
        IndexBuffer             = (1 << 1),

        /**
        \brief The resource can be used to bind a set of constants.
        \remarks This can only be used for Buffer resources.
        */
        ConstantBuffer          = (1 << 2),

        /**
        \brief The resource can be used to bind a buffer for read access.
        \remarks This can be used for Buffer resources (e.g. \c samplerBuffer in GLSL, or \c StructuredBuffer in HLSL) and
        Texture resources (e.g. \c sampler2D in GLSL, or \c Texture2D in HLSL).
        \todo Maybe rename to SRV or ShaderRead.
        */
        SampleBuffer            = (1 << 3),

        /**
        \brief The resource can be used to bind a storage for unordered read/write access.
        \remarks This can be used for Buffer resources (e.g. \c buffer in GLSL, or \c RWStructuredBuffer in HLSL) and
        Texture resources (e.g. \c image2D in GLSL, or \c RWTexture2D in HLSL).
        \todo Maybe rename to UAV or ShaderWrite.
        */
        RWStorageBuffer         = (1 << 4),

        /**
        \brief The resource can be used to bind an output stream buffer (also referred to as "transform feedback").
        \remarks This can only be used for Buffer resources.
        \see CommandBuffer::SetStreamOutputBuffer
        */
        StreamOutputBuffer      = (1 << 5),

        /**
        \brief Hint to the renderer that the resource will hold the arguments for indirect commands.
        \remarks This can only be used for Buffer resources.
        \see CommandBuffer::DrawIndirect
        \see CommandBuffer::DrawIndexedIndirect
        \see CommandBuffer::DispatchIndirect
        */
        IndirectBuffer          = (1 << 6),

        /**
        \brief Texture can be used as render target color attachment.
        \remarks This can only be used for Texture resources.
        \note This cannot be used together with the BindFlags::DepthStencilAttachment flag.
        \see AttachmentDescriptor::texture
        \see AttachmentType::Color
        \todo Maybe rename to RTV
        */
        ColorAttachment         = (1 << 7),

        /**
        \brief Texture can be used as render target depth-stencil attachment.
        \remarks This can only be used for Texture resources.
        \note This cannot be used together with the BindFlags::ColorAttachment flag.
        \see AttachmentDescriptor::texture
        \see AttachmentType::DepthStencil
        \todo Maybe rename to DSV
        */
        DepthStencilAttachment  = (1 << 8),
    };
};

/**
\brief CPU read/write access flag enumeration.
\see BufferDescriptor::cpuAccessFlags
\see TextureDescriptor::cpuAccessFlags
\see CPUAccess
*/
struct CPUAccessFlags
{
    enum
    {
        /**
        \brief Resource mapping with CPU read access is required.
        \see RenderSystem::MapBuffer
        */
        Read        = (1 << 0),

        /**
        \brief Resource mapping with CPU write access is required.
        \see RenderSystem::MapBuffer
        */
        Write       = (1 << 1),

        /*
        \brief Resource mapping with CPU read and write access is required.
        \see CPUAccessFlags::Read
        \see CPUAccessFlags::Write
        */
        ReadWrite   = (Read | Write),
    };
};

/**
\brief Miscellaneous resource flags.
\see BufferDescriptor::miscFlags
\see TextureDescriptor::miscFlags
*/
struct MiscFlags
{
    enum
    {
        /**
        \brief Hint to the renderer that the resource will be frequently updated from the CPU.
        \remarks This is useful for a constant buffer for instance, that is updated by the host program every frame.
        \see RenderSystem::WriteBuffer
        \see RenderSystem::WriteTexture
        \todo Restriction required to support deferred context in D3D11. This must no longer be just a "hint", it must be a strictly defined attribute for a buffer.
        */
        DynamicUsage = (1 << 0),

        /**
        \brief Multi-sampled Texture resource has fixed sample locations.
        \remarks This can only be used with multi-sampled Texture resources (i.e. TextureType::Texture2DMS, TextureType::Texture2DMSArray).
        */
        FixedSamples = (1 << 1),
    };
};


} // /namespace LLGL


#endif



// ================================================================================
