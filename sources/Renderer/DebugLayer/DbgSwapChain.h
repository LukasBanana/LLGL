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


namespace LLGL
{


class DbgBuffer;

class DbgSwapChain final : public SwapChain
{

    public:

        void SetName(const char* name) override;

        void Present() override;

        std::uint32_t GetCurrentSwapIndex() const override;
        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

        const RenderPass* GetRenderPass() const override;

    public:

        DbgSwapChain(SwapChain& instance, const SwapChainDescriptor& desc);

    public:

        SwapChain&                  instance;
        const SwapChainDescriptor   desc;
        std::string                 label;

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        std::unique_ptr<DbgRenderPass> renderPass_;

};


} // /namespace LLGL


#endif



// ================================================================================
