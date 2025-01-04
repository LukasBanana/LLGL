/*
 * D3D11SwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_SWAP_CHAIN_H
#define LLGL_D3D11_SWAP_CHAIN_H

#include "../DXCommon/ComPtr.h"
#include "Texture/D3D11RenderTargetHandles.h"
#include <LLGL/SwapChain.h>
#include <d3d11.h>
#include <dxgi.h>

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3 || defined LLGL_OS_UWP
#   include <dxgi1_2.h>
#endif


namespace LLGL
{


struct NativeHandle;
class D3D11RenderSystem;

class D3D11SwapChain final : public SwapChain
{

    public:

        #include <LLGL/Backend/SwapChain.inl>

    public:

        D3D11SwapChain(
            IDXGIFactory*                       factory,
            const ComPtr<ID3D11Device>&         device,
            D3D11RenderSystem&                  renderSystem,
            const SwapChainDescriptor&          desc,
            const std::shared_ptr<Surface>&     surface
        );

        void SetDebugName(const char* name) override;

    public:

        // Copyies a subresource region from the backbuffer (color or depth-stencil) into the destination resource.
        HRESULT CopySubresourceRegion(
            ID3D11DeviceContext*    context,
            ID3D11Resource*         dstResource,
            UINT                    dstSubresource,
            UINT                    dstX,
            UINT                    dstY,
            UINT                    dstZ,
            const D3D11_BOX&        srcBox,
            DXGI_FORMAT             format
        );

        void ResolveSubresources(ID3D11DeviceContext* context);

        // Returns the handles container for the RTV and DSV objects.
        inline const D3D11RenderTargetHandles& GetRenderTargetHandles() const
        {
            return renderTargetHandles_;
        }

    private:

        bool ResizeBuffersPrimary(const Extent2D& resolution) override;

        bool SetPresentSyncInterval(UINT syncInterval);

        void CreateSwapChain(IDXGIFactory* factory, const Extent2D& resolution, std::uint32_t swapBuffers, std::uint32_t samples);
        #ifdef LLGL_OS_WIN32
        void CreateDXGISwapChain(IDXGIFactory* factory, const NativeHandle& wndHandle, const Extent2D& resolution, std::uint32_t swapBuffers, std::uint32_t samples);
        #endif
        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3 || defined LLGL_OS_UWP
        void CreateDXGISwapChain1(IDXGIFactory2* factory2, const NativeHandle& wndHandle, const Extent2D& resolution, std::uint32_t swapBuffers);
        #endif

        void CreateResolutionDependentResources();

        void StoreDebugNames(std::string (&debugNames)[5]);
        void RestoreDebugNames(const std::string (&debugNames)[5]);

    private:

        ComPtr<ID3D11Device>            device_;
        D3D11RenderSystem&              renderSystem_;

        ComPtr<IDXGISwapChain>          swapChain_;
        UINT                            swapChainInterval_      = 0;
        DXGI_SAMPLE_DESC                swapChainSampleDesc_    = { 1, 0 };
        DXGI_FORMAT                     colorFormat_            = DXGI_FORMAT_UNKNOWN;
        DXGI_FORMAT                     depthStencilFormat_     = DXGI_FORMAT_UNKNOWN;

        ComPtr<ID3D11Texture2D>         colorBuffer_;
        ComPtr<ID3D11Texture2D>         colorBufferMS_;
        D3D11BindingLocator             colorBufferLocator_;
        ComPtr<ID3D11Texture2D>         depthBuffer_;
        D3D11BindingLocator             depthBufferLocator_;
        D3D11RenderTargetHandles        renderTargetHandles_;

        bool                            hasDebugName_           = false;
        bool                            swapEffectFlip_         = false; // DXGI swap effect is DXGI_SWAP_EFFECT_FLIP_*
        bool                            tearingSupported_       = false;
        bool                            windowedMode_           = false;
        bool                            isPresentationDirty_    = false; // Has the back buffer been resized before it was presented again?

};


} // /namespace LLGL


#endif



// ================================================================================
