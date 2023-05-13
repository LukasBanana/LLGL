/*
 * DbgSwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgSwapChain.h"
#include "DbgCore.h"
#include "../../Core/CoreUtils.h"


namespace LLGL
{


static void SetDefaultAttachmentDesc(AttachmentFormatDescriptor& dst, Format format)
{
    dst.format  = format;
    dst.loadOp  = AttachmentLoadOp::Load;
    dst.storeOp = AttachmentStoreOp::Store;
}

static RenderPassDescriptor MakeRenderPassDesc(const SwapChain& swapChain)
{
    RenderPassDescriptor renderPassDesc;
    {
        /* Set first color attachment to swap-chain's color format */
        SetDefaultAttachmentDesc(renderPassDesc.colorAttachments[0], swapChain.GetColorFormat());

        /* Set depth- and stencil attachments to swap-chain's depth-stencil format  */
        const auto depthStencilFormat = swapChain.GetDepthStencilFormat();
        SetDefaultAttachmentDesc(renderPassDesc.depthAttachment, depthStencilFormat);
        SetDefaultAttachmentDesc(renderPassDesc.stencilAttachment, depthStencilFormat);
    }
    return renderPassDesc;
}

DbgSwapChain::DbgSwapChain(SwapChain& instance, const SwapChainDescriptor& desc) :
    instance { instance },
    desc     { desc     }
{
    ShareSurfaceAndConfig(instance);
    if (const auto* renderPass = instance.GetRenderPass())
        renderPass_ = MakeUnique<DbgRenderPass>(*renderPass, MakeRenderPassDesc(*this));
}

void DbgSwapChain::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

void DbgSwapChain::Present()
{
    instance.Present();
}

std::uint32_t DbgSwapChain::GetCurrentSwapIndex() const
{
    return instance.GetCurrentSwapIndex();
}

std::uint32_t DbgSwapChain::GetSamples() const
{
    return instance.GetSamples();
}

Format DbgSwapChain::GetColorFormat() const
{
    return instance.GetColorFormat();
}

Format DbgSwapChain::GetDepthStencilFormat() const
{
    return instance.GetDepthStencilFormat();
}

bool DbgSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return instance.SetVsyncInterval(vsyncInterval);
}

const RenderPass* DbgSwapChain::GetRenderPass() const
{
    return renderPass_.get();
}

bool DbgSwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    return instance.ResizeBuffers(resolution);
}


} // /namespace LLGL



// ================================================================================
