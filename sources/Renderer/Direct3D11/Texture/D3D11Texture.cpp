/*
 * D3D11Texture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11Texture.h"
#include "../D3D11Types.h"
#include "../D3D11ObjectUtils.h"
#include "../D3D11ResourceFlags.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../TextureUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Report.h>


namespace LLGL
{


D3D11Texture::D3D11Texture(ID3D11Device* device, const TextureDescriptor& desc) :
    Texture         { desc.type, desc.bindFlags             },
    baseFormat_     { desc.format                           },
    bindingLocator_ { ResourceType::Texture, desc.bindFlags }
{
    switch (desc.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            CreateTexture1D(device, desc);
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            CreateTexture2D(device, desc);
            break;
        case TextureType::Texture3D:
            CreateTexture3D(device, desc);
            break;
    }

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D11Texture::SetDebugName(const char* name)
{
    D3D11SetObjectName(GetNative(), name);
    if (srv_)
        D3D11SetObjectNameSubscript(srv_.Get(), name, ".SRV");
    if (uav_)
        D3D11SetObjectNameSubscript(uav_.Get(), name, ".UAV");
}

Extent3D D3D11Texture::GetMipExtent(std::uint32_t mipLevel) const
{
    Extent3D size;

    if (ID3D11Resource* resource = native_.Get())
    {
        D3D11_RESOURCE_DIMENSION dimension;
        resource->GetType(&dimension);

        switch (dimension)
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                ComPtr<ID3D11Texture1D> tex1D;
                DXThrowIfCastFailed(native_.As(&tex1D), "ID3D11Texture1D");

                /* Query MIP-level size for 1D texture */
                D3D11_TEXTURE1D_DESC desc;
                tex1D->GetDesc(&desc);

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
                ComPtr<ID3D11Texture2D> tex2D;
                DXThrowIfCastFailed(native_.As(&tex2D), "ID3D11Texture2D");

                /* Query MIP-level size for 2D texture */
                D3D11_TEXTURE2D_DESC desc;
                tex2D->GetDesc(&desc);

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
                ComPtr<ID3D11Texture3D> tex3D;
                DXThrowIfCastFailed(native_.As(&tex3D), "ID3D11Texture3D");

                /* Query MIP-level size for 3D texture */
                D3D11_TEXTURE3D_DESC desc;
                tex3D->GetDesc(&desc);

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
    /* Initialize texture descriptor */
    TextureDescriptor texDesc;

    texDesc.type        = GetType();
    texDesc.bindFlags   = GetBindFlags();
    texDesc.miscFlags   = 0;

    /* Get resource dimension to query the respective D3D descriptor */
    D3D11_RESOURCE_DIMENSION dimension;
    native_->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            ComPtr<ID3D11Texture1D> tex1D;
            DXThrowIfCastFailed(native_.As(&tex1D), "ID3D11Texture1D");

            /* Query descriptor from 1D texture */
            D3D11_TEXTURE1D_DESC desc;
            tex1D->GetDesc(&desc);

            texDesc.format      = GetBaseFormat();
            texDesc.extent      = { desc.Width, 1u, 1u };
            texDesc.arrayLayers = desc.ArraySize;
            texDesc.mipLevels   = desc.MipLevels;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            ComPtr<ID3D11Texture2D> tex2D;
            DXThrowIfCastFailed(native_.As(&tex2D), "ID3D11Texture2D");

            /* Query descriptor from 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            tex2D->GetDesc(&desc);

            texDesc.format      = GetBaseFormat();
            texDesc.extent      = { desc.Width, desc.Height, 1u };
            texDesc.arrayLayers = desc.ArraySize;
            texDesc.mipLevels   = desc.MipLevels;
            texDesc.samples     = desc.SampleDesc.Count;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            ComPtr<ID3D11Texture3D> tex3D;
            DXThrowIfCastFailed(native_.As(&tex3D), "ID3D11Texture3D");

            /* Query descriptor from 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            tex3D->GetDesc(&desc);

            texDesc.format      = GetBaseFormat();
            texDesc.extent      = { desc.Width, desc.Height, desc.Depth };
            texDesc.mipLevels   = desc.MipLevels;
        }
        break;
    }

    return texDesc;
}

Format D3D11Texture::GetFormat() const
{
    return GetBaseFormat();
}

SubresourceFootprint D3D11Texture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    return CalcPackedSubresourceFootprint(GetType(), GetBaseFormat(), GetMipExtent(0), mipLevel, GetNumArrayLayers());
}

static ComPtr<ID3D11Texture1D> DXCreateTexture1D(
    ID3D11Device*                   device,
    const D3D11_TEXTURE1D_DESC&     desc,
    const D3D11_SUBRESOURCE_DATA*   initialData = nullptr)
{
    ComPtr<ID3D11Texture1D> tex1D;

    HRESULT hr = device->CreateTexture1D(&desc, initialData, tex1D.GetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Texture1D");

    return tex1D;
}

static ComPtr<ID3D11Texture2D> DXCreateTexture2D(
    ID3D11Device*                   device,
    const D3D11_TEXTURE2D_DESC&     desc,
    const D3D11_SUBRESOURCE_DATA*   initialData = nullptr)
{
    ComPtr<ID3D11Texture2D> tex2D;

    HRESULT hr = device->CreateTexture2D(&desc, initialData, tex2D.GetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Texture2D");

    return tex2D;
}

static ComPtr<ID3D11Texture3D> DXCreateTexture3D(
    ID3D11Device*                   device,
    const D3D11_TEXTURE3D_DESC&     desc,
    const D3D11_SUBRESOURCE_DATA*   initialData = nullptr)
{
    ComPtr<ID3D11Texture3D> tex3D;

    HRESULT hr = device->CreateTexture3D(&desc, initialData, tex3D.GetAddressOf());
    DXThrowIfCreateFailed(hr, "ID3D11Texture3D");

    return tex3D;
}

HRESULT D3D11Texture::UpdateSubresource(
    ID3D11DeviceContext*    context,
    UINT                    mipLevel,
    UINT                    baseArrayLayer,
    UINT                    numArrayLayers,
    const D3D11_BOX&        dstBox,
    const ImageView&        imageView,
    Report*                 report)
{
    /* Check if source image must be converted */
    Format format = GetBaseFormat();
    const auto& formatAttribs = GetFormatAttribs(format);

    /* Get destination subresource index */
    const Extent3D extent
    {
        dstBox.right  - dstBox.left,
        dstBox.bottom - dstBox.top,
        dstBox.back   - dstBox.front
    };

    const SubresourceCPUMappingLayout dataLayout = CalcSubresourceCPUMappingLayout(format, extent, numArrayLayers, imageView.format, imageView.dataType);

    if (imageView.dataSize < dataLayout.imageSize)
    {
        if (report != nullptr)
        {
            report->Errorf(
                "image data size (%zu) is too small to update subresource of D3D11 texture (%zu is required)",
                imageView.dataSize, dataLayout.imageSize
            );
        }
        return E_INVALIDARG;
    }

    DynamicByteArray intermediateData;
    const char* srcData = reinterpret_cast<const char*>(imageView.data);
    LLGL_ASSERT_PTR(srcData);

    if ((formatAttribs.flags & FormatFlags::IsCompressed) == 0 &&
        (formatAttribs.format != imageView.format || formatAttribs.dataType != imageView.dataType))
    {
        /* Convert image data (e.g. from RGB to RGBA), and redirect initial data to new buffer */
        intermediateData    = ConvertImageBuffer(imageView, formatAttribs.format, formatAttribs.dataType, LLGL_MAX_THREAD_COUNT);
        srcData             = intermediateData.get();
        LLGL_ASSERT(intermediateData.size() == dataLayout.subresourceSize);
    }

    /* Update subresource with specified image data */
    for_range(arrayLayer, numArrayLayers)
    {
        UINT dstSubresource = CalcSubresource(mipLevel, baseArrayLayer + arrayLayer);
        context->UpdateSubresource(
            native_.Get(),
            dstSubresource,
            &dstBox,
            srcData,
            dataLayout.rowStride,
            dataLayout.layerStride
        );
        srcData += dataLayout.layerStride;
    }

    return S_OK;
}

