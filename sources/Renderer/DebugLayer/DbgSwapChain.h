/*
 * DbgSwapChain.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_SWAP_CHAIN_H
#define LLGL_DBG_SWAP_CHAIN_H


#include <LLGL/SwapChain.h>


namespace LLGL
{


class DbgBuffer;

class DbgSwapChain final : public SwapChain
{

    public:

        void SetName(const char* name) override;

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

        const RenderPass* GetRenderPass() const override;

    public:

        DbgSwapChain(SwapChain& instance);

    public:

        SwapChain&  instance;
        std::string label;

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

};


} // /namespace LLGL


#endif



// ================================================================================
