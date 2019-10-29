/*
 * MTRenderTarget.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderTarget.h"
#include "MTTexture.h"
#include "../../CheckedCast.h"
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
        switch (attachment.type)
        {
            case AttachmentType::Color:
                /* Create color attachment */
                CreateAttachment(
                    device,
                    native_.colorAttachments[numColorAttachments_],
                    attachment,
                    renderPass_.GetColorAttachments()[numColorAttachments_],
                    numColorAttachments_
                );
                ++numColorAttachments_;
                break;

            case AttachmentType::Depth:
                /* Create depth attachment */
                CreateAttachment(
                    device,
                    native_.depthAttachment,
                    attachment,
                    renderPass_.GetDepthAttachment(),
                    0
                );
                break;

            case AttachmentType::DepthStencil:
                /* Create depth-stencil attachment */
                CreateAttachment(
                    device,
                    native_.depthAttachment,
                    attachment,
                    renderPass_.GetDepthAttachment(),
                    0
                );
                Copy(native_.stencilAttachment, native_.depthAttachment);
                break;

            case AttachmentType::Stencil:
                /* Create stencil attachment */
                CreateAttachment(
                    device,
                    native_.stencilAttachment,
                    attachment,
                    renderPass_.GetStencilAttachment(),
                    0
                );
                break;
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
            attachment.texture          = CreateRenderTargetTexture(device, desc.type, tex);
            attachment.resolveTexture   = tex;
        }
        else
        {
            /* Set native texture in attachment */
            attachment.texture = tex;
        }
    }
    else if (desc.type != AttachmentType::Color)
    {
        /* Create native texture for depth-stencil attachments */
        attachment.texture = CreateRenderTargetTexture(device, desc.type);
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

static MTLPixelFormat SelectPixelFormat(const AttachmentType type)
{
    switch (type)
    {
        case AttachmentType::Color:         return MTLPixelFormatRGBA8Unorm;
        case AttachmentType::Depth:         return MTLPixelFormatDepth32Float;
        case AttachmentType::DepthStencil:  return MTLPixelFormatDepth32Float_Stencil8;
        case AttachmentType::Stencil:       return MTLPixelFormatStencil8;
    }
    return MTLPixelFormatInvalid;
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

id<MTLTexture> MTRenderTarget::CreateRenderTargetTexture(
    id<MTLDevice>                   device,
    const AttachmentType            type,
    id<MTLTexture>                  resolveTexture)
{
    auto texDesc = CreateTextureDesc(
        device,
        (resolveTexture != nil ? [resolveTexture pixelFormat] : SelectPixelFormat(type)),
        renderPass_.GetSampleCount()
    );

    id<MTLTexture> texture = [device newTextureWithDescriptor:texDesc];
    [texDesc release];

    return texture;
}


} // /namespace LLGL



// ================================================================================
