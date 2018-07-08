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
#include <algorithm>


namespace LLGL
{


#define _DEB_DISABLE_MIPS

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
    #ifndef _DEB_DISABLE_MIPS//TODO: mipmapping is not supported yet
    dst.MipLevels           = NumMipLevels(src);
    #else
    dst.MipLevels           = 1;
    #endif
    dst.Format              = D3D12Types::Map(src.format);
    dst.SampleDesc.Count    = 1;
    dst.SampleDesc.Quality  = 0;
    dst.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dst.Flags               = D3D12_RESOURCE_FLAG_NONE;

    switch (src.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            dst.Width               = src.extent.width;
            dst.Height              = 1;
            dst.DepthOrArraySize    = std::max(1u, src.arrayLayers);
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            dst.Width               = src.extent.width;
            dst.Height              = src.extent.height;
            dst.DepthOrArraySize    = std::max(1u, src.arrayLayers);
            break;

        case TextureType::Texture3D:
            dst.Width               = src.extent.width;
            dst.Height              = src.extent.height;
            dst.DepthOrArraySize    = src.extent.depth;
            break;

        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            dst.Width               = src.extent.width;
            dst.Height              = src.extent.height;
            dst.DepthOrArraySize    = std::max(1u, src.arrayLayers) * 6;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            dst.Width               = src.extent.width;
            dst.Height              = src.extent.height;
            dst.DepthOrArraySize    = src.arrayLayers;
            dst.SampleDesc.Count    = std::max(1u, src.samples);
            break;
    }
}

D3D12Texture::D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc) :
    Texture         { desc.type                    },
    format_         { D3D12Types::Map(desc.format) },
    #ifndef _DEB_DISABLE_MIPS//TODO: mipmapping not supported yet
    numMipLevels_   { NumMipLevels(desc)           },
    #else
    numMipLevels_   { 1                            },
    #endif
    numArrayLayers_ { desc.arrayLayers             }
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
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.arrayLayers     = desc.DepthOrArraySize;
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.extent.height   = desc.Height;
            texDesc.arrayLayers     = desc.DepthOrArraySize;
            break;

        case TextureType::Texture3D:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.extent.height   = desc.Height;
            texDesc.extent.depth    = desc.DepthOrArraySize;
            break;

        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.extent.height   = desc.Height;
            texDesc.arrayLayers     = desc.DepthOrArraySize / 6;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.extent.height   = desc.Height;
            texDesc.arrayLayers     = desc.DepthOrArraySize;
            texDesc.samples         = desc.SampleDesc.Count;
            texDesc.flags           |= TextureFlags::FixedSamples;
            break;
    }

    return texDesc;
}

void D3D12Texture::UpdateSubresource(
    ID3D12Device*               device,
    ID3D12GraphicsCommandList*  commandList,
    ComPtr<ID3D12Resource>&     uploadBuffer,
    D3D12_SUBRESOURCE_DATA&     subresourceData,
    UINT                        firstArrayLayer,
    UINT                        numArrayLayers)
{
    /* Clamp arguments */
    firstArrayLayer = std::min(firstArrayLayer, numArrayLayers_ - 1u);
    numArrayLayers  = std::min(numArrayLayers, numArrayLayers_ - firstArrayLayer);

    /* Create the GPU upload buffer */
    UINT64 uploadBufferSize     = GetRequiredIntermediateSize(resource_.Get(), 0, numArrayLayers);
    UINT64 uploadBufferOffset   = 0;

    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
    );

    /* Upload subresource for each array layer */
    for (UINT arrayLayer = 0; arrayLayer < numArrayLayers; ++arrayLayer)
    {
        /* Update subresource for current array layer */
        UINT subresourceIndex = D3D12CalcSubresource(0, firstArrayLayer + arrayLayer, 0, numMipLevels_, numArrayLayers_);

        UpdateSubresources(
            commandList,        // pCmdList
            resource_.Get(),    // pDestinationResource
            uploadBuffer.Get(), // pIntermediate
            uploadBufferOffset, // IntermediateOffset
            subresourceIndex,   // FirstSubresource
            1,                  // NumSubresources
            &subresourceData    // pSrcData
        );

        /* Move to next buffer region */
        subresourceData.pData = (reinterpret_cast<const std::int8_t*>(subresourceData.pData) + subresourceData.SlicePitch);
        uploadBufferOffset += subresourceData.SlicePitch;
    }

    /* Transition texture resource for shader access */
    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource_.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );

    commandList->ResourceBarrier(1, &resourceBarrier);
}

void D3D12Texture::CreateResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
    {
        srvDesc.Format                  = format_;
        srvDesc.ViewDimension           = D3D12Types::Map(GetType());
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

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
    device->CreateShaderResourceView(resource_.Get(), &srvDesc, cpuDescriptorHandle);
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
