/*
 * D3D12Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Texture.h"
#include "../Command/D3D12CommandContext.h"
#include "../D3D12ObjectUtils.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12Types.h"
#include "../Buffer/D3D12Buffer.h"
#include "../../DXCommon/DXCore.h"
#include "../../TextureUtils.h"
#include "../../../Core/Helper.h"
#include <algorithm>


namespace LLGL
{


D3D12Texture::D3D12Texture(ID3D12Device* device, const TextureDescriptor& desc) :
    Texture         { desc.type, desc.bindFlags      },
    format_         { D3D12Types::Map(desc.format)   },
    numMipLevels_   { NumMipLevels(desc)             },
    numArrayLayers_ { std::max(1u, desc.arrayLayers) }
{
    CreateNativeTexture(device, desc);
    if (SupportsGenerateMips())
        CreateMipDescHeap(device);
}

void D3D12Texture::SetName(const char* name)
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
    texDesc.bindFlags   = 0;
    texDesc.miscFlags   = 0;
    texDesc.format      = D3D12Types::Unmap(desc.Format);
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
    return D3D12Types::Unmap(GetDXFormat());
}

//TODO: make use of <D3D12StagingBufferPool> instead of <uploadBuffer>
void D3D12Texture::UpdateSubresource(
    ID3D12Device*               device,
    ID3D12GraphicsCommandList*  commandList,
    ComPtr<ID3D12Resource>&     uploadBuffer,
    D3D12_SUBRESOURCE_DATA&     subresourceData,
    UINT                        mipLevel,
    UINT                        firstArrayLayer,
    UINT                        numArrayLayers)
{
    /* Clamp arguments */
    firstArrayLayer = std::min(firstArrayLayer, numArrayLayers_ - 1u);
    numArrayLayers  = std::min(numArrayLayers, numArrayLayers_ - firstArrayLayer);

    /* Create the GPU upload buffer */
    UINT64 uploadBufferSize     = GetRequiredIntermediateSize(resource_.native.Get(), 0, 1) * numArrayLayers;
    UINT64 uploadBufferOffset   = 0;

    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(uploadBuffer.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for texture upload buffer");

    /* Upload subresource for each array layer */
    for (UINT arrayLayer = 0; arrayLayer < numArrayLayers; ++arrayLayer)
    {
        /* Update subresource for current array layer */
        UINT subresourceIndex = CalcSubresource(mipLevel, firstArrayLayer + arrayLayer);

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

// Returns the memory footprint of a texture with row alignment
static void GetMemoryFootprintWithAlignment(const Format format, const Extent3D& extent, UINT rowAlignment, UINT& rowPitch, UINT64& bufferSize)
{
    const auto& formatAttribs = GetFormatAttribs(format);
    rowPitch    = GetAlignedSize(extent.width * formatAttribs.bitSize / (8 * formatAttribs.blockWidth), rowAlignment);
    bufferSize  = rowPitch * (extent.height / formatAttribs.blockHeight) * extent.depth - rowPitch + rowPitch;
}

void D3D12Texture::CreateSubresourceCopyAsReadbackBuffer(
    ID3D12Device*           device,
    D3D12CommandContext&    commandContext,
    const TextureRegion&    region,
    ComPtr<ID3D12Resource>& readbackBuffer,
    UINT&                   rowStride)
{
    /* Determine required buffer size for texture subresource */
    const auto offset = CalcTextureOffset(GetType(), region.offset, region.subresource.baseArrayLayer);
    const auto extent = CalcTextureExtent(GetType(), region.extent, region.subresource.numArrayLayers);
    const auto format = GetFormat();

    UINT64 bufferSize = 0;
    GetMemoryFootprintWithAlignment(format, extent, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT, rowStride, bufferSize);

    /* Create readback buffer with texture resource descriptor */
    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(readbackBuffer.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for texture readback buffer");

    /* Copy host visible resource to CPU accessible resource */
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT bufferFootprint;
    {
        bufferFootprint.Offset              = 0;
        bufferFootprint.Footprint.Format    = GetDXFormat();
        bufferFootprint.Footprint.Width     = extent.width;
        bufferFootprint.Footprint.Height    = extent.height;
        bufferFootprint.Footprint.Depth     = extent.depth;
        bufferFootprint.Footprint.RowPitch  = rowStride;
    }

    const auto srcBox = CalcRegion(offset, extent);

    commandContext.TransitionResource(resource_, D3D12_RESOURCE_STATE_COPY_SOURCE, true);
    {
        const UINT srcSubresource = CalcSubresource(region.subresource.baseMipLevel, 0/*region.subresource.baseArrayLayer*/);
        commandContext.GetCommandList()->CopyTextureRegion(
            &CD3DX12_TEXTURE_COPY_LOCATION(readbackBuffer.Get(), bufferFootprint),
            0, 0, 0,
            &CD3DX12_TEXTURE_COPY_LOCATION(GetNative(), srcSubresource),
            &srcBox
        );
    }
    commandContext.TransitionResource(resource_, resource_.usageState, true);
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

    srvDesc.Format                  = D3D12Types::ToDXGIFormatSRV(format);
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
        D3D12Types::Map(desc.format),
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

    uavDesc.Format          = D3D12Types::ToDXGIFormatUAV(format);
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

UINT D3D12Texture::CalcSubresource(UINT mipLevel, UINT arrayLayer) const
{
    return D3D12CalcSubresource(mipLevel, arrayLayer, /*planeSlice: */0, numMipLevels_, numArrayLayers_);
}

UINT D3D12Texture::CalcSubresource(const TextureLocation& location) const
{
    /* Only include array layer in subresource calculation for array and cube texture types */
    return CalcSubresource(location.mipLevel, std::min(location.arrayLayer, numArrayLayers_ - 1));
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

D3D12_TEXTURE_COPY_LOCATION D3D12Texture::CalcCopyLocation(const D3D12Buffer& srcBuffer, UINT64 srcOffset, const Extent3D& extent, UINT rowPitch) const
{
    D3D12_TEXTURE_COPY_LOCATION copyDesc;
    {
        copyDesc.pResource                            = srcBuffer.GetNative();
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

bool D3D12Texture::SupportsGenerateMips() const
{
    return DXTextureSupportsGenerateMips(GetBindFlags(), GetNumMipLevels());
}


/*
 * ======= Private: =======
 */

// see https://docs.microsoft.com/en-us/windows/desktop/api/d3d12/ne-d3d12-d3d12_resource_flags
static D3D12_RESOURCE_FLAGS GetD3DTextureResourceFlags(long bindFlags, UINT numMipLevels)
{
    D3D12_RESOURCE_FLAGS flagsD3D = D3D12_RESOURCE_FLAG_NONE;

    bool enabledSrvAndUav = DXTextureSupportsGenerateMips(bindFlags, numMipLevels);

    if ((bindFlags & BindFlags::Sampled) == 0 && !enabledSrvAndUav)
        flagsD3D |= (D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    else if ((bindFlags & BindFlags::ColorAttachment) != 0)
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    else if ((bindFlags & BindFlags::DepthStencilAttachment) != 0)
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    if ((bindFlags & BindFlags::Storage) != 0 || enabledSrvAndUav)
        flagsD3D |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    return flagsD3D;
}

static void Convert(D3D12_RESOURCE_DESC& dst, const TextureDescriptor& src)
{
    dst.Dimension           = D3D12Types::MapResourceDimension(src.type);
    dst.Alignment           = 0;
    dst.MipLevels           = NumMipLevels(src);
    dst.Format              = D3D12Types::ToDXGIFormatDSV(D3D12Types::Map(src.format));
    dst.SampleDesc.Count    = 1;
    dst.SampleDesc.Quality  = 0;
    dst.Layout              = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    dst.Flags               = GetD3DTextureResourceFlags(src.bindFlags, dst.MipLevels);

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

//TODO: incomplete
static D3D12_RESOURCE_STATES GetInitialDXResourceState(const TextureDescriptor& desc)
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
        optClearValue.Format                = descD3D.Format;
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
    resource_.transitionState   = D3D12_RESOURCE_STATE_COPY_DEST;
    resource_.usageState        = GetInitialDXResourceState(desc);
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
    auto hr = device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(mipDescHeap_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create D3D12 descriptor heap for MIP-map chain");

    auto descSize       = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    auto cpuDescHandle  = mipDescHeap_->GetCPUDescriptorHandleForHeapStart();

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
    auto uavDimension = GetMipChainUAVDimension(GetType());
    auto resourceDesc = resource_.native->GetDesc();

    for (UINT i = 1; i < GetNumMipLevels(); ++i)
    {
        if (resourceDesc.Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            CreateUnorderedAccessViewPrimary(
                device,
                uavDimension,
                format_,
                TextureSubresource{ 0, static_cast<UINT>(resourceDesc.DepthOrArraySize) >> i, i, 1 },
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
                TextureSubresource{ 0, GetNumArrayLayers(), i, 1 },
                cpuDescHandle
            );
            cpuDescHandle.ptr += descSize;
        }
    }
}


} // /namespace LLGL



// ================================================================================
