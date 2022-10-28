/*
 * MTSwapChain.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_SWAP_CHAIN_H
#define LLGL_MT_SWAP_CHAIN_H


#import <MetalKit/MetalKit.h>

#include <LLGL/Window.h>
#include <LLGL/SwapChain.h>
#include <LLGL/Platform/Platform.h>
#include "RenderState/MTRenderPass.h"


namespace LLGL
{


class MTSwapChain final : public SwapChain
{

    public:

        /* ----- Common ----- */

        MTSwapChain(
            id<MTLDevice>                   device,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface
        );

        void Present() override;

        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

    public:

        // Returns the native MTKView object.
        inline MTKView* GetMTKView() const
        {
            return view_;
        }

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        MTKView*        view_       = nullptr;
        #ifdef LLGL_OS_IOS
        CAMetalLayer*   metalLayer_ = nullptr;
        #endif
        MTRenderPass    renderPass_;

};


} // /namespace LLGL


#endif



// ================================================================================
