/*
 * D3D12Texture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Texture.h"
#include "../Command/D3D12CommandContext.h"
#include "../D3D12SubresourceContext.h"
#include "../D3D12ObjectUtils.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
#include "../Buffer/D3D12Buffer.h"
#include "../../DXCommon/DXCore.h"
#include "../../TextureUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Backend/Direct3D12/NativeHandle.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


D3D12Texture::D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc) :
    Texture         { desc.type, desc.bindFlags          },
    baseFormat_     { desc.format                        },
    format_         { DXTypes::ToDXGIFormat(desc.format) },
    numMipLevels_   { NumMipLevels(desc)                 },
    numArrayLayers_ { std::max(1u, desc.arrayLayers)     },
    extent_         { desc.extent                        }
{
    CreateNativeTexture(device, desc);

    if (SupportsGenerateMips())
        CreateMipDescHeap(device);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

bool D3D12Texture::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (auto* nativeHandleD3D = GetTypedNativeHandle<Direct3D12::ResourceNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleD3D->type                   = Direct3D12::ResourceNativeType::SamplerDescriptor;
        nativeHandleD3D->resource.resource      = resource_.Get();
        nativeHandleD3D->resource.resourceState = resource_.currentState;
        nativeHandleD3D->resource.resource->AddRef();
        return true;
    }
    return false;
}

void D3D12Texture::SetDebugName(const char* name)
{
    D3D12SetObjectName(resource_.Get(), name);
}

Extent3D D3D12Texture::GetMipExtent(std::uint32_t mipLevel) const
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

TextureDescriptor D3D12Texture::GetDesc() const
{
    /* Setup texture descriptor */
    TextureDescriptor texDesc;

    auto desc = resource_.native->GetDesc();

    texDesc.type        = GetType();
    texDesc.bindFlags   = GetBindFlags();
    texDesc.miscFlags   = 0;
    texDesc.format      = GetBaseFormat();
    texDesc.mipLevels   = desc.MipLevels;

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

Format D3D12Texture::GetFormat() const
{
    return GetBaseFormat();
}

SubresourceFootprint D3D12Texture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    SubresourceFootprint footprint;
    {
        ComPtr<ID3D12Device> device;
        HRESULT hr = resource_.native->GetDevice(IID_PPV_ARGS(device.GetAddressOf()));
        if (SUCCEEDED(hr))
        {
            UINT    subresourceIndex    = D3D12CalcSubresource(mipLevel, 0, 0, GetNumMipLevels(), GetNumArrayLayers());
            UINT64  totalSize           = 0;
            UINT    rows                = 0;
            UINT64  rowSize             = 0;

            D3D12_RESOURCE_DESC resourceDesc = resource_.native->GetDesc();

            D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedFootprint;
            device->GetCopyableFootprints(&resourceDesc, subresourceIndex, 1, 0, &placedFootprint, &rows, &rowSize, &totalSize);

            footprint.size          = totalSize;
            footprint.rowAlignment  = D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
            footprint.rowSize       = static_cast<std::uint32_t>(rowSize);
            footprint.rowStride     = placedFootprint.Footprint.RowPitch;
            footprint.layerSize     = (rows > 1 ? placedFootprint.Footprint.RowPitch * (rows - 1) + footprint.rowSize : footprint.rowSize * rows);
            footprint.layerStride   = placedFootprint.Footprint.RowPitch * rows;
        }
    }
    return footprint;
}

static D3D12_TEXTURE_COPY_LOCATION GetD3DTextureSubresourceLocation(ID3D12Resource* resource, UINT subresource)
{
    D3D12_TEXTURE_COPY_LOCATION copyDesc;
    {
        copyDesc.pResource          = resource;
        copyDesc.Type               = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        copyDesc.SubresourceIndex   = subresource;
    }
    return copyDesc;
}

static void ConvertD3DTextureExtent(D3D12_RESOURCE_DESC& outDesc, TextureType type, const Extent3D& extent, std::uint32_t arrayLayers)
{
    switch (type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            outDesc.Width               = extent.width;
            outDesc.Height              = 1;
            outDesc.DepthOrArraySize    = std::max(1u, arrayLayers);
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            outDesc.Width               = extent.width;
            outDesc.Height              = extent.height;
            outDesc.DepthOrArraySize    = std::max(1u, arrayLayers);
            break;

        case TextureType::Texture3D:
            outDesc.Width               = extent.width;
            outDesc.Height              = extent.height;
            outDesc.DepthOrArraySize    = extent.depth;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            outDesc.Width               = extent.width;
            outDesc.Height              = extent.height;
            outDesc.DepthOrArraySize    = arrayLayers;
            break;
    }
}

