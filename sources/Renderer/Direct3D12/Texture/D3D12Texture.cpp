/*
 * D3D12Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

static void Convert(D3D12_RESOURCE_DESC& dst, const TextureDescriptor& src)
{
    dst.Dimension           = GetResourceDimension(src.type);
    dst.Alignment           = 0;
    dst.MipLevels           = NumMipLevels(src);
    dst.Format              = D3D12Types::Map(src.format);
    dst.SampleDesc.Count    = 1;
    dst.SampleDesc.Quality  = 0;
    dst.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dst.Flags               = D3D12_RESOURCE_FLAG_NONE;

    switch (src.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            dst.Width               = src.texture1D.width;
            dst.Height              = 1;
            dst.DepthOrArraySize    = std::max(1u, src.texture1D.layers);
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            dst.Width               = src.texture2D.width;
            dst.Height              = src.texture2D.height;
            dst.DepthOrArraySize    = std::max(1u, src.texture2D.layers);
            break;

        case TextureType::Texture3D:
            dst.Width               = src.texture3D.width;
            dst.Height              = src.texture3D.height;
            dst.DepthOrArraySize    = src.texture3D.depth;
            break;

        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            dst.Width               = src.textureCube.width;
            dst.Height              = src.textureCube.height;
            dst.DepthOrArraySize    = std::max(1u, src.textureCube.layers) * 6;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            dst.Width               = src.texture2DMS.width;
            dst.Height              = src.texture2DMS.height;
            dst.DepthOrArraySize    = src.texture2DMS.layers;
            dst.SampleDesc.Count    = std::max(1u, src.texture2DMS.samples);
            break;
    }
}

D3D12Texture::D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc) :
    Texture         { desc.type                    },
    format_         { D3D12Types::Map(desc.format) },
    numMipLevels_   { NumMipLevels(desc)           },
    numArrayLayers_ { NumArrayLayers(desc)         }
{
    /* Setup resource descriptor by texture descriptor and create hardware resource */
    D3D12_RESOURCE_DESC descD3D;
    Convert(descD3D, desc);
    CreateResource(device, descD3D);
}

Extent3D D3D12Texture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    Extent3D size;

    auto desc = resource_->GetDesc();

    switch (desc.Dimension)
    {
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
        {
            if (mipLevel < desc.MipLevels)
            {
                size.width  = std::max(1u, static_cast<std::uint32_t>(desc.Width) >> mipLevel);
                size.height = desc.DepthOrArraySize;
                size.depth  = 1u;
            }
        }
        break;

        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
        {
            if (mipLevel < desc.MipLevels)
            {
                size.width  = std::max(1u, static_cast<std::uint32_t>(desc.Width) >> mipLevel);
                size.height = std::max(1u, desc.Height >> mipLevel);
                size.depth  = desc.DepthOrArraySize;
            }
        }
        break;

        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
        {
            if (mipLevel < desc.MipLevels)
            {
                size.width  = std::max(1u, static_cast<std::uint32_t>(desc.Width) >> mipLevel);
                size.height = std::max(1u, desc.Height >> mipLevel);
                size.depth  = std::max(1u, static_cast<std::uint32_t>(desc.DepthOrArraySize) >> mipLevel);
            }
        }
        break;

        default:
        break;
    }

    return size;
}

TextureDescriptor D3D12Texture::QueryDesc() const
{
    /* Setup texture descriptor */
    TextureDescriptor texDesc;

    auto desc = resource_->GetDesc();

    texDesc.type   = GetType();
    texDesc.format = D3D12Types::Unmap(desc.Format);
    texDesc.flags  = 0;

    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            texDesc.texture1D.width     = static_cast<std::uint32_t>(desc.Width);
            texDesc.texture1D.layers    = desc.DepthOrArraySize;
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::Texture3D:
            texDesc.texture3D.width     = static_cast<std::uint32_t>(desc.Width);
            texDesc.texture3D.height    = desc.Height;
            texDesc.texture3D.depth     = desc.DepthOrArraySize;
            break;

        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            texDesc.textureCube.width   = static_cast<std::uint32_t>(desc.Width);
            texDesc.textureCube.height  = desc.Height;
            texDesc.textureCube.layers  = desc.DepthOrArraySize / 6;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            texDesc.texture2DMS.width           = static_cast<std::uint32_t>(desc.Width);
            texDesc.texture2DMS.height          = desc.Height;
            texDesc.texture2DMS.layers          = desc.DepthOrArraySize;
            texDesc.texture2DMS.samples         = desc.SampleDesc.Count;
            texDesc.texture2DMS.fixedSamples    = true;
            break;
    }

    return texDesc;
}

