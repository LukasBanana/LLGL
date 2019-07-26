/*
 * D3D12Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Texture.h"
#include "../D3D12ObjectUtils.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"
#include "../D3D12Types.h"
#include <algorithm>


namespace LLGL
{


#define _DEB_DISABLE_MIPS

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
    CreateNativeTexture(device, desc);
}

void D3D12Texture::SetName(const char* name)
{
    D3D12SetObjectName(resource_.Get(), name);
}

Extent3D D3D12Texture::QueryMipExtent(std::uint32_t mipLevel) const
{
    Extent3D size;

    auto desc = resource_.native->GetDesc();

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

    auto desc = resource_.native->GetDesc();

    texDesc.type            = GetType();
    texDesc.bindFlags       = 0;
    texDesc.cpuAccessFlags  = 0;
    texDesc.miscFlags       = 0;
    texDesc.format          = D3D12Types::Unmap(desc.Format);

    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.arrayLayers     = desc.DepthOrArraySize;
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.extent.height   = desc.Height;
            texDesc.arrayLayers     = desc.DepthOrArraySize;
            break;

        case TextureType::Texture3D:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.extent.height   = desc.Height;
            texDesc.extent.depth    = desc.DepthOrArraySize;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            texDesc.extent.width    = static_cast<std::uint32_t>(desc.Width);
            texDesc.extent.height   = desc.Height;
            texDesc.arrayLayers     = desc.DepthOrArraySize;
            texDesc.samples         = desc.SampleDesc.Count;
            texDesc.miscFlags       |= MiscFlags::FixedSamples;
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
    UINT64 uploadBufferSize     = GetRequiredIntermediateSize(resource_.native.Get(), 0, numArrayLayers);
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
        UINT subresourceIndex = CalcSubresource(0, firstArrayLayer + arrayLayer);

        UpdateSubresources(
            commandList,            // pCmdList
            resource_.native.Get(), // pDestinationResource
            uploadBuffer.Get(),     // pIntermediate
            uploadBufferOffset,     // IntermediateOffset
            subresourceIndex,       // FirstSubresource
            1,                      // NumSubresources
            &subresourceData        // pSrcData
        );

        /* Move to next buffer region */
        subresourceData.pData = (reinterpret_cast<const std::int8_t*>(subresourceData.pData) + subresourceData.SlicePitch);
        uploadBufferOffset += subresourceData.SlicePitch;
    }

    /* Transition texture resource for shader access */
    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        resource_.native.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        resource_.usageState
    );

    commandList->ResourceBarrier(1, &resourceBarrier);
}

void D3D12Texture::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    CreateShaderResourceViewPrimary(
        device,
        D3D12Types::MapSrvDimension(GetType()),
        format_,
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        TextureSubresource{ 0, numArrayLayers_, 0, numMipLevels_ },
        cpuDescHandle
    );
}

void D3D12Texture::CreateShaderResourceView(ID3D12Device* device, const TextureViewDescriptor& desc, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    CreateShaderResourceViewPrimary(
        device,
        D3D12Types::MapSrvDimension(desc.type),
        D3D12Types::Map(desc.format),
        D3D12Types::Map(desc.swizzle),
        desc.subresource,
        cpuDescHandle
    );
}