static void UpdateD3DTextureSubresource(
    ID3D12Resource*             dstTexture,
    D3D12SubresourceContext&    context,
    D3D12_SUBRESOURCE_DATA      subresourceData,
    UINT                        mipLevel,
    UINT                        firstArrayLayer,
    UINT                        numArrayLayers,
    UINT                        maxNumMipLevels,
    UINT                        maxNumArrayLayers)
{
    /* Clamp arguments */
    firstArrayLayer = std::min(firstArrayLayer, maxNumArrayLayers - 1u);
    numArrayLayers  = std::min(numArrayLayers, maxNumArrayLayers - firstArrayLayer);

    /* Create the GPU upload buffer */
    UINT64          srcBufferOffset             = 0;
    UINT64          srcBufferSubresourceSize    = GetAlignedSize<UINT64>(GetRequiredIntermediateSize(dstTexture, 0, 1), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
    ID3D12Resource* srcBuffer                   = context.CreateUploadBuffer(srcBufferSubresourceSize * numArrayLayers);

    /* Upload subresource for each array layer */
    for_subrange(arrayLayer, firstArrayLayer, firstArrayLayer + numArrayLayers)
    {
        /* Update subresource for current array layer */
        const UINT dstSubresource = D3D12CalcSubresource(mipLevel, arrayLayer, /*planeSlice:*/ 0, maxNumMipLevels, maxNumArrayLayers);

        UpdateSubresources<1>(
            /*pCmdList:*/               context.GetCommandList(),
            /*pDestinationResource:*/   dstTexture,
            /*pIntermediate:*/          srcBuffer,
            /*IntermediateOffset:*/     srcBufferOffset,
            /*FirstSubresource:*/       dstSubresource,
            /*NumSubresources:*/        1,
            /*pSrcData:*/               &subresourceData
        );

        /* Move to next buffer region */
        subresourceData.pData = (static_cast<const char*>(subresourceData.pData) + subresourceData.SlicePitch);
        srcBufferOffset += srcBufferSubresourceSize;
    }
}

void D3D12Texture::UpdateSubresource(
    D3D12SubresourceContext&        context,
    const D3D12_SUBRESOURCE_DATA&   subresourceData,
    const TextureSubresource&       subresource)
{
    LLGL_ASSERT(subresource.numMipLevels == 1);

    /* Update native resource of this texture with the specified subresource data */
    UpdateD3DTextureSubresource(
        /*dstTexture:*/         resource_.native.Get(),
        /*context:*/            context,
        /*subresourceData:*/    subresourceData,
        /*mipLevel:*/           subresource.baseMipLevel,
        /*firstArrayLayer:*/    subresource.baseArrayLayer,
        /*numArrayLayers:*/     subresource.numArrayLayers,
        /*maxNumMipLevels:*/    GetNumMipLevels(),
        /*maxNumArrayLayers:*/  GetNumArrayLayers()
    );
}

void D3D12Texture::UpdateSubresourceRegion(
    D3D12SubresourceContext&        context,
    const D3D12_SUBRESOURCE_DATA&   subresourceData,
    const TextureRegion&            region)
{
    const auto& subresource = region.subresource;
    LLGL_ASSERT(subresource.numMipLevels == 1);

    /* Create intermediate texture with region size */
    D3D12_RESOURCE_DESC texDesc = resource_.native->GetDesc();
    {
        ConvertD3DTextureExtent(texDesc, GetType(), region.extent, subresource.numArrayLayers);
        texDesc.MipLevels = 1;
    }
    ID3D12Resource* intermediateTexture = context.CreateTexture(texDesc);

    /* Update native resource of this texture with the specified subresource data */
    UpdateD3DTextureSubresource(
        /*dstTexture:*/         intermediateTexture,
        /*context:*/            context,
        /*subresourceData:*/    subresourceData,
        /*mipLevel:*/           0,
        /*firstArrayLayer:*/    0,
        /*numArrayLayers:*/     subresource.numArrayLayers,
        /*maxNumMipLevels:*/    1,
        /*maxNumArrayLayers:*/  subresource.numArrayLayers
    );

    const Offset3D  dstOffset   = CalcTextureOffset(GetType(), region.offset);
    const Extent3D  srcExtent   = CalcTextureExtent(GetType(), region.extent);
    const D3D12_BOX srcBox      = CalcRegion(Offset3D{}, srcExtent);

    /* Transition texture resource for shader access */
    context.GetCommandContext().TransitionBarrier(intermediateTexture, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
    context.GetCommandContext().TransitionResource(GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, true);

    for_range(arrayLayer, subresource.numArrayLayers)
    {
        const UINT                          dstSubresource = CalcSubresource(subresource.baseMipLevel, subresource.baseArrayLayer + arrayLayer);
        const D3D12_TEXTURE_COPY_LOCATION   dstLocationD3D = GetD3DTextureSubresourceLocation(GetNative(), dstSubresource);

        const UINT                          srcSubresource = D3D12CalcSubresource(0, arrayLayer, 0, 1, subresource.numArrayLayers);
        const D3D12_TEXTURE_COPY_LOCATION   srcLocationD3D = GetD3DTextureSubresourceLocation(intermediateTexture, srcSubresource);

        context.GetCommandList()->CopyTextureRegion(
            /*pDst:*/       &dstLocationD3D,
            /*DstX:*/       static_cast<UINT>(dstOffset.x),
            /*DstY:*/       static_cast<UINT>(dstOffset.y),
            /*DstZ:*/       static_cast<UINT>(dstOffset.z),
            /*pSrc:*/       &srcLocationD3D,
            /*pSrcBox:*/    &srcBox
        );
    }
}

// Returns the memory footprint of a texture with row alignment
static void GetMemoryFootprintWithAlignment(
    const Format    format,
    const Extent3D& extent,
    UINT            numArrayLayers,
    UINT&           rowStride,
    UINT&           layerSize,
    UINT&           layerStride,
    UINT64&         outBufferSize)
{
    const FormatAttributes& formatAttribs   = GetFormatAttribs(format);
    const UINT              rowSize         = extent.width * formatAttribs.bitSize / (8u * formatAttribs.blockWidth);

    layerSize       = rowSize * (extent.height / formatAttribs.blockHeight);
    rowStride       = GetAlignedSize<UINT>(rowSize, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
    layerStride     = GetAlignedSize<UINT>(rowStride * (extent.height / formatAttribs.blockHeight), D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
    outBufferSize   = layerStride * extent.depth * numArrayLayers;
}

void D3D12Texture::CreateSubresourceCopyAsReadbackBuffer(
    D3D12SubresourceContext&    context,
    const TextureRegion&        region,
    UINT                        plane,
    UINT&                       outRowStride,
    UINT&                       outLayerSize,
    UINT&                       outLayerStride)
{
    auto GetFormatForSubresourceCopy = [](Format format, UINT plane) -> Format
    {
        /* D3D12_PLACED_SUBRESOURCE_FOOTPRINT::Format must be DXGI_FORMAT_R32_TYPELESS for depth and DXGI_FORMAT_R8_TYPELESS for stencil */
        return (IsDepthOrStencilFormat(format) ? (plane == 0 ? Format::R32Float : Format::R8UInt) : format);
    };

    /* Determine required buffer size for texture subresource */
    const Offset3D  offset = CalcTextureOffset(GetType(), region.offset, 0);
    const Extent3D  extent = CalcTextureExtent(GetType(), region.extent, 1);
    const Format    format = GetFormatForSubresourceCopy(GetFormat(), plane);

    UINT64 dstBufferSize = 0;
    GetMemoryFootprintWithAlignment(
        format,
        extent,
        region.subresource.numArrayLayers,
        outRowStride,
        outLayerSize,
        outLayerStride,
        dstBufferSize
    );

    /* Create readback buffer with texture resource descriptor */
    ID3D12Resource* dstBuffer = context.CreateReadbackBuffer(dstBufferSize);

    /* Copy host visible resource to CPU accessible resource */
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT dstBufferFootprint;
    {
        dstBufferFootprint.Offset               = 0;
        dstBufferFootprint.Footprint.Format     = DXTypes::ToDXGIFormatTypeless(DXTypes::ToDXGIFormat(format));
        dstBufferFootprint.Footprint.Width      = extent.width;
        dstBufferFootprint.Footprint.Height     = extent.height;
        dstBufferFootprint.Footprint.Depth      = extent.depth;
        dstBufferFootprint.Footprint.RowPitch   = outRowStride;
    }

    const D3D12_BOX srcBox = CalcRegion(offset, extent);

    context.GetCommandContext().TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_SOURCE, true);

    for_range(arrayLayer, region.subresource.numArrayLayers)
    {
        const UINT srcSubresource = CalcSubresource(region.subresource.baseMipLevel, region.subresource.baseArrayLayer + arrayLayer, plane);
        const CD3DX12_TEXTURE_COPY_LOCATION dstLocationD3D(dstBuffer, dstBufferFootprint);
        const CD3DX12_TEXTURE_COPY_LOCATION srcLocationD3D(GetNative(), srcSubresource);
        context.GetCommandList()->CopyTextureRegion(
            /*pDst:*/       &dstLocationD3D,
            /*DstX:*/       0,
            /*DstY:*/       0,
            /*DstZ:*/       0,
            /*pSrc:*/       &srcLocationD3D,
            /*pSrcBox:*/    &srcBox
        );
        dstBufferFootprint.Offset += outLayerStride;
    }
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

void D3D12Texture::CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const TextureViewDescriptor& desc)
{
    CreateShaderResourceViewPrimary(
        device,
        D3D12Types::MapSrvDimension(desc.type),
        DXTypes::ToDXGIFormat(desc.format),
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

    srvDesc.Format                  = DXTypes::ToDXGIFormatSRV(format);
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

void D3D12Texture::CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    CreateUnorderedAccessViewPrimary(
        device,
        D3D12Types::MapUavDimension(GetType()),
        format_,
        TextureSubresource{ 0, numArrayLayers_, 0, 1 },
        cpuDescHandle
    );
}

void D3D12Texture::CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const TextureViewDescriptor& desc)
{
    CreateUnorderedAccessViewPrimary(
        device,
        D3D12Types::MapUavDimension(desc.type),
        DXTypes::ToDXGIFormat(desc.format),
        desc.subresource,
        cpuDescHandle
    );
}

//private
void D3D12Texture::CreateUnorderedAccessViewPrimary(
    ID3D12Device*               device,
    D3D12_UAV_DIMENSION         dimension,
    DXGI_FORMAT                 format,
    const TextureSubresource&   subresource,
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle)
{
    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

    uavDesc.Format          = DXTypes::ToDXGIFormatUAV(format);
    uavDesc.ViewDimension   = dimension;

    switch (uavDesc.ViewDimension)
    {
        case D3D12_UAV_DIMENSION_TEXTURE1D:
            uavDesc.Texture1D.MipSlice              = subresource.baseMipLevel;
            break;

        case D3D12_UAV_DIMENSION_TEXTURE1DARRAY:
            uavDesc.Texture1DArray.MipSlice         = subresource.baseMipLevel;
            uavDesc.Texture1DArray.FirstArraySlice  = subresource.baseArrayLayer;
            uavDesc.Texture1DArray.ArraySize        = subresource.numArrayLayers;
            break;

        case D3D12_UAV_DIMENSION_TEXTURE2D:
            uavDesc.Texture2D.MipSlice              = subresource.baseMipLevel;
            uavDesc.Texture2D.PlaneSlice            = 0;
            break;

        case D3D12_UAV_DIMENSION_TEXTURE2DARRAY:
            uavDesc.Texture2DArray.MipSlice         = subresource.baseMipLevel;
            uavDesc.Texture2DArray.FirstArraySlice  = subresource.baseArrayLayer;
            uavDesc.Texture2DArray.ArraySize        = subresource.numArrayLayers;
            uavDesc.Texture2DArray.PlaneSlice       = 0;
            break;

        case D3D12_UAV_DIMENSION_TEXTURE3D:
            uavDesc.Texture3D.MipSlice              = subresource.baseMipLevel;
            uavDesc.Texture3D.FirstWSlice           = subresource.baseArrayLayer;
            uavDesc.Texture3D.WSize                 = subresource.numArrayLayers;
            break;

        default:
            break;
    }

    device->CreateUnorderedAccessView(resource_.native.Get(), nullptr, &uavDesc, cpuDescHandle);
}

UINT D3D12Texture::CalcSubresource(UINT mipLevel, UINT arrayLayer, UINT plane) const
{
    return D3D12CalcSubresource(mipLevel, arrayLayer, plane, numMipLevels_, numArrayLayers_);
}

UINT D3D12Texture::CalcSubresource(const TextureLocation& location) const
{
    /* Only include array layer in subresource calculation for array and cube texture types */
    return CalcSubresource(location.mipLevel, std::min(location.arrayLayer, numArrayLayers_ - 1));
}

D3D12_TEXTURE_COPY_LOCATION D3D12Texture::CalcCopyLocation(const TextureLocation& location) const
{
    return GetD3DTextureSubresourceLocation(GetNative(), CalcSubresource(location));
}

D3D12_TEXTURE_COPY_LOCATION D3D12Texture::CalcCopyLocation(ID3D12Resource* srcResource, UINT64 srcOffset, const Extent3D& extent, UINT rowPitch) const
{
    D3D12_TEXTURE_COPY_LOCATION copyDesc;
    {
        copyDesc.pResource                            = srcResource;
        copyDesc.Type                                 = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        copyDesc.PlacedFootprint.Offset               = srcOffset;
        copyDesc.PlacedFootprint.Footprint.Format     = GetDXFormat();
        copyDesc.PlacedFootprint.Footprint.Width      = extent.width;
        copyDesc.PlacedFootprint.Footprint.Height     = extent.height;
        copyDesc.PlacedFootprint.Footprint.Depth      = extent.depth;
        copyDesc.PlacedFootprint.Footprint.RowPitch   = rowPitch;
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

static bool DXTextureSupportsGenerateMips(long bindFlags, UINT numMipLevels)
{
    return ((bindFlags & BindFlags::ColorAttachment) != 0 && numMipLevels > 1);
}

DXGI_FORMAT D3D12Texture::GetBaseDXFormat() const
{
    return DXTypes::ToDXGIFormat(GetBaseFormat());
}

bool D3D12Texture::SupportsGenerateMips() const
{
    return DXTextureSupportsGenerateMips(GetBindFlags(), GetNumMipLevels());
}


/*
 * ======= Private: =======
 */

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_resource_flags
static D3D12_RESOURCE_FLAGS GetD3D12TextureResourceFlags(long bindFlags, UINT numMipLevels)
{
    D3D12_RESOURCE_FLAGS flagsD3D = D3D12_RESOURCE_FLAG_NONE;

    bool isSrvAndUavEnabled = DXTextureSupportsGenerateMips(bindFlags, numMipLevels);

    if (!( (bindFlags & BindFlags::Sampled) != 0 || isSrvAndUavEnabled ) && (bindFlags & BindFlags::DepthStencilAttachment) != 0)
    {
        /* D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE must be used together with D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL */
        return (D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    }

    if ( (bindFlags & BindFlags::Storage) != 0 || isSrvAndUavEnabled )
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    if ((bindFlags & BindFlags::ColorAttachment) != 0)
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    if ((bindFlags & BindFlags::DepthStencilAttachment) != 0)
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    return flagsD3D;
}

static void ConvertD3D12TextureDesc(D3D12_RESOURCE_DESC& dst, const TextureDescriptor& src)
{
    dst.Dimension           = D3D12Types::MapResourceDimension(src.type);
    dst.Alignment           = 0;
    dst.MipLevels           = NumMipLevels(src);
    dst.Format              = DXTypes::SelectTextureDXGIFormat(src.format, src.bindFlags);
    dst.SampleDesc.Count    = (IsMultiSampleTexture(src.type) ? std::max(1u, src.samples) : 1u);
    dst.SampleDesc.Quality  = 0;
    dst.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dst.Flags               = GetD3D12TextureResourceFlags(src.bindFlags, dst.MipLevels);
    ConvertD3DTextureExtent(dst, src.type, src.extent, src.arrayLayers);
}

//TODO: incomplete
static D3D12_RESOURCE_STATES GetInitialD3D12ResourceState(const TextureDescriptor& desc)
{
    D3D12_RESOURCE_STATES flags = D3D12_RESOURCE_STATE_COMMON;

    if ((desc.bindFlags & BindFlags::Storage) != 0)
    {
        /* Read/write states are prior to read-only states */
        flags |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }
    else
    {
        /* Combine read states */
        if ((desc.bindFlags & BindFlags::DepthStencilAttachment) != 0)
            flags |= D3D12_RESOURCE_STATE_DEPTH_READ;
        if ((desc.bindFlags & BindFlags::Sampled) != 0)
            flags |= (D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }

    return flags;
}

void D3D12Texture::CreateNativeTexture(ID3D12Device* device, const TextureDescriptor& desc)
{
    /* Setup resource descriptor by texture descriptor and create hardware resource */
    D3D12_RESOURCE_DESC descD3D;
    ConvertD3D12TextureDesc(descD3D, desc);

    /* Get optimal clear value (if specified) */
    const bool useClearValue = ((desc.bindFlags & (BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment)) != 0);

    D3D12_CLEAR_VALUE optClearValue;
    if ((desc.bindFlags & BindFlags::ColorAttachment) != 0)
    {
        optClearValue.Format    = DXTypes::ToDXGIFormatRTV(DXTypes::ToDXGIFormat(desc.format));
        optClearValue.Color[0]  = desc.clearValue.color[0];
        optClearValue.Color[1]  = desc.clearValue.color[1];
        optClearValue.Color[2]  = desc.clearValue.color[2];
        optClearValue.Color[3]  = desc.clearValue.color[3];
    }
    else if ((desc.bindFlags & BindFlags::DepthStencilAttachment) != 0)
    {
        optClearValue.Format                = DXTypes::ToDXGIFormatDSV(descD3D.Format);
        optClearValue.DepthStencil.Depth    = desc.clearValue.depth;
        optClearValue.DepthStencil.Stencil  = static_cast<UINT8>(desc.clearValue.stencil);
    }

    /* Create hardware resource for the texture */
    const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &descD3D,
        resource_.SetInitialAndUsageStates(D3D12_RESOURCE_STATE_COPY_DEST, GetInitialD3D12ResourceState(desc)),
        (useClearValue ? &optClearValue : nullptr),
        IID_PPV_ARGS(resource_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for D3D12 hardware texture");
}

// Determine SRV dimension for descriptor heaps used in D3D12MipGenerator: either 1D array, 2D array, or 3D
static D3D12_SRV_DIMENSION GetMipChainSRVDimension(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        case TextureType::Texture3D:
            return D3D12_SRV_DIMENSION_TEXTURE3D;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            break;
    }
    return D3D12_SRV_DIMENSION_UNKNOWN;
}

// Determine UAV dimension for descriptor heaps used in D3D12MipGenerator: either 1D array, 2D array, or 3D
static D3D12_UAV_DIMENSION GetMipChainUAVDimension(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        case TextureType::Texture3D:
            return D3D12_UAV_DIMENSION_TEXTURE3D;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            break;
    }
    return D3D12_UAV_DIMENSION_UNKNOWN;
}

void D3D12Texture::CreateMipDescHeap(ID3D12Device* device)
{
    /* Create descriptor heap for all MIP-map levels */
    D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
    {
        heapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heapDesc.NumDescriptors = GetNumMipLevels();
        heapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heapDesc.NodeMask       = 0;
    }
    HRESULT hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mipDescHeap_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap for MIP-map chain");
    D3D12SetObjectName(mipDescHeap_.Get(), "LLGL::D3D12Texture::mipDescHeap");

    const UINT                  descSize        = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle   = mipDescHeap_->GetCPUDescriptorHandleForHeapStart();

    /* Create SRV for first MIP-map */
    CreateShaderResourceViewPrimary(
        device,
        GetMipChainSRVDimension(GetType()),
        format_,
        D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING,
        TextureSubresource{ 0, GetNumArrayLayers(), 0, GetNumMipLevels() },
        cpuDescHandle
    );
    cpuDescHandle.ptr += descSize;

    /* Create UAVs for remaining MIP-maps */
    D3D12_UAV_DIMENSION uavDimension = GetMipChainUAVDimension(GetType());
    D3D12_RESOURCE_DESC resourceDesc = resource_.native->GetDesc();

    for_subrange(mipLevel, 1, GetNumMipLevels())
    {
        if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            CreateUnorderedAccessViewPrimary(
                device,
                uavDimension,
                format_,
                TextureSubresource{ 0, std::max(1u, static_cast<UINT>(resourceDesc.DepthOrArraySize) >> mipLevel), mipLevel, 1 },
                cpuDescHandle
            );
            cpuDescHandle.ptr += descSize;
        }
        else
        {
            CreateUnorderedAccessViewPrimary(
                device,
                uavDimension,
                format_,
                TextureSubresource{ 0, GetNumArrayLayers(), mipLevel, 1 },
                cpuDescHandle
            );
            cpuDescHandle.ptr += descSize;
        }
    }
}


} // /namespace LLGL



// ================================================================================
