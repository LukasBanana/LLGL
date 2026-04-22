/*
 * WGSwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SWAP_CHAIN_H
#define LLGL_WG_SWAP_CHAIN_H


#include <LLGL/SwapChain.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGRenderSystem;

struct WGFramebuffer
{
    WGPUTexture     colorTexture;
    WGPUTextureView colorTextureView;
    WGPUTexture     depthStencil;
    WGPUTextureView depthStencilView;
};

class WGSwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        WGSwapChain(WGRenderSystem& renderSystem, const SwapChainDescriptor& desc, const std::shared_ptr<Surface>& surface);
        ~WGSwapChain();

        // Creates a new transient texture view for the current back buffer. This is used for render pass attachments.
        WGFramebuffer GetCurrentFramebuffer();

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        void CreateWebGpuSurface(WGPUInstance instance, const SwapChainDescriptor& desc, Surface& surface);
        void UpdateWebGpuSurface(const Extent2D& resolution, WGPUPresentMode presentMode);

        void ReleaseTransientFramebuffer();

        void CreateDepthStencilTexture(const Extent2D& resolution);
        void ReleaseDepthStencilTexture();

    private:

        WGPUDevice          device_             = nullptr;
        WGPUSurface         surface_            = nullptr;
        WGPUTextureFormat   colorFormat_        = WGPUTextureFormat_BGRA8Unorm;
        WGPUTextureFormat   depthStencilFormat_ = WGPUTextureFormat_Undefined;
        WGPUPresentMode     presentMode_        = WGPUPresentMode_Mailbox;
        Extent2D            resolution_;

        WGFramebuffer       framebuffer_        = {};

};


} // /namespace LLGL


#endif



// ================================================================================
