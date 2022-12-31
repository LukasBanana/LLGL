/*
 * NullSwapChain.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_SWAP_CHAIN_H
#define LLGL_NULL_SWAP_CHAIN_H


#include <LLGL/SwapChain.h>
#include <string>


namespace LLGL
{


class NullSwapChain final : public SwapChain
{

    public:

        NullSwapChain(const SwapChainDescriptor& desc, const std::shared_ptr<Surface>& surface);

    public:

        void SetName(const char* name) override;

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

        const RenderPass* GetRenderPass() const override;

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        std::string         label_;
        std::uint32_t       samples_            = 1;
        Format              colorFormat_        = Format::Undefined;
        Format              depthStencilFormat_ = Format::Undefined;
        std::uint32_t       vsyncInterval_      = 0;
        const RenderPass*   renderPass_         = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
