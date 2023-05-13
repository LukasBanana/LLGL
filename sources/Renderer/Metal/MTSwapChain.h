/*
 * MTSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        std::uint32_t GetCurrentSwapIndex() const override;
        std::uint32_t GetNumSwapBuffers() const override;
        std::uint32_t GetSamples() const override;

        Format GetColorFormat() const override;
        Format GetDepthStencilFormat() const override;

        const RenderPass* GetRenderPass() const override;

        bool SetVsyncInterval(std::uint32_t vsyncInterval) override;

    public:

        // Updates the native render pass descriptor with the specified clear values. Returns null on failure.
        MTLRenderPassDescriptor* GetAndUpdateNativeRenderPass(
            const MTRenderPass& renderPass,
            std::uint32_t       numClearValues,
            const ClearValue*   clearValues
        );

        // Returns the native MTKView object.
        inline MTKView* GetMTKView() const
        {
            return view_;
        }

        // Returns the native render pass descriptor <MTLRenderPassDescriptor>.
        inline MTLRenderPassDescriptor* GetNativeRenderPass() const
        {
            return view_.currentRenderPassDescriptor;
        }

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        MTKView*                    view_                       = nullptr;
        #ifdef LLGL_OS_IOS
        CAMetalLayer*               metalLayer_                 = nullptr;
        #endif

        MTLRenderPassDescriptor*    nativeMutableRenderPass_    = nullptr; // Cannot be id<>
        MTRenderPass                renderPass_;

};


} // /namespace LLGL


#endif



// ================================================================================