static void CreateD3D11TextureSubresourceCopyWithCPUAccess(
    ID3D11Device*                   device,
    ID3D11DeviceContext*            context,
    const ComPtr<ID3D11Resource>&   inTexture,
    UINT                            inTextureMipLevels,
    UINT                            inTextureArraySize,
    ComPtr<ID3D11Resource>&         outTexture,
    D3D11_USAGE                     outTextureUsage,
    UINT                            cpuAccessFlags,
    UINT                            srcFirstMipLevel,
    UINT                            srcFirstArrayLayer,
    const D3D11_BOX*                srcBox)
{
    D3D11_RESOURCE_DIMENSION dimension;
    inTexture->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            ComPtr<ID3D11Texture1D> tex1D;
            DXThrowIfCastFailed(inTexture.As(&tex1D), "ID3D11Texture1D");

            /* Create temporary 1D texture with a similar descriptor */
            D3D11_TEXTURE1D_DESC desc;
            tex1D->GetDesc(&desc);
            {
                if (srcBox != nullptr)
                {
                    /* Override dimension if source box is specified */
                    desc.Width      = (srcBox->right - srcBox->left);
                }
                desc.MipLevels      = 1;
                desc.ArraySize      = inTextureArraySize;
                desc.Usage          = outTextureUsage;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = 0;
            }
            outTexture = DXCreateTexture1D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            ComPtr<ID3D11Texture2D> tex2D;
            DXThrowIfCastFailed(inTexture.As(&tex2D), "ID3D11Texture2D");

            /* Query and modify descriptor for 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            tex2D->GetDesc(&desc);
            {
                if (srcBox != nullptr)
                {
                    /* Override dimension if source box is specified */
                    desc.Width      = (srcBox->right - srcBox->left);
                    desc.Height     = (srcBox->bottom - srcBox->top);
                    desc.ArraySize  = inTextureArraySize;
                }
                desc.MipLevels      = 1;
                desc.Usage          = outTextureUsage;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = 0; // Don't adopt D3D11_RESOURCE_MISC_TEXTURECUBE here for CPU access textures
            }
            outTexture = DXCreateTexture2D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            ComPtr<ID3D11Texture3D> tex3D;
            DXThrowIfCastFailed(inTexture.As(&tex3D), "ID3D11Texture3D");

            /* Query and modify descriptor for 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            tex3D->GetDesc(&desc);
            {
                if (srcBox != nullptr)
                {
                    /* Override dimension if source box is specified */
                    desc.Width      = (srcBox->right - srcBox->left);
                    desc.Height     = (srcBox->bottom - srcBox->top);
                    desc.Depth      = (srcBox->back - srcBox->front);
                }
                desc.MipLevels      = 1;
                desc.Usage          = outTextureUsage;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = 0;
            }
            outTexture = DXCreateTexture3D(device, desc);
        }
        break;
    }

    /* Copy subresource */
    for_range(arrayLayer, inTextureArraySize)
    {
        UINT dstSubresource = D3D11CalcSubresource(0, arrayLayer, 1);
        UINT srcSubresource = D3D11CalcSubresource(srcFirstMipLevel, srcFirstArrayLayer + arrayLayer, inTextureMipLevels);
        context->CopySubresourceRegion(outTexture.Get(), dstSubresource, 0, 0, 0, inTexture.Get(), srcSubresource, srcBox);
    }
}

