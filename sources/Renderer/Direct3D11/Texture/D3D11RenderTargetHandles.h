/*
 * D3D11RenderTargetHandles.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_FRAMEBUFFER_VIEW_H
#define LLGL_D3D11_FRAMEBUFFER_VIEW_H


#include <LLGL/RenderTarget.h>
#include <LLGL/Container/DynamicArray.h>
#include "../RenderState/D3D11BindingLocator.h"
#include "../../DXCommon/ComPtr.h"
#include <vector>
#include <d3d11.h>


namespace LLGL
{


// Container class for render-target (RTV) and depth-stencil views (DSV) as well as their binding locators for the D3D11BindingTable.
class D3D11RenderTargetHandles final
{

    public:

        D3D11RenderTargetHandles();
        D3D11RenderTargetHandles(UINT numRenderTargetViews, bool hasDepthStencilView);
        ~D3D11RenderTargetHandles();

        D3D11RenderTargetHandles(const D3D11RenderTargetHandles&) = delete;
        D3D11RenderTargetHandles& operator = (const D3D11RenderTargetHandles&) = delete;

        void Allocate(UINT numRenderTargetViews, bool hasDepthStencilView);
        void Release();
        void Reset();

        // Sets the specified RTV with its binding locator and subresource range.
        void SetRenderTargetView(UINT index, ID3D11RenderTargetView* rtv, D3D11BindingLocator* locator = nullptr, const D3D11SubresourceRange& subresourceRange = { 0u, ~0u });

        // Sets the specified DSV with its binding locator.
        void SetDepthStencilView(ID3D11DepthStencilView* dsv, D3D11BindingLocator* locator = nullptr);

        // Returns the list of native render-target views (RTV).
        ID3D11RenderTargetView* const * GetRenderTargetViews() const;

        // Returns the list of render-target locators for the binding table.
        D3D11BindingLocator* const * GetRenderTargetLocators() const;

        // Returns the list of render-target subresource ranges for the binding table.
        const D3D11SubresourceRange* GetRenderTargetSubresourceRanges() const;

        // Returns the native depth-stencil view (DSV).
        ID3D11DepthStencilView* GetDepthStencilView() const;

        // Returns the list of depth-stencil locator for the binding table.
        D3D11BindingLocator* GetDepthStencilLocator() const;

        // Returns the number of render-target views.
        inline UINT GetNumRenderTargetViews() const
        {
            return numRenderTargetViews_;
        }

        // Returns true if this container has a depth-stencil view.
        inline bool HasDepthStencilView() const
        {
            return (hasDepthStencilView_ != 0);
        }

    private:

        void ReleaseResourceViews();

        ID3D11RenderTargetView** GetMutableRenderTargetViews();
        D3D11BindingLocator** GetMutableRenderTargetLocators();
        D3D11SubresourceRange* GetMutableRenderTargetSubresourceRanges();

        ID3D11DepthStencilView** GetAddressOfDepthStencilView() const;
        D3D11BindingLocator** GetAddressOfDepthStencilLocator() const;

    private:

        char*   data_                       = nullptr;
        UINT    numRenderTargetViews_ : 4; // Number of bits to store ( D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT )
        UINT    hasDepthStencilView_  : 1;

};


} // /namespace LLGL


#endif



// ================================================================================