//private
void D3D12Texture::CreateShaderResourceViewPrimary(
    ID3D12Device*               device,
    D3D12_SRV_DIMENSION         dimension,
    DXGI_FORMAT                 format,
    UINT                        componentMapping,
    const TextureSubresource&   subresource,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

    srvDesc.Format                  = format;
    srvDesc.ViewDimension           = dimension;
    srvDesc.Shader4ComponentMapping = componentMapping;

    switch (srvDesc.ViewDimension)
    {
        case D3D12_SRV_DIMENSION_TEXTURE1D:
            srvDesc.Texture1D.MostDetailedMip               = subresource.baseMipLevel;
            srvDesc.Texture1D.MipLevels                     = subresource.numMipLevels;
            srvDesc.Texture1D.ResourceMinLODClamp           = 0.0f;
            break;

        case D3D12_SRV_DIMENSION_TEXTURE1DARRAY:
            srvDesc.Texture1DArray.MostDetailedMip          = subresource.baseMipLevel;
            srvDesc.Texture1DArray.MipLevels                = subresource.numMipLevels;
            srvDesc.Texture1DArray.FirstArraySlice          = subresource.baseArrayLayer;
            srvDesc.Texture1DArray.ArraySize                = subresource.numArrayLayers;
            srvDesc.Texture1DArray.ResourceMinLODClamp      = 0.0f;
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2D:
            srvDesc.Texture2D.MostDetailedMip               = subresource.baseMipLevel;
            srvDesc.Texture2D.MipLevels                     = subresource.numMipLevels;
            srvDesc.Texture2D.PlaneSlice                    = 0;
            srvDesc.Texture2D.ResourceMinLODClamp           = 0.0f;
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2DARRAY:
            srvDesc.Texture2DArray.MostDetailedMip          = subresource.baseMipLevel;
            srvDesc.Texture2DArray.MipLevels                = subresource.numMipLevels;
            srvDesc.Texture2DArray.FirstArraySlice          = subresource.baseArrayLayer;
            srvDesc.Texture2DArray.ArraySize                = subresource.numArrayLayers;
            srvDesc.Texture2DArray.PlaneSlice               = 0;
            srvDesc.Texture2DArray.ResourceMinLODClamp      = 0.0f;
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2DMS:
            break;

        case D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY:
            srvDesc.Texture2DMSArray.FirstArraySlice        = subresource.baseArrayLayer;
            srvDesc.Texture2DMSArray.ArraySize              = subresource.numArrayLayers;
            break;

        case D3D12_SRV_DIMENSION_TEXTURE3D:
            srvDesc.Texture3D.MostDetailedMip               = subresource.baseMipLevel;
            srvDesc.Texture3D.MipLevels                     = subresource.numMipLevels;
            srvDesc.Texture3D.ResourceMinLODClamp           = 0.0f;
            break;

        case D3D12_SRV_DIMENSION_TEXTURECUBE:
            srvDesc.TextureCube.MostDetailedMip             = subresource.baseMipLevel;
            srvDesc.TextureCube.MipLevels                   = subresource.numMipLevels;
            srvDesc.TextureCube.ResourceMinLODClamp         = 0.0f;
            break;

        case D3D12_SRV_DIMENSION_TEXTURECUBEARRAY:
            srvDesc.TextureCubeArray.MostDetailedMip        = subresource.baseMipLevel;
            srvDesc.TextureCubeArray.MipLevels              = subresource.numMipLevels;
            srvDesc.TextureCubeArray.First2DArrayFace       = subresource.baseArrayLayer;
            srvDesc.TextureCubeArray.NumCubes               = subresource.numArrayLayers / 6;
            srvDesc.TextureCubeArray.ResourceMinLODClamp    = 0.0f;
            break;

        default:
            break;
    }

    device->CreateShaderResourceView(resource_.native.Get(), &srvDesc, cpuDescHandle);
}

UINT D3D12Texture::CalcSubresource(UINT mipLevel, UINT arrayLayer) const
{
    return D3D12CalcSubresource(mipLevel, arrayLayer, /*planeSlice: */0, numMipLevels_, numArrayLayers_);
}

UINT D3D12Texture::CalcSubresource(const TextureLocation& location) const
{
    /* Only include array layer in subresource calculation for array and cube texture types */
    switch (GetType())
    {
        case TextureType::Texture1DArray:
            return CalcSubresource(location.mipLevel, static_cast<UINT>(location.offset.y));
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::Texture2DMSArray:
        case TextureType::TextureCubeArray:
            return CalcSubresource(location.mipLevel, static_cast<UINT>(location.offset.z));
        default:
            return CalcSubresource(location.mipLevel, 0);
    }
}

D3D12_TEXTURE_COPY_LOCATION D3D12Texture::CalcCopyLocation(const TextureLocation& location) const
{
    D3D12_TEXTURE_COPY_LOCATION copyDesc;
    {
        copyDesc.pResource          = GetNative();
        copyDesc.Type               = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        copyDesc.SubresourceIndex   = CalcSubresource(location);
    }
    return copyDesc;
}