void D3D11Texture::CreateSubresourceCopyWithCPUAccess(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    ComPtr<ID3D11Resource>& textureOutput,
    UINT                    cpuAccessFlags,
    const TextureRegion&    region)
{
    const Offset3D offset = CalcTextureOffset(GetType(), region.offset);
    const Extent3D extent = CalcTextureExtent(GetType(), region.extent);

    const D3D11_BOX srcBox
    {
        static_cast<UINT>(offset.x),
        static_cast<UINT>(offset.y),
        static_cast<UINT>(offset.z),
        static_cast<UINT>(offset.x) + extent.width,
        static_cast<UINT>(offset.y) + extent.height,
        static_cast<UINT>(offset.z) + extent.depth,
    };

    const bool isDepthStencilOrMultisampled = ((GetBindFlags() & BindFlags::DepthStencilAttachment) != 0 || IsMultiSampleTexture(GetType()));
    if (isDepthStencilOrMultisampled)
    {
        /* Copy texture into intermediate staging texture with same dimension */
        ComPtr<ID3D11Resource> intermediateTexture;
        CreateD3D11TextureSubresourceCopyWithCPUAccess(
            /*device:*/             device,
            /*context:*/            context,
            /*inTexture:*/          native_.Get(),
            /*inTextureMipLevels:*/ numMipLevels_,
            /*inTextureArraySize:*/ region.subresource.numArrayLayers,
            /*outTexture:*/         intermediateTexture,
            /*outTextureUsage:*/    D3D11_USAGE_DEFAULT,
            /*cpuAccessFlags:*/     0,
            /*srcFirstMipLevel:*/   region.subresource.baseMipLevel,
            /*srcFirstArrayLayer:*/ region.subresource.baseArrayLayer,
            /*srcBox:*/             nullptr
        );

        /* Copy intermediate texture into output texture */
        CreateD3D11TextureSubresourceCopyWithCPUAccess(
            /*device:*/             device,
            /*context:*/            context,
            /*inTexture:*/          intermediateTexture.Get(),
            /*inTextureMipLevels:*/ 1,
            /*inTextureArraySize:*/ region.subresource.numArrayLayers,
            /*outTexture:*/         textureOutput,
            /*outTextureUsage:*/    D3D11_USAGE_STAGING,
            /*cpuAccessFlags:*/     cpuAccessFlags,
            /*srcFirstMipLevel:*/   0,
            /*srcFirstArrayLayer:*/ 0,
            /*srcBox:*/             &srcBox
        );
    }
    else
    {
        CreateD3D11TextureSubresourceCopyWithCPUAccess(
            /*device:*/             device,
            /*context:*/            context,
            /*inTexture:*/          native_.Get(),
            /*inTextureMipLevels:*/ numMipLevels_,
            /*inTextureArraySize:*/ region.subresource.numArrayLayers,
            /*outTexture:*/         textureOutput,
            /*outTextureUsage:*/    D3D11_USAGE_STAGING,
            /*cpuAccessFlags:*/     cpuAccessFlags,
            /*srcFirstMipLevel:*/   region.subresource.baseMipLevel,
            /*srcFirstArrayLayer:*/ region.subresource.baseArrayLayer,
            /*srcBox:*/             &srcBox
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
        srvDesc.Format = DXTypes::ToDXGIFormatSRV(format);

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
                srvDesc.TextureCubeArray.NumCubes           = numArrayLayers / 6;
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
    HRESULT hr = device->CreateShaderResourceView(resource, &srvDesc, srvOutput);
    DXThrowIfCreateFailed(hr, "ID3D11ShaderResourceView", errorContextInfo);
}

static void CreateD3D11TextureSubresourceUAV(
    ID3D11Device*               device,
    ID3D11Resource*             resource,
    ID3D11UnorderedAccessView** uavOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        mipLevel,
    UINT                        baseArrayLayerOrSlice,
    UINT                        numArrayLayersOrSlices,
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
                uavDesc.Texture1D.MipSlice              = mipLevel;
                break;

            case TextureType::Texture2D:
                uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE2D;
                uavDesc.Texture2D.MipSlice              = mipLevel;
                break;

            case TextureType::Texture3D:
                uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE3D;
                uavDesc.Texture3D.MipSlice              = mipLevel;
                uavDesc.Texture3D.FirstWSlice           = baseArrayLayerOrSlice;
                uavDesc.Texture3D.WSize                 = numArrayLayersOrSlices;
                break;

            case TextureType::Texture1DArray:
                uavDesc.ViewDimension                   = D3D11_UAV_DIMENSION_TEXTURE1DARRAY;
                uavDesc.Texture1DArray.MipSlice         = mipLevel;
                uavDesc.Texture1DArray.FirstArraySlice  = baseArrayLayerOrSlice;
                uavDesc.Texture1DArray.ArraySize        = numArrayLayersOrSlices;
                break;

            case TextureType::Texture2DArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
                uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
                uavDesc.Texture2DArray.MipSlice         = mipLevel;
                uavDesc.Texture2DArray.FirstArraySlice  = baseArrayLayerOrSlice;
                uavDesc.Texture2DArray.ArraySize        = numArrayLayersOrSlices;
                break;

            case TextureType::Texture2DMS:
            case TextureType::Texture2DMSArray:
                /* ignore */
                break;
        }
    }
    HRESULT hr = device->CreateUnorderedAccessView(resource, &uavDesc, uavOutput);
    DXThrowIfCreateFailed(hr, "ID3D11UnorderedAccessView", errorContextInfo);
}

