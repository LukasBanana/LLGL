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
#include <LLGL/Canvas.h>
#include <LLGL/Platform/Platform.h>
#include "RenderState/MTRenderPass.h"


@interface MTSwapChainViewDelegate : NSObject <MTKViewDelegate>
@end


namespace LLGL
{


class MTSwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        MTSwapChain(
            id<MTLDevice>                   device,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface,
            const RendererInfo&             rendererInfo
        );

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

        // Allocates the MetalKit view and initializes it with the specified surface.
        MTKView* AllocMTKViewAndInitWithSurface(id<MTLDevice> device, Surface& surface);

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        MTKView*                    view_                       = nullptr;
        MTSwapChainViewDelegate*    viewDelegate_               = nullptr;

        MTLRenderPassDescriptor*    nativeMutableRenderPass_    = nullptr; // Cannot be id<>
        MTRenderPass                renderPass_;

};


} // /namespace LLGL


#endif



// ================================================================================