D3D12_BOX D3D12Texture::CalcRegion(const Offset3D& offset, const Extent3D& extent) const
{
    /* Ignore sub components of offset and extent if it's handled by the subresource index */
    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            return CD3DX12_BOX(
                offset.x,
                offset.x + static_cast<LONG>(extent.width)
            );
        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            return CD3DX12_BOX(
                offset.x,
                offset.y,
                offset.x + static_cast<LONG>(extent.width),
                offset.y + static_cast<LONG>(extent.height)
            );
        case TextureType::Texture3D:
            return CD3DX12_BOX(
                offset.x,
                offset.y,
                offset.z,
                offset.x + static_cast<LONG>(extent.width),
                offset.y + static_cast<LONG>(extent.height),
                offset.z + static_cast<LONG>(extent.depth)
            );
        default:
            return CD3DX12_BOX(0,0,0 , 0,0,0);
    }
}


/*
 * ======= Private: =======
 */

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_resource_flags
static D3D12_RESOURCE_FLAGS GetD3DTextureResourceFlags(const TextureDescriptor& desc)
{
    D3D12_RESOURCE_FLAGS flagsD3D = D3D12_RESOURCE_FLAG_NONE;

    if ((desc.bindFlags & BindFlags::ColorAttachment) != 0)
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    else if ((desc.bindFlags & BindFlags::DepthStencilAttachment) != 0)
        flagsD3D |= (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);

    if ((desc.bindFlags & BindFlags::SampleBuffer) == 0)
        flagsD3D |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;

    if ((desc.bindFlags & BindFlags::RWStorageBuffer) != 0)
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    return flagsD3D;
}

static void Convert(D3D12_RESOURCE_DESC& dst, const TextureDescriptor& src)
{
    dst.Dimension           = D3D12Types::MapResourceDimension(src.type);
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
    dst.Flags               = GetD3DTextureResourceFlags(src);

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
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            dst.Width               = src.extent.width;
            dst.Height              = src.extent.height;
            dst.DepthOrArraySize    = std::max(1u, src.arrayLayers);
            break;

        case TextureType::Texture3D:
            dst.Width               = src.extent.width;
            dst.Height              = src.extent.height;
            dst.DepthOrArraySize    = src.extent.depth;
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

void D3D12Texture::CreateNativeTexture(ID3D12Device* device, const TextureDescriptor& desc)
{
    /* Setup resource descriptor by texture descriptor and create hardware resource */
    D3D12_RESOURCE_DESC descD3D;
    Convert(descD3D, desc);

    /* Get optimal clear value (if specified) */
    bool useClearValue = ((desc.bindFlags & (BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment)) != 0);

    D3D12_CLEAR_VALUE optClearValue;
    if ((desc.bindFlags & BindFlags::ColorAttachment) != 0)
    {
        optClearValue.Format    = descD3D.Format;
        optClearValue.Color[0]  = desc.clearValue.color.r;
        optClearValue.Color[1]  = desc.clearValue.color.g;
        optClearValue.Color[2]  = desc.clearValue.color.b;
        optClearValue.Color[3]  = desc.clearValue.color.a;
    }
    else if ((desc.bindFlags & BindFlags::DepthStencilAttachment) != 0)
    {
        optClearValue.Format    = descD3D.Format;
        optClearValue.DepthStencil.Depth    = desc.clearValue.depth;
        optClearValue.DepthStencil.Stencil  = static_cast<UINT8>(desc.clearValue.stencil);
    }

    /* Create hardware resource for the texture */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &descD3D,
        D3D12_RESOURCE_STATE_COPY_DEST,
        (useClearValue ? &optClearValue : nullptr),
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 hardware texture");

    /* Determine resource usage */
    if ((desc.bindFlags & BindFlags::DepthStencilAttachment) != 0)
        resource_.usageState = D3D12_RESOURCE_STATE_DEPTH_READ;
    else
        resource_.usageState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}


} // /namespace LLGL



// ================================================================================
