/*
 * D3D11Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Texture.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../D3D11ResourceFlags.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../TextureUtils.h"


namespace LLGL
{


D3D11Texture::D3D11Texture(const TextureDescriptor& desc) :
    Texture { desc.type, desc.bindFlags }
{
}

void D3D11Texture::SetName(const char* name)
{
    D3D11SetObjectName(GetNative().resource.Get(), name);
    if (srv_)
        D3D11SetObjectNameSubscript(srv_.Get(), name, ".SRV");
    if (uav_)
        D3D11SetObjectNameSubscript(uav_.Get(), name, ".UAV");
}

Extent3D D3D11Texture::GetMipExtent(std::uint32_t mipLevel) const
{
    Extent3D size;

    if (native_.resource)
    {
        D3D11_RESOURCE_DIMENSION dimension;
        native_.resource->GetType(&dimension);

        switch (dimension)
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                /* Query MIP-level size for 1D texture */
                D3D11_TEXTURE1D_DESC desc;
                native_.tex1D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                {
                    size.width  = std::max(1u, desc.Width >> mipLevel);
                    size.height = desc.ArraySize;
                    size.depth  = 1u;
                }
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                /* Query MIP-level size for 2D texture */
                D3D11_TEXTURE2D_DESC desc;
                native_.tex2D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                {
                    size.width  = std::max(1u, desc.Width  >> mipLevel);
                    size.height = std::max(1u, desc.Height >> mipLevel);
                    size.depth  = desc.ArraySize;
                }
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                /* Query MIP-level size for 3D texture */
                D3D11_TEXTURE3D_DESC desc;
                native_.tex3D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                {
                    size.width  = std::max(1u, desc.Width  >> mipLevel);
                    size.height = std::max(1u, desc.Height >> mipLevel);
                    size.depth  = std::max(1u, desc.Depth  >> mipLevel);
                }
            }
            break;
        }
    }

    return size;
}

