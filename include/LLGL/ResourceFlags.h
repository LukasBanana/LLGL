/*
 * ResourceFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
        \brief The resource can be used to bind an output stream buffer (also referred to as "transform feedback").
        \remarks This can only be used for Buffer resources.
        \see CommandBuffer::BeginStreamOutput
        */
        StreamOutputBuffer      = (1 << 3),

        /**
        \brief Hint to the renderer that the resource will hold the arguments for indirect commands.
        \remarks This can only be used for Buffer resources.
        \see CommandBuffer::DrawIndirect
        \see CommandBuffer::DrawIndexedIndirect
        \see CommandBuffer::DispatchIndirect
        */
        IndirectBuffer          = (1 << 4),

        /**
        \brief The resource can be used to bind a buffer or texture for read access.
        \remarks This can be used for Buffer resources (e.g. \c samplerBuffer in GLSL, or \c StructuredBuffer in HLSL) and
        Texture resources (e.g. \c sampler2D in GLSL, or \c Texture2D in HLSL).
        */
        Sampled                 = (1 << 5),

        /**
        \brief The resource can be used to bind a buffer or texture for unordered read/write access.
        \remarks This can be used for Buffer resources (e.g. \c buffer in GLSL, or \c RWStructuredBuffer in HLSL) and
        Texture resources (e.g. \c image2D in GLSL, or \c RWTexture2D in HLSL).
        */
        Storage                 = (1 << 6),

        /**
        \brief Texture can be used as render target color attachment.
        \remarks This can only be used for Texture resources.
        \note This cannot be used together with the BindFlags::DepthStencilAttachment flag.
        \see AttachmentDescriptor::texture
        \see AttachmentType::Color
        */
        ColorAttachment         = (1 << 7),

        /**
        \brief Texture can be used as render target depth-stencil attachment.
        \remarks This can only be used for Texture resources.
        \note This cannot be used together with the BindFlags::ColorAttachment flag.
        \see AttachmentDescriptor::texture
        \see AttachmentType::DepthStencil
        */
        DepthStencilAttachment  = (1 << 8),

        /**
        \brief Specifies a resource as a combination of a Texture and Sampler (e.g. \c sampler2D in GLSL).
        \remarks This is only used for shader reflection and ignored by resource creation.
        \note Only supported with: OpenGL, Vulkan.
        \see ShaderResource::binding
        */
        CombinedSampler         = (1 << 9),

        /**
        \brief Specifies a resource can be used as source for a copy command.
        \see CommandBuffer::CopyBuffer
        \see CommandBuffer::CopyBufferFromTexture
        \see CommandBuffer::CopyTexture
        \see CommandBuffer::CopyTextureFromBuffer
        */
        CopySrc                 = (1 << 10),

        /**
        \brief Specifies a resource can be used as destination for a copy or fill command.
        \see CommandBuffer::CopyBuffer
        \see CommandBuffer::CopyBufferFromTexture
        \see CommandBuffer::CopyTexture
        \see CommandBuffer::CopyTextureFromBuffer
        \see CommandBuffer::FillBuffer
        */
        CopyDst                 = (1 << 11),
    };
};

/**
\brief CPU read/write access flag enumeration.
\see BufferDescriptor::cpuAccessFlags
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
        DynamicUsage    = (1 << 0),

        /**
        \brief Multi-sampled Texture resource has fixed sample locations.
        \remarks This can only be used with multi-sampled Texture resources (i.e. TextureType::Texture2DMS, TextureType::Texture2DMSArray).
        */
        FixedSamples    = (1 << 1),

        /**
        \brief Generates MIP-maps at texture creation time with the initial image data (if specified).
        \remarks To generate MIP-maps, the texture must be created with the binding flags BindFlags::Sampled and BindFlags::ColorAttachment, which is the default.
        \remarks This can be used to generate all MIP-maps when a new texture is created without explicitly encoding the CommandBuffer::GenerateMips function.
        The number of MIP-maps being generated depends on the \c mipLevels attribute in TextureDescriptor.
        \see TextureDescriptor::mipLevels
        \see CommandBuffer::GenerateMips
        */
        GenerateMips    = (1 << 2),

        /**
        \brief Specifies to ignore resource data initialization.
        \remarks If this is specified, a texture or buffer resource will stay uninitialized during creation and the content is undefined.
        */
        NoInitialData   = (1 << 3),

        /**
        \brief Enables a storage buffer to be used for \c AppendStructuredBuffer and \c ConsumeStructuredBuffer in HLSL only.
        \remarks This can only be used with buffers that also have the binding flag BindFlags::Storage and a \c stride greater than zero.
        \remarks This cannot be used together with the MiscFlags::Counter bit.
        \note Only supported with: Direct3D 11, Direct3D 12.
        \see BufferDescriptor::stride
        \see https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_buffer_uav_flag
        */
        Append          = (1 << 4),

        /**
        \brief Enables the hidden counter in a storage buffer to be used for \c RWStructuredBuffer in HLSL only.
        \remarks This can only be used with buffers that also have the binding flag BindFlags::Storage and a \c stride greater than zero.
        \remarks This cannot be used together with the MiscFlags::Append bit.
        \note Only supported with Direct3D 11, Direct3D 12.
        \see BufferDescriptor::stride
        \see https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_buffer_uav_flag
        */
        Counter         = (1 << 5),
    };
};


} // /namespace LLGL


#endif



// ================================================================================