void D3D11Texture::CreateSubresourceCopyWithUIntFormat(
    ID3D11Device*               device,
    ComPtr<ID3D11Resource>&     textureOutput,
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
    DXGI_FORMAT                 format      = DXTypes::ToDXGIFormatUInt(GetBaseDXFormat());

    native_->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            /* Create temporary 1D texture with a similar descriptor */
            D3D11_TEXTURE1D_DESC desc;
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
            textureOutput = DXCreateTexture1D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            /* Query and modify descriptor for 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            {
                desc.Width          = region.extent.width;
                desc.Height         = region.extent.height;
                desc.MipLevels      = 1;
                desc.ArraySize      = region.subresource.numArrayLayers;
                desc.Format         = format;
                desc.SampleDesc     = { 1, 0 };
                desc.Usage          = D3D11_USAGE_DEFAULT;
                desc.BindFlags      = bindFlags;
                desc.CPUAccessFlags = 0;
                desc.MiscFlags      = 0; // Don't adopt D3D11_RESOURCE_MISC_TEXTURECUBE here for CPU access textures
            }
            textureOutput = DXCreateTexture2D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            /* Query and modify descriptor for 3D texture */
            D3D11_TEXTURE3D_DESC desc;
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
            textureOutput = DXCreateTexture3D(device, desc);
        }
        break;
    }

    /* Create SRV and UAV for entire subresource copy */
    if (srvOutput != nullptr)
    {
        const UINT numArrayLayers = region.subresource.numArrayLayers;
        CreateD3D11TextureSubresourceSRV(
            device,
            textureOutput.Get(),
            srvOutput,
            subresourceType,
            format,
            0,
            1,
            0,
            numArrayLayers,
            "for texture subresource copy"
        );
    }

    if (uavOutput != nullptr)
    {
        const UINT numArrayLayersOrSlices = (subresourceType == TextureType::Texture3D ? region.extent.depth : region.subresource.numArrayLayers);
        CreateD3D11TextureSubresourceUAV(
            device,
            textureOutput.Get(),
            uavOutput,
            subresourceType,
            format,
            0,
            0,
            numArrayLayersOrSlices,
            "for texture subresource copy"
        );
    }
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
    if (device == nullptr)
    {
        ComPtr<ID3D11Device> parentDevice;
        native_->GetDevice(parentDevice.GetAddressOf());
        CreateD3D11TextureSubresourceSRV(
            parentDevice.Get(),
            native_.Get(),
            srvOutput,
            type,
            format,
            baseMipLevel,
            numMipLevels,
            baseArrayLayer,
            numArrayLayers,
            __FUNCTION__
        );
    }
    else
    {
        CreateD3D11TextureSubresourceSRV(
            device,
            native_.Get(),
            srvOutput,
            type,
            format,
            baseMipLevel,
            numMipLevels,
            baseArrayLayer,
            numArrayLayers,
            __FUNCTION__
        );
    }
}

