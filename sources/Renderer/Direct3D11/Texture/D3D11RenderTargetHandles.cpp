/*
 * D3D11RenderTargetHandles.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11RenderTargetHandles.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <cstring>


namespace LLGL
{


static constexpr std::size_t g_RTVStride = (sizeof(ID3D11RenderTargetView*) + sizeof(D3D11BindingLocator*) + sizeof(D3D11SubresourceRange));
static constexpr std::size_t g_DSVStride = (sizeof(ID3D11DepthStencilView*) + sizeof(D3D11BindingLocator*));

static std::size_t GetRTHandleDataSize(UINT numRenderTargetViews, bool hasDepthStencilView)
{
    return (g_RTVStride * numRenderTargetViews + (hasDepthStencilView ? g_DSVStride : 0));
}

D3D11RenderTargetHandles::D3D11RenderTargetHandles() :
    numRenderTargetViews_ { 0 },
    hasDepthStencilView_  { 0 }
{
}

D3D11RenderTargetHandles::D3D11RenderTargetHandles(UINT numRenderTargetViews, bool hasDepthStencilView) :
    D3D11RenderTargetHandles {}
{
    Allocate(numRenderTargetViews, hasDepthStencilView);
}

D3D11RenderTargetHandles::~D3D11RenderTargetHandles()
{
    Release();
}

void D3D11RenderTargetHandles::Allocate(UINT numRenderTargetViews, bool hasDepthStencilView)
{
    /* Release previous buffer */
    Release();

    /* Allocate new buffer and zero-initialize */
    const std::size_t dataSize = GetRTHandleDataSize(numRenderTargetViews, hasDepthStencilView);
    data_ = new char[dataSize];
    std::memset(data_, 0, dataSize);

    /* Store new information */
    numRenderTargetViews_   = numRenderTargetViews;
    hasDepthStencilView_    = (hasDepthStencilView ? 1 : 0);
}

void D3D11RenderTargetHandles::Release()
{
    if (data_ != nullptr)
    {
        /* Release reosurce views and free buffer */
        ReleaseResourceViews();
        delete [] data_;
        data_ = nullptr;
    }
}

void D3D11RenderTargetHandles::Reset()
{
    if (data_ != nullptr)
    {
        /* Release resource views and zero initialize buffer */
        ReleaseResourceViews();
        const std::size_t dataSize = GetRTHandleDataSize(GetNumRenderTargetViews(), HasDepthStencilView());
        std::memset(data_, 0, dataSize);
    }
}

void D3D11RenderTargetHandles::SetRenderTargetView(UINT index, ID3D11RenderTargetView* rtv, D3D11BindingLocator* locator, const D3D11SubresourceRange& subresourceRange)
{
    LLGL_ASSERT(index < GetNumRenderTargetViews());
    LLGL_ASSERT_PTR(rtv);
    LLGL_ASSERT(GetRenderTargetViews()[index] == nullptr);

    GetMutableRenderTargetViews()[index] = rtv;
    GetMutableRenderTargetLocators()[index] = locator;
    GetMutableRenderTargetSubresourceRanges()[index] = subresourceRange;

    rtv->AddRef();
}

void D3D11RenderTargetHandles::SetDepthStencilView(ID3D11DepthStencilView* dsv, D3D11BindingLocator* locator)
{
    LLGL_ASSERT(HasDepthStencilView());
    LLGL_ASSERT(*GetAddressOfDepthStencilView() == nullptr);

    GetAddressOfDepthStencilView()[0] = dsv;
    GetAddressOfDepthStencilLocator()[0] = locator;

    dsv->AddRef();
}

ID3D11RenderTargetView* const * D3D11RenderTargetHandles::GetRenderTargetViews() const
{
    return reinterpret_cast<ID3D11RenderTargetView* const *>(data_ /* +dataOffset (0) */);
}

D3D11BindingLocator* const * D3D11RenderTargetHandles::GetRenderTargetLocators() const
{
    const std::size_t dataOffset = sizeof(ID3D11RenderTargetView*) * numRenderTargetViews_;
    return reinterpret_cast<D3D11BindingLocator* const *>(data_ + dataOffset);
}

const D3D11SubresourceRange* D3D11RenderTargetHandles::GetRenderTargetSubresourceRanges() const
{
    const std::size_t dataOffset = (sizeof(ID3D11RenderTargetView*) + sizeof(D3D11BindingLocator*)) * numRenderTargetViews_;
    return reinterpret_cast<const D3D11SubresourceRange*>(data_ + dataOffset);
}

ID3D11DepthStencilView* D3D11RenderTargetHandles::GetDepthStencilView() const
{
    return (HasDepthStencilView() ? *GetAddressOfDepthStencilView() : nullptr);
}

D3D11BindingLocator* D3D11RenderTargetHandles::GetDepthStencilLocator() const
{
    return (HasDepthStencilView() ? *GetAddressOfDepthStencilLocator() : nullptr);
}


/*
 * ======= Private: =======
 */

void D3D11RenderTargetHandles::ReleaseResourceViews()
{
    /* Explicitly release previous RTV objects */
    for (auto it = GetRenderTargetViews(), itEnd = it + GetNumRenderTargetViews(); it != itEnd; ++it)
    {
        ID3D11RenderTargetView* rtv = *it;
        rtv->Release();
    }

    /* Explicitly release previous DSV object */
    if (HasDepthStencilView())
    {
        ID3D11DepthStencilView* dsv = GetDepthStencilView();
        dsv->Release();
    }
}

ID3D11RenderTargetView** D3D11RenderTargetHandles::GetMutableRenderTargetViews()
{
    return reinterpret_cast<ID3D11RenderTargetView**>(data_ /* +dataOffset (0) */);
}

D3D11BindingLocator** D3D11RenderTargetHandles::GetMutableRenderTargetLocators()
{
    const std::size_t dataOffset = sizeof(ID3D11RenderTargetView*) * numRenderTargetViews_;
    return reinterpret_cast<D3D11BindingLocator**>(data_ + dataOffset);
}

D3D11SubresourceRange* D3D11RenderTargetHandles::GetMutableRenderTargetSubresourceRanges()
{
    const std::size_t dataOffset = (sizeof(ID3D11RenderTargetView*) + sizeof(D3D11BindingLocator*)) * numRenderTargetViews_;
    return reinterpret_cast<D3D11SubresourceRange*>(data_ + dataOffset);
}

ID3D11DepthStencilView** D3D11RenderTargetHandles::GetAddressOfDepthStencilView() const
{
    const std::size_t dataOffset = g_RTVStride * numRenderTargetViews_;
    return reinterpret_cast<ID3D11DepthStencilView**>(data_ + dataOffset);
}

D3D11BindingLocator** D3D11RenderTargetHandles::GetAddressOfDepthStencilLocator() const
{
    const std::size_t dataOffset = g_RTVStride * numRenderTargetViews_ + sizeof(ID3D11DepthStencilView*);
    return reinterpret_cast<D3D11BindingLocator**>(data_ + dataOffset);
}


} // /namespace LLGL



// ================================================================================
