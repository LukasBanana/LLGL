/*
 * MTRenderPass.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTRenderPass.h"
#include <LLGL/RenderPassFlags.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/SwapChainFlags.h>
#include <LLGL/Platform/Platform.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include "../MTTypes.h"
#include "../Texture/MTTexture.h"
#include "../MTDevice.h"
#include "../../CheckedCast.h"
#include "../../TextureUtils.h"
#include "../../RenderPassUtils.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


static NSUInteger GetMTRenderPassSampleCount(id<MTLDevice> device, std::uint32_t sampleCount)
{
    return MTDevice::FindSuitableSampleCountOr1(device, static_cast<NSUInteger>(GetClampedSamples(sampleCount)));
}

static void ConvertAttachmentFormat(MTAttachmentFormat& dst, const AttachmentFormatDescriptor& src)
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
MTRenderPass::MTRenderPass(id<MTLDevice> device, const RenderPassDescriptor& desc) :
    sampleCount_ { GetMTRenderPassSampleCount(device, desc.samples) }
{
    const auto numColorAttachments = NumEnabledColorAttachments(desc);
    LLGL_ASSERT(numColorAttachments <= LLGL_MAX_NUM_COLOR_ATTACHMENTS);
    colorAttachments_.resize(numColorAttachments);
    for_range(i, numColorAttachments)
        ConvertAttachmentFormat(colorAttachments_[i], desc.colorAttachments[i]);
    ConvertAttachmentFormat(depthAttachment_, desc.depthAttachment);
    ConvertAttachmentFormat(stencilAttachment_, desc.stencilAttachment);
}

// Default initializer
MTRenderPass::MTRenderPass(id<MTLDevice> device, const RenderTargetDescriptor& desc) :
    sampleCount_ { GetMTRenderPassSampleCount(device, desc.samples) }
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
        const std::uint32_t numColorAttachments = NumActiveColorAttachments(desc);
        colorAttachments_.resize(numColorAttachments);
        for_range(i, numColorAttachments)
        {
            const auto& attachment = desc.colorAttachments[i];
            const Format format = GetAttachmentFormat(attachment);
            if (IsColorFormat(format))
                colorAttachments_[i] = MakeMTAttachmentFormat(format);
            else
                LLGL_TRAP("invalid format for render-pass color attachment: %s", ToString(format));
        }

        if (IsAttachmentEnabled(desc.depthStencilAttachment))
        {
            const auto& attachment = desc.depthStencilAttachment;
            const Format format = GetAttachmentFormat(attachment);
            if (IsDepthAndStencilFormat(format))
                depthAttachment_ = stencilAttachment_ = MakeMTAttachmentFormat(format);
            else if (IsDepthFormat(format))
                depthAttachment_ = MakeMTAttachmentFormat(format);
            else if (IsStencilFormat(format))
                stencilAttachment_ = MakeMTAttachmentFormat(format);
            else
                LLGL_TRAP("invalid format for render-pass depth-stencil attachment: %s", ToString(format));
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
        #endif
        return MTLPixelFormatDepth32Float_Stencil8;
    }
    else
    {
        if (@available(iOS 13.0, *))
        {
            if (depthBits == 16)
                return MTLPixelFormatDepth16Unorm;
        }
        return MTLPixelFormatDepth32Float;
    }
}

// Swap-chain initializer
MTRenderPass::MTRenderPass(id<MTLDevice> device, const SwapChainDescriptor& desc) :
    sampleCount_ { GetMTRenderPassSampleCount(device, desc.samples) }
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

static void SetMTColorAttachmentClearValue(
    MTLRenderPassColorAttachmentDescriptor* dst,
    const MTAttachmentFormat&               fmt,
    const ClearValue&                       clearValue)
{
    dst.loadAction  = fmt.loadAction;
    dst.clearColor  = MTTypes::ToMTLClearColor(clearValue.color);
}

static void SetMTDepthAttachmentClearValue(
    MTLRenderPassDepthAttachmentDescriptor* dst,
    const MTAttachmentFormat&               fmt,
    const ClearValue&                       clearValue)
{
    dst.loadAction  = fmt.loadAction;
    dst.clearDepth  = clearValue.depth;
}

static void SetMTStencilAttachmentClearValue(
    MTLRenderPassStencilAttachmentDescriptor* dst,
    const MTAttachmentFormat&               fmt,
    const ClearValue&                       clearValue)
{
    dst.loadAction      = fmt.loadAction;
    dst.clearStencil    = clearValue.stencil;
}

std::uint32_t MTRenderPass::UpdateNativeRenderPass(
    MTLRenderPassDescriptor*    nativeRenderPass,
    std::uint32_t               numClearValues,
    const ClearValue*           clearValues) const
{
    if (nativeRenderPass == nullptr)
        return 0;

    /* Update clear value for color attachments */
    std::uint32_t clearValueIndex = 0;

    const ClearValue defaultClearValue;

    for_range(i, colorAttachments_.size())
    {
        const MTAttachmentFormat& attachment = colorAttachments_[i];

        if (attachment.loadAction != MTLLoadActionDontCare)
        {
            nativeRenderPass.colorAttachments[i].loadAction = attachment.loadAction;
            //nativeRenderPass.colorAttachments[i].storeAction = attachment.storeAction;

            /* Clear color attachment with input or default value */
            if (attachment.loadAction == MTLLoadActionClear)
            {
                if (clearValueIndex < numClearValues)
                {
                    nativeRenderPass.colorAttachments[i].clearColor = MTTypes::ToMTLClearColor(clearValues[clearValueIndex].color);
                    ++clearValueIndex;
                }
                else
                    nativeRenderPass.colorAttachments[i].clearColor = MTTypes::ToMTLClearColor(defaultClearValue.color);
            }
        }
    }

    /* Update clear value for depth attachment */
    std::uint32_t numDepthStencilAttachments = 0;

    if (depthAttachment_.loadAction != MTLLoadActionDontCare)
    {
        nativeRenderPass.depthAttachment.loadAction = depthAttachment_.loadAction;
        //nativeRenderPass.depthAttachment.storeAction = depthAttachment_.storeAction;

        if (depthAttachment_.loadAction == MTLLoadActionClear)
        {
            /* Clear depth attachment with input or default value */
            if (clearValueIndex < numClearValues)
            {
                nativeRenderPass.depthAttachment.clearDepth = clearValues[clearValueIndex].depth;
                numDepthStencilAttachments = 1;
            }
            else
                nativeRenderPass.depthAttachment.clearDepth = defaultClearValue.depth;
        }
    }

    /* Update clear value for stencil attachment */
    if (stencilAttachment_.loadAction != MTLLoadActionDontCare)
    {
        nativeRenderPass.stencilAttachment.loadAction = stencilAttachment_.loadAction;
        //nativeRenderPass.stencilAttachment.storeAction = stencilAttachment_.storeAction;

        if (stencilAttachment_.loadAction == MTLLoadActionClear)
        {
            /* Clear stencil attachment with input or default value */
            if (clearValueIndex < numClearValues)
            {
                nativeRenderPass.stencilAttachment.clearStencil = clearValues[clearValueIndex].stencil;
                numDepthStencilAttachments = 1;
            }
            else
                nativeRenderPass.stencilAttachment.clearStencil = defaultClearValue.stencil;
        }
    }

    return (clearValueIndex + numDepthStencilAttachments);
}


} // /namespace LLGL



// ================================================================================
