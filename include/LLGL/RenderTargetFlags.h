/*
 * RenderTargetFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_TARGET_FLAGS_H
#define LLGL_RENDER_TARGET_FLAGS_H


#include <LLGL/Format.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/ForwardDecls.h>
#include <LLGL/PipelineStateFlags.h>
#include <vector>
#include <cstdint>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Render target attachment descriptor structure.
\see RenderTargetDescriptor
*/
struct AttachmentDescriptor
{
    AttachmentDescriptor() = default;
    AttachmentDescriptor(const AttachmentDescriptor&) = default;

    //! Constructor for the specified depth-, or stencil attachment.
    inline AttachmentDescriptor(Format format) :
        format { format }
    {
    }

    //! Constructor for the specified depth-, stencil-, or color attachment.
    inline AttachmentDescriptor(Texture* texture, std::uint32_t mipLevel = 0, std::uint32_t arrayLayer = 0) :
        texture    { texture    },
        mipLevel   { mipLevel   },
        arrayLayer { arrayLayer }
    {
    }

    /**
    \brief Specifies the render-target attachment format. By default Format::Undefined.
    \remarks If this is undefined, the \c texture <b>must not</b> be null and the format will be determined by texture's format.
    \see Texture::GetFormat
    */
    Format          format      = Format::Undefined;

    /**
    \brief Pointer to the texture which is to be used as target output. By default null.
    \remarks If this is null, the attribute \c format <b>must not</b> be Format::Undefined.
    The texture must also have been created either with the binding flag BindFlags::ColorAttachment or BindFlags::DepthStencilAttachment.
    \see AttachmentDescriptor::type
    \see TextureDescriptor::bindFlags
    */
    Texture*        texture     = nullptr;

    /**
    \brief Specifies the MIP-map level which is to be attached to a render target.
    \remarks This is only used for non-multi-sample textures.
    All multi-sample textures will always use the first MIP-map level (i.e. TextureType::Texture2DMS and TextureType::Texture2DMSArray).
    \remarks If \c texture is null, this attribute is ignored.
    */
    std::uint32_t   mipLevel    = 0;

    /**
    \brief Specifies the array texture layer which is to be used as render target attachment.
    \remarks This is only used for array textures and cube textures (i.e. TextureType::Texture1DArray,
    TextureType::Texture2DArray, TextureType::TextureCube, TextureType::TextureCubeArray, and TextureType::Texture2DMSArray).
    \remarks For cube textures (i.e. TextureType::TextureCube and TextureType::TextureCubeArray), each cube has its own 6 array layers.
    The layer index for the respective cube faces is described at the TextureDescriptor::arrayLayers member.
    \remarks If \c texture is null, this attribute is ignored.
    \see TextureDescriptor::arrayLayers
    */
    std::uint32_t   arrayLayer  = 0;
};

/**
\brief Render target descriptor structure.
\remarks Here is a small example of a render target descriptor with a color attachmnet
and an anonymous depth attachment (i.e. without a texture reference, which is only allowed for depth/stencil attachments):
\code
LLGL::RenderTargetDescriptor myRenderTargetDesc;

auto myRenderTargetSize = myColorTexture->GetMipExtent(0);
myRenderTargetDesc.resolution = { myRenderTargetSize.width, myRenderTargetSize.height };

myRenderTargetDesc.attachments = {
    LLGL::AttachmentDescriptor{ myColorTexture },
    LLGL::AttachmentDescriptor{ LLGL::Format::D32Float },
};

auto myRenderTarget = myRenderer->CreateRenderTarget(myRenderTargetDesc);
\endcode
\see RenderSystem::CreateRenderTarget
*/
struct RenderTargetDescriptor
{
    /**
    \brief Optional render pass object that will be used with the render target. By default null.
    \remarks If this is null, a default render pass is created for the render target.
    The default render pass determines the attachment formats by the render target attachments and keeps the load and store operations at its default values.
    \see RenderSystem::CreateRenderPass
    \see AttachmentFormatDescriptor::loadOp
    \see AttachmentFormatDescriptor::storeOp
    */
    const RenderPass*                   renderPass          = nullptr;

    /**
    \brief Specifies the resolution of the render targets.
    \remarks All attachments with a reference to a texture must have the same resolution,
    i.e. the specified array layer and MIP-map level must have the same extent.
    \see Texture::GetMipExtent
    */
    Extent2D                            resolution;

    /**
    \brief Number of samples for the render targets. By default 1.
    \remarks If the specified number of samples is not supported, LLGL will silently reduce it.
    The actual number of samples can be queried by the \c GetSamples function of the RenderTarget interface.
    \see RenderTarget::GetSamples
    */
    std::uint32_t                       samples             = 1;

    #if 1//TODO: replace this

    /**
    \brief Specifies whether custom multi-sampling is used or not. By default false.
    \remarks If this is true, only multi-sampled textures can be attached to a render-target,
    i.e. textures of the following types: Texture2DMS, Texture2DMSArray.
    If this is false, only non-multi-sampled textures can be attached to a render-target.
    This field will be ignored if multi-sampling is disabled.
    \todo Remove this attribute and support custom resolve attachments instead.
    */
    bool                                customMultiSampling = false;

    /**
    \brief Specifies all render target attachment descriptors.
    \remarks This container can also be empty, if the respective fragment shader has no direct output but writes into a storage texture instead
    (e.g. \c image3D in GLSL, or <code>RWTexture3D<float4></code> in HLSL).
    If the respective rendering API does not support render targets without any attachments, LLGL will generate a dummy texture.
    \todo Replace this attribute by a fixed number of for color, resolve, and depth-stencil attachments.
    */
    std::vector<AttachmentDescriptor>   attachments;

    #else

    /**
    \brief Specifies the list of color attachment descriptors.
    \remarks Each attachment descriptor describes into which target will be rendered.
    \remarks For each attachment, for which a texture is specified, this texture must have the same number of samples as specified by RenderTargetDescriptor::samples,
    must have the same size as specified by RenderTargetDescriptor::resolution, and must have been created with the binding flag BindFlags::ColorAttachment.
    \see TextureDescriptor::samples
    */
    AttachmentDescriptor                colorAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];

    /**
    \brief Specifies the list of attachment descriptors for which the corresponding multi-sampled color attachments will be resolved into after a render pass.
    \remarks Each attachment descriptor describes a multi-sampled resolve target for the corresponding color attachment.
    \remarks For each attachment, for which a texture is specified, this texture must have 1 sample,
    must have the same size as specified by RenderTargetDescriptor::resolution, and must have been created with the binding flag BindFlags::ColorAttachment.
    */
    AttachmentDescriptor                resolveAttachments[LLGL_MAX_NUM_COLOR_ATTACHMENTS];

    /**
    \brief Specifies the depth-stencil attachment descriptor.
    \remarks If a texture is specified for this attachment, this texture must have the same number of samples as specified by RenderTargetDescriptor::samples,
    must have the same size as specified by RenderTargetDescriptor::resolution, and must have been created with the binding flag BindFlags::DepthStencilAttachment.
    \see TextureDescriptor::samples
    */
    AttachmentDescriptor                depthStencilAttachment;

    #endif
};


} // /namespace LLGL


#endif



// ================================================================================
