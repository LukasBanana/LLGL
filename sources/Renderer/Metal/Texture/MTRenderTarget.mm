/*
 * MTRenderTarget.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTRenderTarget.h"
#include "MTTexture.h"
#include "../MTTypes.h"
#include "../MTDevice.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/Exception.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include <string>


namespace LLGL
{


static void CopyMTLAttachmentDesc(MTLRenderPassAttachmentDescriptor* dst, const MTLRenderPassAttachmentDescriptor* src)
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
    resolution_          { desc.resolution                 },
    numColorAttachments_ { NumActiveColorAttachments(desc) },
    renderPass_          { device, desc                    }
{
    /* Allocate native render pass descriptor */
    nativeRenderPass_ = [[MTLRenderPassDescriptor alloc] init];

    for_range(i, numColorAttachments_)
    {
        const auto& attachment = desc.colorAttachments[i];
        const Format format = GetAttachmentFormat(attachment);
        if (IsColorFormat(format))
        {
            /* Create color attachment */
            CreateAttachment(
                device,
                nativeRenderPass_.colorAttachments[i],
                attachment,
                &(desc.resolveAttachments[i]),
                renderPass_.GetColorAttachments()[i],
                i
            );
        }
        else
            LLGL_TRAP("invalid format for render-target color attachment: %s", ToString(format));
    }

    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        const auto& attachment = desc.depthStencilAttachment;
        const Format format = GetAttachmentFormat(attachment);
        if (IsDepthAndStencilFormat(format))
        {
            /* Create depth-stencil attachment */
            CreateAttachment(
                device,
                nativeRenderPass_.depthAttachment,
                attachment,
                nullptr,
                renderPass_.GetDepthAttachment(),
                0
            );
            CopyMTLAttachmentDesc(nativeRenderPass_.stencilAttachment, nativeRenderPass_.depthAttachment);
        }
        else if (IsDepthFormat(format))
        {
            /* Create depth attachment */
            CreateAttachment(
                device,
                nativeRenderPass_.depthAttachment,
                attachment,
                nullptr,
                renderPass_.GetDepthAttachment(),
                0
            );
        }
        else if (IsStencilFormat(format))
        {
            /* Create stencil attachment */
            CreateAttachment(
                device,
                nativeRenderPass_.stencilAttachment,
                attachment,
                nullptr,
                renderPass_.GetStencilAttachment(),
                0
            );
        }
        else
            LLGL_TRAP("invalid format for render-target depth-stencil attachment: %s", ToString(format));
    }
}

MTRenderTarget::~MTRenderTarget()
{
    /* Release all internally created textures */
    for (id<MTLTexture> tex : internalTextures_)
        [tex release];
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
    return (nativeRenderPass_.depthAttachment.texture != nil);
}

bool MTRenderTarget::HasStencilAttachment() const
{
    return (nativeRenderPass_.stencilAttachment.texture != nil);
}

const RenderPass* MTRenderTarget::GetRenderPass() const
{
    return (&renderPass_);
}

MTLRenderPassDescriptor* MTRenderTarget::GetAndUpdateNativeRenderPass(
    const MTRenderPass& renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    /* Create copy of native render pass descriptor for the first time */
    if (nativeMutableRenderPass_ == nil)
        nativeMutableRenderPass_ = [nativeRenderPass_ copy];

    /* Update mutable render pass with clear values */
    if (renderPass.GetColorAttachments().size() == numColorAttachments_)
        renderPass.UpdateNativeRenderPass(nativeMutableRenderPass_, numClearValues, clearValues);

    return nativeMutableRenderPass_;
}


/*
 * ======= Private: =======
 */

void MTRenderTarget::CreateAttachment(
    id<MTLDevice>                   	device,
    MTLRenderPassAttachmentDescriptor*  outAttachment,
    const AttachmentDescriptor&         inAttachment,
    const AttachmentDescriptor*         inResolveAttachment,
    const MTAttachmentFormat&           fmt,
    std::uint32_t                       slot)
{
    if (Texture* texture = inAttachment.texture)
    {
        auto& textureMT = LLGL_CAST(MTTexture&, *texture);
        id<MTLTexture> tex = textureMT.GetNative();

        if (inAttachment.format != Format::Undefined)
        {
            /* Create texture view with format */
            const MTLPixelFormat pixelFormat = MTTypes::ToMTLPixelFormat(inAttachment.format);
            outAttachment.texture = CreateAttachmentTextureView(tex, pixelFormat);
        }
        else
        {
            /* Use input texture directly */
            outAttachment.texture = tex;
        }
    }
    else
    {
        /* Create internal texture for attachment */
        const MTLPixelFormat pixelFormat = MTTypes::ToMTLPixelFormat(inAttachment.format);
        outAttachment.texture = CreateAttachmentTexture(device, pixelFormat);
    }

    /* Add optional resolve attachment if a texture is specified */
    if (inResolveAttachment != nullptr && inResolveAttachment->texture != nullptr)
    {
        auto& textureMT = LLGL_CAST(MTTexture&, *inResolveAttachment->texture);
        id<MTLTexture> tex = textureMT.GetNative();

        if (inResolveAttachment->format != Format::Undefined)
        {
            /* Create texture view with format */
            const MTLPixelFormat pixelFormat = MTTypes::ToMTLPixelFormat(inAttachment.format);
            outAttachment.resolveTexture = CreateAttachmentTextureView(tex, pixelFormat);
        }
        else
        {
            /* Use input texture directly */
            outAttachment.resolveTexture = tex;
        }

        outAttachment.resolveLevel = inResolveAttachment->mipLevel;
        outAttachment.resolveSlice = inResolveAttachment->arrayLayer;
    }

    /* Store remaining attachment parameters */
    outAttachment.level        = inAttachment.mipLevel;
    outAttachment.slice        = inAttachment.arrayLayer;
    outAttachment.loadAction   = fmt.loadAction;
    outAttachment.storeAction  = fmt.storeAction;

    if (outAttachment.storeAction == MTLStoreActionStore && outAttachment.resolveTexture != nil)
        outAttachment.storeAction = MTLStoreActionStoreAndMultisampleResolve;
}

MTLTextureDescriptor* MTRenderTarget::CreateTextureDesc(
    id<MTLDevice>   device,
    MTLPixelFormat  pixelFormat,
    NSUInteger      sampleCount)
{
    sampleCount = MTDevice::FindSuitableSampleCountOr1(device, static_cast<std::uint32_t>(sampleCount));
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
    MTLTextureDescriptor* texDesc = CreateTextureDesc(device, pixelFormat, renderPass_.GetSampleCount());

    id<MTLTexture> newTexture = [device newTextureWithDescriptor:texDesc];
    internalTextures_.push_back(newTexture);
    [texDesc release];

    return newTexture;
}

id<MTLTexture> MTRenderTarget::CreateAttachmentTextureView(id<MTLTexture> sourceTexture, MTLPixelFormat pixelFormat)
{
    id<MTLTexture> newTextureView = [sourceTexture newTextureViewWithPixelFormat:pixelFormat];
    internalTextures_.push_back(newTextureView);
    return newTextureView;
}


} // /namespace LLGL



// ================================================================================
