/*
 * NullSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        #include <LLGL/Backend/SwapChain.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        NullSwapChain(
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface,
            const RendererInfo&             rendererInfo
        );

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
