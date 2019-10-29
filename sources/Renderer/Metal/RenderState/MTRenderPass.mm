/*
 * MTRenderPass.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderPass.h"
#include <LLGL/RenderPassFlags.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/RenderContextFlags.h>
#include <LLGL/Platform/Platform.h>
#include "../MTTypes.h"
#include "../Texture/MTTexture.h"
#include "../../CheckedCast.h"
#include "../../TextureUtils.h"


namespace LLGL
{


static void Convert(MTAttachmentFormat& dst, const AttachmentFormatDescriptor& src)
{
    dst.pixelFormat = MTTypes::ToMTLPixelFormat(src.format);
    dst.loadAction  = MTTypes::ToMTLLoadAction(src.loadOp);
    dst.storeAction = MTTypes::ToMTLStoreAction(src.storeOp);
}

static MTAttachmentFormat MakeMTAttachmentFormat(
    const AttachmentDescriptor& desc,
    MTLPixelFormat              defaultPixelFormat = MTLPixelFormatInvalid)
{
    MTAttachmentFormat fmt;
    {
        /* Set pixel format for attachment */
        if (auto texture = desc.texture)
        {
            auto textureMT = LLGL_CAST(const MTTexture*, texture);
            fmt.pixelFormat = [textureMT->GetNative() pixelFormat];
        }
        else
            fmt.pixelFormat = defaultPixelFormat;

        /* Set default load and store actions */
        fmt.loadAction  = MTLLoadActionDontCare;
        fmt.storeAction = MTLStoreActionStore;
    }
    return fmt;
}

// Initializer when a custom render pass is created
MTRenderPass::MTRenderPass(const RenderPassDescriptor& desc) :
    sampleCount_ { GetClampedSamples(desc.samples) }
{
    colorAttachments_.resize(desc.colorAttachments.size());
    for (std::size_t i = 0; i < colorAttachments_.size(); ++i)
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
            switch (attachment.type)
            {
                case AttachmentType::Color:
                    colorAttachments_.push_back(MakeMTAttachmentFormat(attachment));
                    break;
                case AttachmentType::Depth:
                    depthAttachment_    = MakeMTAttachmentFormat(attachment, MTLPixelFormatDepth32Float);
                    break;
                case AttachmentType::DepthStencil:
                    depthAttachment_    = MakeMTAttachmentFormat(attachment, MTLPixelFormatDepth32Float_Stencil8);
                    stencilAttachment_  = MakeMTAttachmentFormat(attachment, MTLPixelFormatDepth32Float_Stencil8);
                    break;
                case AttachmentType::Stencil:
                    stencilAttachment_  = MakeMTAttachmentFormat(attachment, MTLPixelFormatStencil8);
                    break;
            }
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

static MTLPixelFormat GetDepthStencilMTLPixelFormat(int depthBits, int stencilBits)
{
    #if 0 //TODO: graphics pipeline must get the correct type from the render context
    if (depthBits > 0 && stencilBits > 0)
    {
        if (depthBits == 32)
            return MTLPixelFormatDepth32Float_Stencil8;
    }
    else if (depthBits > 0)
    {
        if (depthBits == 32)
            return MTLPixelFormatDepth32Float;
        else if (depthBits == 16)
            return MTLPixelFormatDepth16Unorm;
    }
    else if (stencilBits > 0)
        return MTLPixelFormatStencil8;
    return MTLPixelFormatDepth24Unorm_Stencil8;
    #else
    return MTLPixelFormatDepth32Float_Stencil8;
    #endif
}

// RenderContext initializer
MTRenderPass::MTRenderPass(const RenderContextDescriptor& desc) :
    sampleCount_ { GetClampedSamples(desc.samples) }
{
    const MTLPixelFormat colorFormat        = GetColorMTLPixelFormat(desc.videoMode.colorBits);
    const MTLPixelFormat depthStencilFormat = GetDepthStencilMTLPixelFormat(desc.videoMode.depthBits, desc.videoMode.stencilBits);

    colorAttachments_ = { MakeDefaultMTAttachmentFormat(colorFormat) };

    if (desc.videoMode.depthBits > 0)
        depthAttachment_ = MakeDefaultMTAttachmentFormat(depthStencilFormat);
    if (desc.videoMode.stencilBits > 0)
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
