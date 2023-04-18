/*
 * MTRenderPass.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTRenderPass.h"
#include <LLGL/RenderPassFlags.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/SwapChainFlags.h>
#include <LLGL/Platform/Platform.h>
#include <LLGL/Misc/ForRange.h>
#include "../MTTypes.h"
#include "../Texture/MTTexture.h"
#include "../../CheckedCast.h"
#include "../../TextureUtils.h"
#include "../../RenderPassUtils.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


static void Convert(MTAttachmentFormat& dst, const AttachmentFormatDescriptor& src)
{
    dst.pixelFormat = MTTypes::ToMTLPixelFormat(src.format);
    dst.loadAction  = MTTypes::ToMTLLoadAction(src.loadOp);
    dst.storeAction = MTTypes::ToMTLStoreAction(src.storeOp);
}

static MTAttachmentFormat MakeMTAttachmentFormat(const Format format)
{
    MTAttachmentFormat fmt;
    {
        /* Set pixel format and default load and store actions */
        fmt.pixelFormat = MTTypes::ToMTLPixelFormat(format);
        fmt.loadAction  = MTLLoadActionDontCare;
        fmt.storeAction = MTLStoreActionStore;
    }
    return fmt;
}

// Initializer when a custom render pass is created
MTRenderPass::MTRenderPass(const RenderPassDescriptor& desc) :
    sampleCount_ { GetClampedSamples(desc.samples) }
{
    const auto numColorAttachments = NumEnabledColorAttachments(desc);
    LLGL_ASSERT(numColorAttachments <= LLGL_MAX_NUM_COLOR_ATTACHMENTS);
    colorAttachments_.resize(numColorAttachments);
    for_range(i, numColorAttachments)
        Convert(colorAttachments_[i], desc.colorAttachments[i]);
    Convert(depthAttachment_, desc.depthAttachment);
    Convert(stencilAttachment_, desc.stencilAttachment);
}

// Default initializer
MTRenderPass::MTRenderPass(const RenderTargetDescriptor& desc) :
    sampleCount_ { GetClampedSamples(desc.samples) }
{
    if (auto renderPass = desc.renderPass)
    {
        /* Copy render pass */
        auto renderPassMT = LLGL_CAST(const MTRenderPass*, renderPass);
        colorAttachments_   = renderPassMT->GetColorAttachments();
        depthAttachment_    = renderPassMT->GetDepthAttachment();
        stencilAttachment_  = renderPassMT->GetStencilAttachment();
    }
    else
    {
        /* Create default render pass for render target */
        for (const auto& attachment : desc.attachments)
        {
            const Format format = GetAttachmentFormat(attachment);
            if (IsDepthAndStencilFormat(format))
                depthAttachment_ = stencilAttachment_ = MakeMTAttachmentFormat(format);
            else if (IsDepthFormat(format))
                depthAttachment_ = MakeMTAttachmentFormat(format);
            else if (IsStencilFormat(format))
                stencilAttachment_ = MakeMTAttachmentFormat(format);
            else if (IsColorFormat(format))
                colorAttachments_.push_back(MakeMTAttachmentFormat(format));
        }
    }
}

static MTAttachmentFormat MakeDefaultMTAttachmentFormat(MTLPixelFormat format)
{
    MTAttachmentFormat fmt;
    {
        fmt.pixelFormat = format;
        fmt.loadAction  = MTLLoadActionDontCare;
        fmt.storeAction = MTLStoreActionStore;
    }
    return fmt;
}

static MTLPixelFormat GetColorMTLPixelFormat(int /*colorBits*/)
{
    return MTLPixelFormatBGRA8Unorm;
}

static MTLPixelFormat GetDepthStencilMTLPixelFormat(int depthBits, int stencilBits, id<MTLDevice> device)
{
    if (stencilBits == 8)
    {
        #ifdef LLGL_OS_MACOS
        if (depthBits == 24 && device != nil && device.depth24Stencil8PixelFormatSupported)
            return MTLPixelFormatDepth24Unorm_Stencil8;
        else
        #endif
            return MTLPixelFormatDepth32Float_Stencil8;
    }
    else
    {
        if (depthBits == 16)
            return MTLPixelFormatDepth16Unorm;
        else
            return MTLPixelFormatDepth32Float;
    }
}

// Swap-chain initializer
MTRenderPass::MTRenderPass(const SwapChainDescriptor& desc, id<MTLDevice> device) :
    sampleCount_ { GetClampedSamples(desc.samples) }
{
    const MTLPixelFormat colorFormat        = GetColorMTLPixelFormat(desc.colorBits);
    const MTLPixelFormat depthStencilFormat = GetDepthStencilMTLPixelFormat(desc.depthBits, desc.stencilBits, device);

    colorAttachments_ = { MakeDefaultMTAttachmentFormat(colorFormat) };

    if (desc.depthBits > 0)
        depthAttachment_ = MakeDefaultMTAttachmentFormat(depthStencilFormat);
    if (desc.stencilBits > 0)
        stencilAttachment_ = MakeDefaultMTAttachmentFormat(depthStencilFormat);
}

MTLPixelFormat MTRenderPass::GetDepthStencilFormat() const
{
    /* If both depth and stencil formats are used, they need to have the same format, so simply return one of them */
    if (depthAttachment_.pixelFormat != MTLPixelFormatInvalid)
        return depthAttachment_.pixelFormat;
    else
        return stencilAttachment_.pixelFormat;
}


} // /namespace LLGL



// ================================================================================
