/*
 * D3D9SwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_SWAP_CHAIN_H
#define LLGL_D3D9_SWAP_CHAIN_H


#include <LLGL/SwapChain.h>
#include <string>
#include "Direct3D9.h"
#include "../DXCommon/ComPtr.h"
#include "../../Core/Assertion.h"


namespace LLGL
{


class D3D9SwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9SwapChain(
            IDirect3DDevice9*               device,
            const SwapChainDescriptor&      desc,
            const std::shared_ptr<Surface>& surface,
            const RendererInfo&             rendererInfo
        );

        inline UINT GetNumBackBuffers() const
        {
            return numBackBuffers_;
        }

        inline IDirect3DSurface9* GetBackBufferSurface(UINT index) const
        {
            LLGL_ASSERT(index < LLGL_ARRAY_LENGTH(backBuffers_));
            return backBuffers_[index].Get();
        }

        inline IDirect3DSurface9* GetDepthStencilSurface() const
        {
            return depthStencil_.Get();
        }

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

    private:

        D3DMULTISAMPLE_TYPE GetMultiSampleType() const;

        void CreateResolutionDependentResources(const Extent2D& resolution);

    private:

        IDirect3DDevice9*           device_             = nullptr;
        ComPtr<IDirect3DSwapChain9> swapChain_;

        UINT                        maxNumBackBuffers_  = 0;
        UINT                        numBackBuffers_     = 1;
        ComPtr<IDirect3DSurface9>   backBuffers_[2];

        ComPtr<IDirect3DSurface9>   depthStencil_;

        std::string                 label_;
        std::uint32_t               samples_            = 1;
        Format                      colorFormat_        = Format::Undefined;
        Format                      depthStencilFormat_ = Format::Undefined;
        std::uint32_t               vsyncInterval_      = 0;
        const RenderPass*           renderPass_         = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