void D3D11Texture::CreateSubresourceUAV(
    ID3D11Device*               device,
    ID3D11UnorderedAccessView** uavOutput,
    const TextureType           type,
    const DXGI_FORMAT           format,
    UINT                        mipLevel,
    UINT                        baseArrayLayerOrSlice,
    UINT                        numArrayLayersOrSlices)
{
    if (device == nullptr)
    {
        ComPtr<ID3D11Device> parentDevice;
        native_->GetDevice(parentDevice.GetAddressOf());
        CreateD3D11TextureSubresourceUAV(
            parentDevice.Get(),
            native_.Get(),
            uavOutput,
            type,
            format,
            mipLevel,
            baseArrayLayerOrSlice,
            numArrayLayersOrSlices,
            __FUNCTION__
        );
    }
    else
    {
        CreateD3D11TextureSubresourceUAV(
            device,
            native_.Get(),
            uavOutput,
            type,
            format,
            mipLevel,
            baseArrayLayerOrSlice,
            numArrayLayersOrSlices,
            __FUNCTION__
        );
    }
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
            return D3D11_BOX
            {
                static_cast<UINT>(offset.x),
                0u,
                0u,
                static_cast<UINT>(offset.x) + extent.width,
                1u,
                1u
            };

        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            return D3D11_BOX
            {
                static_cast<UINT>(offset.x),
                static_cast<UINT>(offset.y),
                0u,
                static_cast<UINT>(offset.x) + extent.width,
                static_cast<UINT>(offset.y) + extent.height,
                1u
            };

        case TextureType::Texture3D:
            return D3D11_BOX
            {
                static_cast<UINT>(offset.x),
                static_cast<UINT>(offset.y),
                static_cast<UINT>(offset.z),
                static_cast<UINT>(offset.x) + extent.width,
                static_cast<UINT>(offset.y) + extent.height,
                static_cast<UINT>(offset.z) + extent.depth
            };

        default:
            return D3D11_BOX{ 0u, 0u, 0u, 0u, 0u, 0u };
    }
}

