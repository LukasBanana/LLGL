/*
 * D3D12Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Texture.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../D3D12Types.h"


namespace LLGL
{


static D3D12_RESOURCE_DIMENSION GetResourceDimension(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        /* pass */
        case TextureType::Texture1DArray:   return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case TextureType::Texture2D:        /* pass */
        case TextureType::Texture2DArray:   /* pass */
        case TextureType::TextureCube:      /* pass */
        case TextureType::TextureCubeArray: /* pass */
        case TextureType::Texture2DMS:      /* pass */
        case TextureType::Texture2DMSArray: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case TextureType::Texture3D:        return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    }
    DXTypes::MapFailed("TextureType", "D3D12_RESOURCE_DIMENSION");
}

D3D12Texture::D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc) :
    Texture { desc.type }
{
    /* Setup resource descriptor by texture descriptor */
    D3D12_RESOURCE_DESC resDesc;
    {
        resDesc.Dimension           = GetResourceDimension(desc.type);
        resDesc.Alignment           = 0;
        resDesc.MipLevels           = 1;
        resDesc.Format              = D3D12Types::Map(desc.format);
        resDesc.SampleDesc.Count    = 1;
        resDesc.SampleDesc.Quality  = 0;
        resDesc.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        resDesc.Flags               = D3D12_RESOURCE_FLAG_NONE;

        switch (resDesc.Dimension)
        {
            case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                resDesc.Width               = desc.texture1D.width;
                resDesc.Height              = 1;
                resDesc.DepthOrArraySize    = desc.texture1D.layers;
                break;

            case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                resDesc.Width               = desc.texture2D.width;
                resDesc.Height              = desc.texture2D.height;
                resDesc.DepthOrArraySize    = desc.texture2D.layers;
                break;

            case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
                resDesc.Width               = desc.texture3D.width;
                resDesc.Height              = desc.texture3D.height;
                resDesc.DepthOrArraySize    = desc.texture3D.depth;
                break;
        }
    }

    /* Create hardware resource */
    CreateResource(device, resDesc);
}

Gs::Vector3ui D3D12Texture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    Gs::Vector3ui size;

    //todo...

    return size;
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


/*
 * ======= Private: =======
 */

void D3D12Texture::CreateResource(
    ID3D12Device* device, const D3D12_RESOURCE_DESC& desc, const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc)
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
        srvHeapDesc.Type            = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        srvHeapDesc.NumDescriptors  = 1;
        srvHeapDesc.Flags           = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        srvHeapDesc.NodeMask        = 0;
    }
    hr = device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(descHeap_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap for texture");

    /* Create SRV in the descriptor heap for the texture */
    device->CreateShaderResourceView(resource_.Get(), srvDesc, descHeap_->GetCPUDescriptorHandleForHeapStart());
}


} // /namespace LLGL



// ================================================================================