void D3D12Texture::UpdateSubresource(
    ID3D12Device*               device,
    ID3D12GraphicsCommandList*  commandList,
    ComPtr<ID3D12Resource>&     uploadBuffer,
    D3D12_SUBRESOURCE_DATA&     subresourceData)
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

void D3D12Texture::CreateResourceView(ID3D12Device* device, ID3D12DescriptorHeap* descriptorHeap)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    {
        srvDesc.Format          = format_;
        srvDesc.ViewDimension   = D3D12Types::Map(GetType());

        /* Initialize texture swizzling (R,G,B,A) */
        srvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2,
            D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3
        );

        switch (srvDesc.ViewDimension)
        {
            case D3D12_SRV_DIMENSION_TEXTURE1D:
                srvDesc.Texture1D.MostDetailedMip               = 0;
                srvDesc.Texture1D.MipLevels                     = numMipLevels_;
                srvDesc.Texture1D.ResourceMinLODClamp           = 0.0f;
                break;

            case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
                srvDesc.Texture1DArray.MostDetailedMip          = 0;
                srvDesc.Texture1DArray.MipLevels                = numMipLevels_;
                srvDesc.Texture1DArray.FirstArraySlice          = 0;
                srvDesc.Texture1DArray.ArraySize                = numArrayLayers_;
                srvDesc.Texture1DArray.ResourceMinLODClamp      = 0.0f;
                break;

            case D3D12_SRV_DIMENSION_TEXTURE2D:
                srvDesc.Texture2D.MostDetailedMip               = 0;
                srvDesc.Texture2D.MipLevels                     = numMipLevels_;
                srvDesc.Texture2D.PlaneSlice                    = 0;
                srvDesc.Texture2D.ResourceMinLODClamp           = 0.0f;
                break;

            case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
                srvDesc.Texture2DArray.MostDetailedMip          = 0;
                srvDesc.Texture2DArray.MipLevels                = numMipLevels_;
                srvDesc.Texture2DArray.FirstArraySlice          = 0;
                srvDesc.Texture2DArray.ArraySize                = numArrayLayers_;
                srvDesc.Texture2DArray.PlaneSlice               = 0;
                srvDesc.Texture2DArray.ResourceMinLODClamp      = 0.0f;
                break;

            case D3D12_SRV_DIMENSION_TEXTURE2DMS:
                break;

            case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
                srvDesc.Texture2DMSArray.FirstArraySlice        = 0;
                srvDesc.Texture2DMSArray.ArraySize              = numArrayLayers_;
                break;

            case D3D12_SRV_DIMENSION_TEXTURE3D:
                srvDesc.Texture3D.MostDetailedMip               = 0;
                srvDesc.Texture3D.MipLevels                     = numMipLevels_;
                srvDesc.Texture3D.ResourceMinLODClamp           = 0.0f;
                break;

            case D3D12_SRV_DIMENSION_TEXTURECUBE:
                srvDesc.TextureCube.MostDetailedMip             = 0;
                srvDesc.TextureCube.MipLevels                   = numMipLevels_;
                srvDesc.TextureCube.ResourceMinLODClamp         = 0.0f;
                break;

            case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
                srvDesc.TextureCubeArray.MostDetailedMip        = 0;
                srvDesc.TextureCubeArray.MipLevels              = numMipLevels_;
                srvDesc.TextureCubeArray.First2DArrayFace       = 0;
                srvDesc.TextureCubeArray.NumCubes               = numArrayLayers_;
                srvDesc.TextureCubeArray.ResourceMinLODClamp    = 0.0f;
                break;

            default:
                break;
        }
    }
    device->CreateShaderResourceView(resource_.Get(), &srvDesc, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
}


/*
 * ======= Private: =======
 */

void D3D12Texture::CreateResource(ID3D12Device* device, const D3D12_RESOURCE_DESC& desc)
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
}


} // /namespace LLGL



// ================================================================================
