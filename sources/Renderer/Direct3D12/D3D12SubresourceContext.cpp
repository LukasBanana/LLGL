/*
 * D3D12SubresourceContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12SubresourceContext.h"
#include "../../Core/Assertion.h"
#include "../DXCommon/DXCore.h"
#include "D3DX12/d3dx12.h"


namespace LLGL
{


D3D12SubresourceContext::D3D12SubresourceContext(D3D12CommandContext& commandContext) :
    commandContext_ { commandContext }
{
}

D3D12SubresourceContext::~D3D12SubresourceContext()
{
    commandContext_.Finish(true);
}

ID3D12Resource* D3D12SubresourceContext::CreateUploadBuffer(UINT64 size, D3D12_RESOURCE_STATES initialState)
{
    /* Create buffer resource in upload heap */
    ComPtr<ID3D12Resource> resource;
    HRESULT hr = GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(size),
        initialState,
        nullptr,
        IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for subresource upload buffer");
    return TakeAndGetNative(resource);
}

ID3D12Resource* D3D12SubresourceContext::CreateReadbackBuffer(UINT64 size, D3D12_RESOURCE_STATES initialState)
{
    /* Create buffer resource in readback heap */
    ComPtr<ID3D12Resource> resource;
    HRESULT hr = GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(size),
        initialState,
        nullptr,
        IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for subresource readback buffer");
    return TakeAndGetNative(resource);
}

ID3D12Resource* D3D12SubresourceContext::CreateTexture(const D3D12_RESOURCE_DESC& desc, D3D12_RESOURCE_STATES initialState)
{
    /* Create texture resource in default heap ready to be initialized with an upload buffer */
    ComPtr<ID3D12Resource> resource;
    HRESULT hr = GetDevice()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        initialState,
        nullptr,
        IID_PPV_ARGS(resource.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for subresource update texture");
    return TakeAndGetNative(resource);
}

ComPtr<ID3D12Resource> D3D12SubresourceContext::TakeResource()
{
    ComPtr<ID3D12Resource> resource = std::move(intermediateResources_.back());
    intermediateResources_.pop_back();
    return resource;
}

ID3D12Resource* D3D12SubresourceContext::TakeAndGetNative(ComPtr<ID3D12Resource>& resource)
{
    intermediateResources_.push_back(std::move(resource));
    return intermediateResources_.back().Get();
}


} // /namespace LLGL



// ================================================================================