TextureDescriptor D3D11Texture::GetDesc() const
{
    /* Get D3D hardware texture resource */
    const auto& hwTex = GetNative();

    /* Initialize texture descriptor */
    TextureDescriptor texDesc;

    texDesc.type        = GetType();
    texDesc.bindFlags   = 0;
    texDesc.miscFlags   = 0;

    /* Get resource dimension to query the respective D3D descriptor */
    D3D11_RESOURCE_DIMENSION dimension;
    hwTex.resource->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            /* Query descriptor from 1D texture */
            D3D11_TEXTURE1D_DESC desc;
            hwTex.tex1D->GetDesc(&desc);

            texDesc.format      = D3D11Types::Unmap(desc.Format);
            texDesc.extent      = { desc.Width, 1u, 1u };
            texDesc.arrayLayers = desc.ArraySize;
            texDesc.mipLevels   = desc.MipLevels;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            /* Query descriptor from 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            hwTex.tex2D->GetDesc(&desc);

            texDesc.format      = D3D11Types::Unmap(desc.Format);
            texDesc.extent      = { desc.Width, desc.Height, 1u };
            texDesc.arrayLayers = desc.ArraySize;
            texDesc.mipLevels   = desc.MipLevels;
            texDesc.samples     = desc.SampleDesc.Count;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            /* Query descriptor from 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            hwTex.tex3D->GetDesc(&desc);

            texDesc.format      = D3D11Types::Unmap(desc.Format);
            texDesc.extent      = { desc.Width, desc.Height, desc.Depth };
            texDesc.mipLevels   = desc.MipLevels;
        }
        break;
    }

    return texDesc;
}

Format D3D11Texture::GetFormat() const
{
    return D3D11Types::Unmap(GetDXFormat());
}

static ComPtr<ID3D11Texture1D> DXCreateTexture1D(
    ID3D11Device*                   device,
    const D3D11_TEXTURE1D_DESC&     desc,
    const D3D11_SUBRESOURCE_DATA*   initialData = nullptr)
{
    ComPtr<ID3D11Texture1D> tex1D;

    auto hr = device->CreateTexture1D(&desc, initialData, &tex1D);
    DXThrowIfCreateFailed(hr, "ID3D11Texture1D");

    return tex1D;
}

static ComPtr<ID3D11Texture2D> DXCreateTexture2D(
    ID3D11Device*                   device,
    const D3D11_TEXTURE2D_DESC&     desc,
    const D3D11_SUBRESOURCE_DATA*   initialData = nullptr)
{
    ComPtr<ID3D11Texture2D> tex2D;

    auto hr = device->CreateTexture2D(&desc, initialData, &tex2D);
    DXThrowIfCreateFailed(hr, "ID3D11Texture2D");

    return tex2D;
}

static ComPtr<ID3D11Texture3D> DXCreateTexture3D(
    ID3D11Device*                   device,
    const D3D11_TEXTURE3D_DESC&     desc,
    const D3D11_SUBRESOURCE_DATA*   initialData = nullptr)
{
    ComPtr<ID3D11Texture3D> tex3D;

    auto hr = device->CreateTexture3D(&desc, initialData, &tex3D);
    DXThrowIfCreateFailed(hr, "ID3D11Texture3D");

    return tex3D;
}

void D3D11Texture::CreateTexture1D(
    ID3D11Device*                           device,
    const TextureDescriptor&                desc,
    const D3D11_SUBRESOURCE_DATA*           initialData,
    const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc,
    const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
{
    /* Create native D3D texture */
    D3D11_TEXTURE1D_DESC descD3D;
    {
        descD3D.Width           = desc.extent.width;
        descD3D.MipLevels       = NumMipLevels(desc);
        descD3D.ArraySize       = desc.arrayLayers;
        descD3D.Format          = D3D11Types::Map(desc.format);
        descD3D.Usage           = DXGetTextureUsage(desc);
        descD3D.BindFlags       = DXGetTextureBindFlags(desc);
        descD3D.CPUAccessFlags  = DXGetCPUAccessFlagsForMiscFlags(desc.miscFlags);
        descD3D.MiscFlags       = DXGetTextureMiscFlags(desc);
    }
    native_.tex1D = DXCreateTexture1D(device, descD3D, initialData);

    /* Store parameters and create default resource views */
    SetResourceParams(descD3D.Format, Extent3D{ descD3D.Width, 1u, 1u }, descD3D.MipLevels, descD3D.ArraySize);
    CreateDefaultResourceViews(device, srvDesc, uavDesc, desc.bindFlags);
}