DXGI_FORMAT D3D11Texture::GetBaseDXFormat() const
{
    return DXTypes::ToDXGIFormat(GetBaseFormat());
}


/*
 * ====== Private: ======
 */

static DXGI_FORMAT SelectTextureDXGIFormat(const TextureDescriptor& desc)
{
    return DXTypes::SelectTextureDXGIFormat(desc.format, desc.bindFlags);
}

void D3D11Texture::CreateTexture1D(
    ID3D11Device*                   device,
    const TextureDescriptor&        desc,
    const D3D11_SUBRESOURCE_DATA*   initialData)
{
    /* Create native D3D texture */
    D3D11_TEXTURE1D_DESC descD3D;
    {
        descD3D.Width           = desc.extent.width;
        descD3D.MipLevels       = NumMipLevels(desc);
        descD3D.ArraySize       = desc.arrayLayers;
        descD3D.Format          = SelectTextureDXGIFormat(desc);
        descD3D.Usage           = DXGetTextureUsage(desc);
        descD3D.BindFlags       = DXGetTextureBindFlags(desc);
        descD3D.CPUAccessFlags  = DXGetCPUAccessFlagsForMiscFlags(desc.miscFlags);
        descD3D.MiscFlags       = DXGetTextureMiscFlags(desc);
    }
    native_ = DXCreateTexture1D(device, descD3D, initialData);

    /* Store parameters and create default resource views */
    SetResourceParams(descD3D.Format, Extent3D{ descD3D.Width, 1u, 1u }, descD3D.MipLevels, descD3D.ArraySize);
    CreateDefaultResourceViews(device, desc.bindFlags);
}

void D3D11Texture::CreateTexture2D(
    ID3D11Device*                   device,
    const TextureDescriptor&        desc,
    const D3D11_SUBRESOURCE_DATA*   initialData)
{
    /* Create native D3D texture */
    D3D11_TEXTURE2D_DESC descD3D;
    {
        descD3D.Width               = desc.extent.width;
        descD3D.Height              = desc.extent.height;
        descD3D.MipLevels           = NumMipLevels(desc);
        descD3D.ArraySize           = desc.arrayLayers;
        descD3D.Format              = SelectTextureDXGIFormat(desc);
        descD3D.SampleDesc.Count    = (IsMultiSampleTexture(desc.type) ? std::max(1u, desc.samples) : 1u);
        descD3D.SampleDesc.Quality  = 0;//(desc.miscFlags & MiscFlags::FixedSamples ? D3D11_CENTER_MULTISAMPLE_PATTERN : 0);
        descD3D.Usage               = DXGetTextureUsage(desc);
        descD3D.BindFlags           = DXGetTextureBindFlags(desc);
        descD3D.CPUAccessFlags      = DXGetCPUAccessFlagsForMiscFlags(desc.miscFlags);
        descD3D.MiscFlags           = DXGetTextureMiscFlags(desc);
    }
    native_ = DXCreateTexture2D(device, descD3D, initialData);

    /* Store parameters and create default resource views */
    SetResourceParams(descD3D.Format, Extent3D{ descD3D.Width, descD3D.Height, 1u }, descD3D.MipLevels, descD3D.ArraySize);
    CreateDefaultResourceViews(device, desc.bindFlags);
}

