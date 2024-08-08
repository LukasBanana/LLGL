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

DbgSwapChain::DbgSwapChain(SwapChain& instance, const SwapChainDescriptor& desc, const PresentCallback& presentCallback) :
    instance         { instance             },
    desc             { desc                 },
    label            { LLGL_DBG_LABEL(desc) },
    presentCallback_ { presentCallback      }
{
    ShareSurfaceAndConfig(instance);
    if (const auto* renderPass = instance.GetRenderPass())
        renderPass_ = MakeUnique<DbgRenderPass>(*renderPass, MakeRenderPassDesc(*this));
}

void DbgSwapChain::SetDebugName(const char* name)
{
    DbgSetObjectName(*this, name);
}

bool DbgSwapChain::IsPresentable() const
{
    return instance.IsPresentable();
}

void DbgSwapChain::Present()
{
    instance.Present();
    if (presentCallback_)
        presentCallback_();
    NotifyFramebufferUsed();
}

std::uint32_t DbgSwapChain::GetCurrentSwapIndex() const
{
    return instance.GetCurrentSwapIndex();
}

std::uint32_t DbgSwapChain::GetNumSwapBuffers() const
{
    return instance.GetNumSwapBuffers();
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

void DbgSwapChain::NotifyNextRenderPass(RenderingDebugger* debugger, const RenderPass* renderPass)
{
    if (!usedSinceRenderPass_ && debugger != nullptr)
    {
        auto* renderPassDbg = LLGL_CAST(const DbgRenderPass*, renderPass);
        if (!(renderPassDbg != nullptr && renderPassDbg->AnySwapChainAttachmentsLoaded(*this)))
        {
            const std::string swapChainLabel = (!label.empty() ? "swap-chain \"" + label + "\"" : "swap-chain");
            debugger->Warningf(
                WarningType::PointlessOperation,
                "%s has not been read or presented since last render pass, but new render pass does not load its previous content",
                swapChainLabel.c_str()
            );
        }
    }
    usedSinceRenderPass_ = false;
}

void DbgSwapChain::NotifyFramebufferUsed()
{
    usedSinceRenderPass_ = true;
}


} // /namespace LLGL



// ================================================================================