void D3D11Texture::CreateTexture2D(
    ID3D11Device*                           device,
    const TextureDescriptor&                desc,
    const D3D11_SUBRESOURCE_DATA*           initialData,
    const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc,
    const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
{
    /* Create native D3D texture */
    D3D11_TEXTURE2D_DESC descD3D;
    {
        descD3D.Width               = desc.extent.width;
        descD3D.Height              = desc.extent.height;
        descD3D.MipLevels           = NumMipLevels(desc);
        descD3D.ArraySize           = desc.arrayLayers;
        descD3D.Format              = D3D11Types::Map(desc.format);
        descD3D.SampleDesc.Count    = (IsMultiSampleTexture(desc.type) ? std::max(1u, desc.samples) : 1u);
        descD3D.SampleDesc.Quality  = 0;//(desc.miscFlags & MiscFlags::FixedSamples ? D3D11_CENTER_MULTISAMPLE_PATTERN : 0);
        descD3D.Usage               = DXGetTextureUsage(desc);
        descD3D.BindFlags           = DXGetTextureBindFlags(desc);
        descD3D.CPUAccessFlags      = DXGetCPUAccessFlagsForMiscFlags(desc.miscFlags);
        descD3D.MiscFlags           = DXGetTextureMiscFlags(desc);
    }
    native_.tex2D = DXCreateTexture2D(device, descD3D, initialData);

    /* Store parameters and create default resource views */
    SetResourceParams(descD3D.Format, Extent3D{ descD3D.Width, descD3D.Height, 1u }, descD3D.MipLevels, descD3D.ArraySize);
    CreateDefaultResourceViews(device, srvDesc, uavDesc, desc.bindFlags);
}

void D3D11Texture::CreateTexture3D(
    ID3D11Device*                           device,
    const TextureDescriptor&                desc,
    const D3D11_SUBRESOURCE_DATA*           initialData,
    const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc,
    const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
{
    /* Create native D3D texture */
    D3D11_TEXTURE3D_DESC descD3D;
    {
        descD3D.Width           = desc.extent.width;
        descD3D.Height          = desc.extent.height;
        descD3D.Depth           = desc.extent.depth;
        descD3D.MipLevels       = NumMipLevels(desc);
        descD3D.Format          = D3D11Types::Map(desc.format);
        descD3D.Usage           = DXGetTextureUsage(desc);
        descD3D.BindFlags       = DXGetTextureBindFlags(desc);
        descD3D.CPUAccessFlags  = DXGetCPUAccessFlagsForMiscFlags(desc.miscFlags);
        descD3D.MiscFlags       = DXGetTextureMiscFlags(desc);
    }
    native_.tex3D = DXCreateTexture3D(device, descD3D, initialData);

    /* Store parameters and create default resource views */
    SetResourceParams(descD3D.Format, Extent3D{ descD3D.Width, descD3D.Height, descD3D.Depth }, descD3D.MipLevels, 1);
    CreateDefaultResourceViews(device, srvDesc, uavDesc, desc.bindFlags);
}

void D3D11Texture::UpdateSubresource(
    ID3D11DeviceContext*        context,
    UINT                        mipLevel,
    UINT                        arrayLayer,
    const D3D11_BOX&            region,
    const SrcImageDescriptor&   imageDesc,
    std::size_t                 threadCount)
{
    /* Check if source image must be converted */
    auto format = D3D11Types::Unmap(format_);
    const auto& formatAttribs = GetFormatAttribs(format);

    /* Get destination subresource index */
    auto dstSubresource = CalcSubresource(mipLevel, arrayLayer);
    auto dataLayout     = CalcSubresourceLayout(
        format,
        Extent3D
        {
            region.right - region.left,
            region.bottom - region.top,
            region.back - region.front
        }
    );

    ByteBuffer intermediateData;
    const void* initialData = imageDesc.data;

    if ((formatAttribs.flags & FormatFlags::IsCompressed) == 0 &&
        (formatAttribs.format != imageDesc.format || formatAttribs.dataType != imageDesc.dataType))
    {
        /* Convert image data (e.g. from RGB to RGBA), and redirect initial data to new buffer */
        intermediateData    = ConvertImageBuffer(imageDesc, formatAttribs.format, formatAttribs.dataType, threadCount);
        initialData         = intermediateData.get();
    }
    else
    {
        /* Validate input data is large enough */
        if (imageDesc.dataSize < dataLayout.dataSize)
        {
            throw std::invalid_argument(
                "image data size is too small to update subresource of D3D11 texture (" +
                std::to_string(dataLayout.dataSize) + " is required but only " + std::to_string(imageDesc.dataSize) + " was specified)"
            );
        }
    }

    /* Update subresource with specified image data */
    context->UpdateSubresource(
        native_.resource.Get(),
        dstSubresource,
        &region,
        initialData,
        dataLayout.rowStride,
        dataLayout.layerStride
    );
}

void D3D11Texture::CreateSubresourceCopyWithCPUAccess(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    D3D11NativeTexture&     textureOutput,
    UINT                    cpuAccessFlags,
    const TextureRegion&    region)
{
    D3D11_RESOURCE_DIMENSION dimension;
    native_.resource->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            /* Create temporary 1D texture with a similar descriptor */
            D3D11_TEXTURE1D_DESC desc;
            native_.tex1D->GetDesc(&desc);
            {
                desc.Width          = region.extent.width;
                desc.MipLevels      = 1;
                desc.ArraySize      = region.subresource.numArrayLayers;
                desc.Usage          = D3D11_USAGE_STAGING;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = 0;
            }
            textureOutput.tex1D = DXCreateTexture1D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            /* Query and modify descriptor for 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            native_.tex2D->GetDesc(&desc);
            {
                desc.Width          = region.extent.width;
                desc.Height         = region.extent.height;
                desc.MipLevels      = 1;
                desc.ArraySize      = region.subresource.numArrayLayers;
                desc.Usage          = D3D11_USAGE_STAGING;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE);
            }
            textureOutput.tex2D = DXCreateTexture2D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            /* Query and modify descriptor for 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            native_.tex3D->GetDesc(&desc);
            {
                desc.Width          = region.extent.width;
                desc.Height         = region.extent.height;
                desc.Depth          = region.extent.depth;
                desc.MipLevels      = 1;
                desc.Usage          = D3D11_USAGE_STAGING;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = 0;
            }
            textureOutput.tex3D = DXCreateTexture3D(device, desc);
        }
        break;
    }

    /* Copy subresource */
    const UINT mipLevel = region.subresource.baseMipLevel;

    const D3D11_BOX srcBox
    {
        static_cast<UINT>(region.offset.x),
        static_cast<UINT>(region.offset.y),
        static_cast<UINT>(region.offset.z),
        static_cast<UINT>(region.offset.x) + region.extent.width,
        static_cast<UINT>(region.offset.y) + region.extent.height,
        static_cast<UINT>(region.offset.z) + region.extent.depth,
    };

    for (std::uint32_t i = 0; i < region.subresource.numArrayLayers; ++i)
    {
        const UINT arrayLayer = region.subresource.baseArrayLayer + i;
        context->CopySubresourceRegion(
            textureOutput.resource.Get(),
            D3D11CalcSubresource(0, i, 1),
            0, // DstX
            0, // DstY
            0, // DstZ
            native_.resource.Get(),
            D3D11CalcSubresource(mipLevel, arrayLayer, numMipLevels_),
            &srcBox
        );
    }
}

static void CreateD3D11TextureSubresourceSRV(
    ID3D11Device*               device,
    ID3D11Resource*             resource,
    ID3D11ShaderResourceView**  srvOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        baseMipLevel,
    UINT                        numMipLevels,
    UINT                        baseArrayLayer,
    UINT                        numArrayLayers,
    const char*                 errorContextInfo = nullptr)
{
    /* Create shader-resource-view (SRV) for subresource */
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    {
        srvDesc.Format = D3D11Types::ToDXGIFormatSRV(format);

        switch (type)
        {
            case TextureType::Texture1D:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURE1D;
                srvDesc.Texture1D.MostDetailedMip           = baseMipLevel;
                srvDesc.Texture1D.MipLevels                 = numMipLevels;
                break;

            case TextureType::Texture2D:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MostDetailedMip           = baseMipLevel;
                srvDesc.Texture2D.MipLevels                 = numMipLevels;
                break;

            case TextureType::Texture3D:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURE3D;
                srvDesc.Texture3D.MostDetailedMip           = baseMipLevel;
                srvDesc.Texture3D.MipLevels                 = numMipLevels;
                break;

            case TextureType::TextureCube:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURECUBE;
                srvDesc.TextureCube.MostDetailedMip         = baseMipLevel;
                srvDesc.TextureCube.MipLevels               = numMipLevels;
                break;

            case TextureType::Texture1DArray:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
                srvDesc.Texture1DArray.MostDetailedMip      = baseMipLevel;
                srvDesc.Texture1DArray.MipLevels            = numMipLevels;
                srvDesc.Texture1DArray.FirstArraySlice      = baseArrayLayer;
                srvDesc.Texture1DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::Texture2DArray:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.MostDetailedMip      = baseMipLevel;
                srvDesc.Texture2DArray.MipLevels            = numMipLevels;
                srvDesc.Texture2DArray.FirstArraySlice      = baseArrayLayer;
                srvDesc.Texture2DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::TextureCubeArray:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
                srvDesc.TextureCubeArray.MostDetailedMip    = baseMipLevel;
                srvDesc.TextureCubeArray.MipLevels          = numMipLevels;
                srvDesc.TextureCubeArray.First2DArrayFace   = baseArrayLayer;
                srvDesc.TextureCubeArray.NumCubes           = numArrayLayers;
                break;

            case TextureType::Texture2DMS:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURE2DMS;
                break;

            case TextureType::Texture2DMSArray:
                srvDesc.ViewDimension                       = D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
                srvDesc.Texture2DMSArray.FirstArraySlice    = baseArrayLayer;
                srvDesc.Texture2DMSArray.ArraySize          = numArrayLayers;
                break;
        }
    }
    auto hr = device->CreateShaderResourceView(resource, &srvDesc, srvOutput);
    DXThrowIfCreateFailed(hr, "ID3D11ShaderResourceView", errorContextInfo);
}

static void CreateD3D11TextureSubresourceUAV(
    ID3D11Device*               device,
    ID3D11Resource*             resource,
    ID3D11UnorderedAccessView** uavOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        baseMipLevel,
    UINT                        baseArrayLayer,
    UINT                        numArrayLayers,
    const char*                 errorContextInfo = nullptr)
{
    /* Create unordered-access-view (UAV) for subresource */
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    {
        uavDesc.Format = DXTypes::ToDXGIFormatUAV(format);

        switch (type)
        {
            case TextureType::Texture1D:
                uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE1D;
                uavDesc.Texture1D.MipSlice              = baseMipLevel;
                break;

            case TextureType::Texture2D:
                uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice              = baseMipLevel;
                break;

            case TextureType::Texture3D:
                uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.MipSlice              = baseMipLevel;
                uavDesc.Texture3D.FirstWSlice           = baseArrayLayer;
                uavDesc.Texture3D.WSize                 = numArrayLayers;
                break;

            case TextureType::Texture1DArray:
                uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
                uavDesc.Texture1DArray.MipSlice         = baseMipLevel;
                uavDesc.Texture1DArray.FirstArraySlice  = baseArrayLayer;
                uavDesc.Texture1DArray.ArraySize        = numArrayLayers;
                break;

            case TextureType::Texture2DArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice         = baseMipLevel;
                uavDesc.Texture2DArray.FirstArraySlice  = baseArrayLayer;
                uavDesc.Texture2DArray.ArraySize        = numArrayLayers;
                break;

            case TextureType::Texture2DMS:
            case TextureType::Texture2DMSArray:
                /* ignore */
                break;
        }
    }
    auto hr = device->CreateUnorderedAccessView(resource, &uavDesc, uavOutput);
    DXThrowIfCreateFailed(hr, "ID3D11UnorderedAccessView", errorContextInfo);
}

void D3D11Texture::CreateSubresourceCopyWithUIntFormat(
    ID3D11Device*               device,
    D3D11NativeTexture&         textureOutput,
    ID3D11ShaderResourceView**  srvOutput,
    ID3D11UnorderedAccessView** uavOutput,
    const TextureRegion&        region,
    const TextureType           subresourceType)
{
    /* Determine binding flags for resource views */
    UINT bindFlags = 0;
    if (srvOutput != nullptr)
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    if (uavOutput != nullptr)
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;

    D3D11_RESOURCE_DIMENSION    dimension   = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    DXGI_FORMAT                 format      = DXGI_FORMAT_UNKNOWN;
    UINT                        arraySize   = 1;

    native_.resource->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            /* Create temporary 1D texture with a similar descriptor */
            D3D11_TEXTURE1D_DESC desc;
            native_.tex1D->GetDesc(&desc);
            format  = DXTypes::ToDXGIFormatUInt(desc.Format);
            arraySize = desc.ArraySize;
            {
                desc.Width          = region.extent.width;
                desc.MipLevels      = 1;
                desc.ArraySize      = region.subresource.numArrayLayers;
                desc.Format         = format;
                desc.Usage          = D3D11_USAGE_DEFAULT;
                desc.BindFlags      = bindFlags;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags      = 0;
            }
            textureOutput.tex1D = DXCreateTexture1D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            /* Query and modify descriptor for 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            native_.tex2D->GetDesc(&desc);
            format = DXTypes::ToDXGIFormatUInt(desc.Format);
            arraySize = desc.ArraySize;
            {
                desc.Width          = region.extent.width;
                desc.Height         = region.extent.height;
                desc.MipLevels      = 1;
                desc.ArraySize      = region.subresource.numArrayLayers;
                desc.Format         = format;
                desc.Usage          = D3D11_USAGE_DEFAULT;
                desc.BindFlags      = bindFlags;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags      = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE);
            }
            textureOutput.tex2D = DXCreateTexture2D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            /* Query and modify descriptor for 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            native_.tex3D->GetDesc(&desc);
            format = DXTypes::ToDXGIFormatUInt(desc.Format);
            {
                desc.Width          = region.extent.width;
                desc.Height         = region.extent.height;
                desc.Depth          = region.extent.depth;
                desc.MipLevels      = 1;
                desc.Format         = format;
                desc.Usage          = D3D11_USAGE_DEFAULT;
                desc.BindFlags      = bindFlags;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags      = 0;
            }
            textureOutput.tex3D = DXCreateTexture3D(device, desc);
        }
        break;
    }

    /* Create SRV and UAV for entire subresource copy */
    if (srvOutput != nullptr)
    {
        CreateD3D11TextureSubresourceSRV(
            device,
            textureOutput.resource.Get(),
            srvOutput,
            subresourceType,
            format,
            0,
            1,
            0,
            arraySize,
            "for texture subresource copy"
        );
    }

    if (uavOutput != nullptr)
    {
        CreateD3D11TextureSubresourceUAV(
            device,
            textureOutput.resource.Get(),
            uavOutput,
            subresourceType,
            format,
            0,
            0,
            arraySize,
            "for texture subresource copy"
        );
    }
}

void D3D11Texture::CreateSubresourceDSV(
    ID3D11Device*               device,
    ID3D11DepthStencilView**    dsvOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        baseMipLevel,
    UINT                        baseArrayLayer,
    UINT                        numArrayLayers)
{
    /* Create depth-stencil-view (DSV) for subresource */
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    {
        dsvDesc.Format  = D3D11Types::ToDXGIFormatDSV(format);
        dsvDesc.Flags   = 0;

        switch (type)
        {
            case TextureType::Texture1D:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE1D;
                dsvDesc.Texture1D.MipSlice                  = baseMipLevel;
                break;

            case TextureType::Texture2D:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2D;
                dsvDesc.Texture2D.MipSlice                  = baseMipLevel;
                break;

            case TextureType::Texture1DArray:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE1DARRAY;
                dsvDesc.Texture1DArray.MipSlice             = baseMipLevel;
                dsvDesc.Texture1DArray.FirstArraySlice      = baseArrayLayer;
                dsvDesc.Texture1DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::Texture3D:
            case TextureType::Texture2DArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
                dsvDesc.Texture2DArray.MipSlice             = baseMipLevel;
                dsvDesc.Texture2DArray.FirstArraySlice      = baseArrayLayer;
                dsvDesc.Texture2DArray.ArraySize            = numArrayLayers;
                break;

            case TextureType::Texture2DMS:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2DMS;
                break;

            case TextureType::Texture2DMSArray:
                dsvDesc.ViewDimension                       = D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY;
                dsvDesc.Texture2DMSArray.FirstArraySlice    = baseArrayLayer;
                dsvDesc.Texture2DMSArray.ArraySize          = numArrayLayers;
                break;
        }
    }
    auto hr = device->CreateDepthStencilView(native_.resource.Get(), &dsvDesc, dsvOutput);
    DXThrowIfCreateFailed(hr, "ID3D11DepthStencilView",  "for texture subresource");
}

void D3D11Texture::CreateSubresourceSRV(
    ID3D11Device*               device,
    ID3D11ShaderResourceView**  srvOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        baseMipLevel,
    UINT                        numMipLevels,
    UINT                        baseArrayLayer,
    UINT                        numArrayLayers)
{
    CreateD3D11TextureSubresourceSRV(
        device,
        native_.resource.Get(),
        srvOutput,
        type,
        format,
        baseMipLevel,
        numMipLevels,
        baseArrayLayer,
        numArrayLayers,
        "for texture subresource"
    );
}

void D3D11Texture::CreateSubresourceUAV(
    ID3D11Device*               device,
    ID3D11UnorderedAccessView** uavOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        baseMipLevel,
    UINT                        baseArrayLayer,
    UINT                        numArrayLayers)
{
    CreateD3D11TextureSubresourceUAV(
        device,
        native_.resource.Get(),
        uavOutput,
        type,
        format,
        baseMipLevel,
        baseArrayLayer,
        numArrayLayers,
        "for texture subresource"
    );
}

// Returns true if the specified texture type contains an array layer for D3D11 textures
static bool HasArrayLayer(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1DArray:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::Texture2DMSArray:
        case TextureType::TextureCubeArray:
            return true;
        default:
            return false;
    }
}

UINT D3D11Texture::CalcSubresource(UINT mipLevel, UINT arrayLayer) const
{
    if (HasArrayLayer(GetType()))
        return D3D11CalcSubresource(mipLevel, arrayLayer, numMipLevels_);
    else
        return D3D11CalcSubresource(mipLevel, 0, numMipLevels_);
}

UINT D3D11Texture::CalcSubresource(const TextureLocation& location) const
{
    return CalcSubresource(location.mipLevel, location.arrayLayer);
}

D3D11_BOX D3D11Texture::CalcRegion(const Offset3D& offset, const Extent3D& extent) const
{
    /* Ignore sub components of offset and extent if it's handled by the subresource index */
    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            return CD3D11_BOX(
                offset.x,
                0,
                0,
                offset.x + static_cast<LONG>(extent.width),
                1,
                1
            );
        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            return CD3D11_BOX(
                offset.x,
                offset.y,
                0,
                offset.x + static_cast<LONG>(extent.width),
                offset.y + static_cast<LONG>(extent.height),
                1
            );
        case TextureType::Texture3D:
            return CD3D11_BOX(
                offset.x,
                offset.y,
                offset.z,
                offset.x + static_cast<LONG>(extent.width),
                offset.y + static_cast<LONG>(extent.height),
                offset.z + static_cast<LONG>(extent.depth)
            );
        default:
            return CD3D11_BOX(0,0,0 , 0,0,0);
    }
}


