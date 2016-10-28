/*
 * D3D12Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Texture.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12Texture::D3D12Texture(const TextureType type) :
    Texture( type )
{
}

Gs::Vector3ui D3D12Texture::QueryMipLevelSize(unsigned int mipLevel) const
{
    Gs::Vector3ui size;

    //todo...

    return size;
}

void D3D12Texture::CreateResource(
    ID3D12Device* device, const D3D12_RESOURCE_DESC& desc, const D3D12_SHADER_RESOURCE_VIEW_DESC& srvDesc)
{
    /* Create hardware resource for the texture */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &desc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(resource_.ReleaseAndGetAddressOf())
    );
    DXThrowIfFailed(hr, "failed to create D3D12 committed resource for texture");

    /* Create descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
    {
        srvHeapDesc.NumDescriptors  = 1;
        srvHeapDesc.Type            = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.Flags           = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        srvHeapDesc.NodeMask        = 0;
    }
    hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(descHeap_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap for texture");

    /* Create SRV in the descriptor heap for the texture */
    device->CreateShaderResourceView(resource_.Get(), &srvDesc, descHeap_->GetCPUDescriptorHandleForHeapStart());
}

void D3D12Texture::UpdateSubresource(
    ID3D12Device* device, ID3D12GraphicsCommandList* commandList, ComPtr<ID3D12Resource>& uploadBuffer, D3D12_SUBRESOURCE_DATA& subresourceData)
{
    /* Create the GPU upload buffer */
    auto uploadBufferSize = GetRequiredIntermediateSize(resource_.Get(), 0, 1);

    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
    );

    UpdateSubresources(commandList, resource_.Get(), uploadBuffer.Get(), 0, 0, 1, &subresourceData);

    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource_.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );

    commandList->ResourceBarrier(1, &resourceBarrier);
}


} // /namespace LLGL



// ================================================================================
