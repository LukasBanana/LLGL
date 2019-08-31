/*
 * RenderTargetFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RENDER_TARGET_FLAGS_H
#define LLGL_RENDER_TARGET_FLAGS_H


#include "TextureFlags.h"
#include "ForwardDecls.h"
#include "GraphicsPipelineFlags.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Render target attachment type enumeration.
\see AttachmentDescriptor
*/
enum class AttachmentType
{
    /**
    \brief Attachment is used for color output.
    \remarks A texture attached to a render target with this attachment type must have been created with binding flag BindFlags::ColorAttachment.
    */
    Color,

    /**
    \brief Attachment is used for depth component output.
    \remarks A texture attached to a render target with this attachment type must have been created with the binding flag BindFlags::DepthStencilAttachment.
    */
    Depth,

    /**
    \brief Attachment is used for depth component and stencil index output.
    \remarks A texture attached to a render target with this attachment type must have been created with the binding flag BindFlags::DepthStencilAttachment.
    */
    DepthStencil,

    /**
    \brief Attachment is used for stencil index output.
    \remarks A texture attached to a render target with this attachment type must have been created with the binding flag BindFlags::DepthStencilAttachment.
    */
    Stencil,
};


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
    inline AttachmentDescriptor(AttachmentType type) :
        type { type }
    {
    }

    //! Constructor for the specified depth-, stencil-, or color attachment.
    inline AttachmentDescriptor(AttachmentType type, Texture* texture, std::uint32_t mipLevel = 0, std::uint32_t arrayLayer = 0) :
        type       { type       },
        texture    { texture    },
        mipLevel   { mipLevel   },
        arrayLayer { arrayLayer }
    {
    }

    /**
    \brief Specifies for which output information the texture attachment is to be used,
    e.g. for color or depth information. By default AttachmentType::Color.
    */
    AttachmentType  type        = AttachmentType::Color;

    /**
    \brief Pointer to the texture which is to be used as target output. By default null.
    \remarks If this is null, the attribute \c type <b>must not</b> be AttachmentType::Color.
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
    LLGL::AttachmentDescriptor { LLGL::AttachmentType::Color, myColorTexture },
    LLGL::AttachmentDescriptor { LLGL::AttachmentType::Depth },
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

    //! Multi-sampling descriptor. By default, multi-sampling is disabled.
    MultiSamplingDescriptor             multiSampling;

    /**
    \brief Specifies whether custom multi-sampling is used or not. By default false.
    \remarks If this is true, only multi-sampled textures can be attached to a render-target,
    i.e. textures of the following types: Texture2DMS, Texture2DMSArray.
    If this is false, only non-multi-sampled textures can be attached to a render-target.
    This field will be ignored if multi-sampling is disabled.
    */
    bool                                customMultiSampling = false;

    /**
    \brief Specifies all render target attachment descriptors.
    \remarks This container can also be empty, if the respective fragment shader has no direct output but writes into a storage texture instead
    (e.g. <code>image3D</code> in GLSL, or <code>RWTexture3D<float4></code> in HLSL).
    If the respective rendering API does not support render targets without any attachments, LLGL will generate a dummy texture.
    */
    std::vector<AttachmentDescriptor>   attachments;
};


} // /namespace LLGL


#endif



// ================================================================================
