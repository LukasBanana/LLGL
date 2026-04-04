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

class WGSwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        WGSwapChain(const SwapChainDescriptor& desc, const std::shared_ptr<Surface>& surface, WGRenderSystem& renderSystem);

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        WGPUSurface surface_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