void D3D11Texture::CreateTexture3D(
    ID3D11Device*                   device,
    const TextureDescriptor&        desc,
    const D3D11_SUBRESOURCE_DATA*   initialData)
{
    /* Create native D3D texture */
    D3D11_TEXTURE3D_DESC descD3D;
    {
        descD3D.Width           = desc.extent.width;
        descD3D.Height          = desc.extent.height;
        descD3D.Depth           = desc.extent.depth;
        descD3D.MipLevels       = NumMipLevels(desc);
        descD3D.Format          = SelectTextureDXGIFormat(desc);
        descD3D.Usage           = DXGetTextureUsage(desc);
        descD3D.BindFlags       = DXGetTextureBindFlags(desc);
        descD3D.CPUAccessFlags  = DXGetCPUAccessFlagsForMiscFlags(desc.miscFlags);
        descD3D.MiscFlags       = DXGetTextureMiscFlags(desc);
    }
    native_ = DXCreateTexture3D(device, descD3D, initialData);

    /* Store parameters and create default resource views */
    SetResourceParams(descD3D.Format, Extent3D{ descD3D.Width, descD3D.Height, descD3D.Depth }, descD3D.MipLevels, 1);
    CreateDefaultResourceViews(device, desc.bindFlags);
}

void D3D11Texture::CreateDefaultResourceViews(ID3D11Device* device, long bindFlags)
{
    if ((bindFlags & BindFlags::Sampled) != 0)
        CreateDefaultSRV(device);
    if ((bindFlags & BindFlags::Storage) != 0)
        CreateDefaultUAV(device);
}

void D3D11Texture::CreateDefaultSRV(ID3D11Device* device)
{
    const bool hasTypelessFormat        = DXTypes::IsTypelessDXGIFormat(GetDXFormat());
    const bool hasDepthStencilFormat    = IsDepthOrStencilFormat(GetBaseFormat());
    if (hasTypelessFormat || hasDepthStencilFormat)
    {
        /* Create SRV with parameters for entire texture resource */
        CreateSubresourceSRV(device, srv_.ReleaseAndGetAddressOf(), GetType(), DXTypes::ToDXGIFormat(GetFormat()), 0, GetNumMipLevels(), 0, GetNumArrayLayers());
    }
    else
    {
        /* Create SRV with D3D default descriptor */
        HRESULT hr = device->CreateShaderResourceView(native_.Get(), nullptr, srv_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11ShaderResourceView", "for texture");
    }
}

void D3D11Texture::CreateDefaultUAV(ID3D11Device* device)
{
    const bool hasTypelessFormat        = DXTypes::IsTypelessDXGIFormat(GetDXFormat());
    const bool hasDepthStencilFormat    = IsDepthOrStencilFormat(GetBaseFormat());
    if (hasTypelessFormat || hasDepthStencilFormat)
    {
        /* Create UAV with parameters for entire texture resource */
        CreateSubresourceUAV(device, uav_.ReleaseAndGetAddressOf(), GetType(), DXTypes::ToDXGIFormat(GetFormat()), 0, 0, GetNumArrayLayers());
    }
    else
    {
        /* Create UAV with D3D default descriptor */
        HRESULT hr = device->CreateUnorderedAccessView(native_.Get(), nullptr, uav_.ReleaseAndGetAddressOf());
        DXThrowIfCreateFailed(hr, "ID3D11UnorderedAccessView", "for texture");
    }
}

void D3D11Texture::SetResourceParams(DXGI_FORMAT format, const Extent3D& extent, UINT mipLevels, UINT arraySize)
{
    format_         = format;
    numMipLevels_   = (mipLevels == 0 ? NumMipLevels(extent.width, extent.height, extent.height) : mipLevels);
    numArrayLayers_ = arraySize;
}


} // /namespace LLGL



// ================================================================================