/*
 * ====== Private: ======
 */

void D3D11Texture::CreateDefaultResourceViews(
    ID3D11Device*                           device,
    const D3D11_SHADER_RESOURCE_VIEW_DESC*  srvDesc,
    const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc,
    long                                    bindFlags)
{
    if ((bindFlags & BindFlags::Sampled) != 0)
        CreateDefaultSRV(device, srvDesc);
    if ((bindFlags & BindFlags::Storage) != 0)
        CreateDefaultUAV(device, uavDesc);
}

void D3D11Texture::CreateDefaultSRV(ID3D11Device* device, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    if (!srvDesc && IsDepthStencilFormat(D3D11Types::Unmap(format_)))
    {
        /* Create internal SRV for entire texture resource */
        CreateSubresourceSRV(device, srv_.ReleaseAndGetAddressOf(), GetType(), GetDXFormat(), 0, numMipLevels_, 0, numArrayLayers_);
    }
    else
    {
        /* Create internal SRV for entire texture resource */
        auto hr = device->CreateShaderResourceView(native_.resource.Get(), srvDesc, srv_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11ShaderResourceView", "for texture");
    }
}

void D3D11Texture::CreateDefaultUAV(ID3D11Device* device, const D3D11_UNORDERED_ACCESS_VIEW_DESC* uavDesc)
{
    auto hr = device->CreateUnorderedAccessView(native_.resource.Get(), uavDesc, uav_.ReleaseAndGetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11UnorderedAccessView", "for texture");
}

void D3D11Texture::SetResourceParams(DXGI_FORMAT format, const Extent3D& extent, UINT mipLevels, UINT arraySize)
{
    format_         = format;
    numMipLevels_   = (mipLevels == 0 ? NumMipLevels(extent.width, extent.height, extent.height) : mipLevels);
    numArrayLayers_ = arraySize;
}


} // /namespace LLGL



// ================================================================================
