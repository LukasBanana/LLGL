/*
 * DbgSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DBG_SWAP_CHAIN_H
#define LLGL_DBG_SWAP_CHAIN_H


#include <LLGL/SwapChain.h>
#include "RenderState/DbgRenderPass.h"
#include <string>
#include <memory>
#include <functional>


namespace LLGL
{


class DbgBuffer;
class RenderingDebugger;

class DbgSwapChain final : public SwapChain
{

    public:

        using PresentCallback = std::function<void()>;

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        DbgSwapChain(SwapChain& instance, const SwapChainDescriptor& desc, const PresentCallback& presentCallback);

        // Notifies that the framebuffer will be put into a new render pass.
        void NotifyNextRenderPass(RenderingDebugger* debugger, const RenderPass* renderPass);

        // Notifies that the framebuffer has been used since the last render pass section.
        void NotifyFramebufferUsed();

    public:

        SwapChain&                  instance;
        const SwapChainDescriptor   desc;
        std::string                 label;

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        std::unique_ptr<DbgRenderPass>  renderPass_;
        PresentCallback                 presentCallback_;
        bool                            usedSinceRenderPass_ = true; // Has the framebuffer been read or presented since the last render pass section?

};


} // /namespace LLGL


#endif



// ================================================================================
