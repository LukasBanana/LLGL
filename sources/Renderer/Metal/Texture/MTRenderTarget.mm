/*
 * MTRenderTarget.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTRenderTarget.h"
#include "MTTexture.h"
#include "../MTTypes.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include <string>


namespace LLGL
{


static void Copy(MTLRenderPassAttachmentDescriptor* dst, const MTLRenderPassAttachmentDescriptor* src)
{
    dst.texture 	= src.texture;
    dst.level       = src.level;
    dst.slice       = src.slice;
    dst.depthPlane  = src.depthPlane;
    dst.loadAction  = src.loadAction;
    dst.storeAction = src.storeAction;
    //[dst.texture retain];
}

MTRenderTarget::MTRenderTarget(id<MTLDevice> device, const RenderTargetDescriptor& desc) :
    resolution_ { desc.resolution },
    renderPass_ { desc            }
{
    /* Allocate native render pass descriptor */
    native_ = [[MTLRenderPassDescriptor alloc] init];

    for (const auto& attachment : desc.attachments)
    {
        const Format format = GetAttachmentFormat(attachment);
        if (IsColorFormat(format))
        {
            /* Create color attachment */
            CreateAttachment(
                device,
                native_.colorAttachments[numColorAttachments_],
                attachment,
                renderPass_.GetColorAttachments()[numColorAttachments_],
                numColorAttachments_
            );
            ++numColorAttachments_;
        }
        else if (IsDepthAndStencilFormat(format))
        {
            /* Create depth-stencil attachment */
            CreateAttachment(
                device,
                native_.depthAttachment,
                attachment,
                renderPass_.GetDepthAttachment(),
                0
            );
            Copy(native_.stencilAttachment, native_.depthAttachment);
        }
        else if (IsDepthFormat(format))
        {
            /* Create depth attachment */
            CreateAttachment(
                device,
                native_.depthAttachment,
                attachment,
                renderPass_.GetDepthAttachment(),
                0
            );
        }
        else if (IsStencilFormat(format))
        {
            /* Create stencil attachment */
            CreateAttachment(
                device,
                native_.stencilAttachment,
                attachment,
                renderPass_.GetStencilAttachment(),
                0
            );
        }
    }
}

MTRenderTarget::~MTRenderTarget()
{
    #if 0//TODO: code analysis warns about invalid reference counting
    /* Release all textures */
    for (std::uint32_t i = 0; i < numColorAttachments_; ++i)
        [native_.colorAttachments[i] release];
    if (auto attachment = native_.depthAttachment)
        [attachment.texture release];
    if (auto attachment = native_.stencilAttachment)
        [attachment.texture release];

    //[native_ release];
    #endif
}

Extent2D MTRenderTarget::GetResolution() const
{
    return resolution_;
}

std::uint32_t MTRenderTarget::GetSamples() const
{
    return static_cast<std::uint32_t>(renderPass_.GetSampleCount());
}

std::uint32_t MTRenderTarget::GetNumColorAttachments() const
{
    return numColorAttachments_;
}

bool MTRenderTarget::HasDepthAttachment() const
{
    return (native_.depthAttachment.texture != nil);
}

bool MTRenderTarget::HasStencilAttachment() const
{
    return (native_.stencilAttachment.texture != nil);
}

const RenderPass* MTRenderTarget::GetRenderPass() const
{
    return (&renderPass_);
}


/*
 * ======= Private: =======
 */

void MTRenderTarget::CreateAttachment(
    id<MTLDevice>                   	device,
    MTLRenderPassAttachmentDescriptor*  attachment,
    const AttachmentDescriptor&         desc,
    const MTAttachmentFormat&           fmt,
    std::uint32_t                       slot)
{
    if (auto texture = desc.texture)
    {
        /* Get native texture and increment reference counter */
        auto textureMT = LLGL_CAST(MTTexture*, texture);
        id<MTLTexture> tex = textureMT->GetNative();
        [tex retain];

        if (renderPass_.GetSampleCount() > 1)
        {
            /* Create resolve texture if multi-sampling is enabled */
            attachment.texture          = CreateAttachmentTexture(device, [tex pixelFormat]);
            attachment.resolveTexture   = tex;
        }
        else
        {
            /* Set native texture in attachment */
            attachment.texture = tex;
        }
    }
    else if (IsDepthAndStencilFormat(desc.format))
    {
        /* Create native texture for depth-stencil attachments */
        attachment.texture = CreateAttachmentTexture(device, MTTypes::ToMTLPixelFormat(desc.format));
    }
    else
    {
        /* Color attachment require a texture object -> error */
        throw std::invalid_argument(
            "failed to create render target: "
            "missing texture for color attachment (at slot " + std::to_string(slot) + ")"
        );
    }

    /* Store remaining attachment parameters */
    attachment.level        = desc.mipLevel;
    attachment.slice        = desc.arrayLayer;
    attachment.loadAction   = fmt.loadAction;
    attachment.storeAction  = fmt.storeAction;

    if (attachment.storeAction == MTLStoreActionStore && attachment.resolveTexture != nil)
        attachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
}

MTLTextureDescriptor* MTRenderTarget::CreateTextureDesc(
    id<MTLDevice>   device,
    MTLPixelFormat  pixelFormat,
    NSUInteger      sampleCount)
{
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    {
        texDesc.textureType     = (sampleCount > 1 ? MTLTextureType2DMultisample : MTLTextureType2D);
        texDesc.pixelFormat     = pixelFormat;
        texDesc.width           = resolution_.width;
        texDesc.height          = resolution_.height;
        texDesc.sampleCount     = sampleCount;
        texDesc.usage           = MTLTextureUsageRenderTarget;
        texDesc.resourceOptions = MTLResourceStorageModePrivate;
    }
    return texDesc;
}

id<MTLTexture> MTRenderTarget::CreateAttachmentTexture(id<MTLDevice> device, MTLPixelFormat pixelFormat)
{
    auto texDesc = CreateTextureDesc(device, pixelFormat, renderPass_.GetSampleCount());

    id<MTLTexture> texture = [device newTextureWithDescriptor:texDesc];
    [texDesc release];

    return texture;
}


} // /namespace LLGL



// ================================================================================
