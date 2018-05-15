/*
 * D3D11Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Texture.h"
#include "../../DXCommon/DXCore.h"
#include "../D3D11Types.h"


namespace LLGL
{


D3D11Texture::D3D11Texture(const TextureType type) :
    Texture { type }
{
}

Extent3D D3D11Texture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    Extent3D size;

    if (hwTexture_.resource)
    {
        D3D11_RESOURCE_DIMENSION dimension;
        hwTexture_.resource->GetType(&dimension);

        switch (dimension)
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                /* Query MIP-level size for 1D texture */
                D3D11_TEXTURE1D_DESC desc;
                hwTexture_.tex1D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                {
                    size.width  = std::max(1u, desc.Width >> mipLevel);
                    size.height = 1u;
                    size.depth  = 1u;
                }
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                /* Query MIP-level size for 2D texture */
                D3D11_TEXTURE2D_DESC desc;
                hwTexture_.tex2D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                {
                    size.width  = std::max(1u, desc.Width  >> mipLevel);
                    size.height = std::max(1u, desc.Height >> mipLevel);
                    size.depth  = 1u;
                }
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                /* Query MIP-level size for 3D texture */
                D3D11_TEXTURE3D_DESC desc;
                hwTexture_.tex3D->GetDesc(&desc);

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

TextureDescriptor D3D11Texture::QueryDesc() const
{
    /* Get D3D hardware texture resource */
    const auto& hwTex = GetHwTexture();

    /* Initialize texture descriptor */
    TextureDescriptor texDesc;

    texDesc.type = GetType();

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

            texDesc.format              = D3D11Types::Unmap(desc.Format);
            texDesc.texture1D.width     = desc.Width;
            texDesc.texture1D.layers    = desc.ArraySize;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            /* Query descriptor from 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            hwTex.tex2D->GetDesc(&desc);

            texDesc.format              = D3D11Types::Unmap(desc.Format);
            texDesc.texture2D.width     = desc.Width;
            texDesc.texture2D.height    = desc.Height;
            texDesc.texture2D.layers    = desc.ArraySize;

            if (texDesc.type == TextureType::TextureCube || texDesc.type == TextureType::TextureCubeArray)
                texDesc.textureCube.layers /= 6;
            else if (texDesc.type == TextureType::Texture2DMS || texDesc.type == TextureType::Texture2DMSArray)
                texDesc.texture2DMS.samples = desc.SampleDesc.Count;
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            /* Query descriptor from 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            hwTex.tex3D->GetDesc(&desc);

            texDesc.format              = D3D11Types::Unmap(desc.Format);
            texDesc.texture3D.width     = desc.Width;
            texDesc.texture3D.height    = desc.Height;
            texDesc.texture3D.depth     = desc.Depth;
        }
        break;
    }

    return texDesc;
}

static ComPtr<ID3D11Texture1D> DXCreateTexture1D(
    ID3D11Device* device, const D3D11_TEXTURE1D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData = nullptr)
{
    ComPtr<ID3D11Texture1D> tex1D;
    auto hr = device->CreateTexture1D(&desc, initialData, &tex1D);
    DXThrowIfFailed(hr, "failed to create D3D11 1D-texture");
    return tex1D;
}

static ComPtr<ID3D11Texture2D> DXCreateTexture2D(
    ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData = nullptr)
{
    ComPtr<ID3D11Texture2D> tex2D;
    auto hr = device->CreateTexture2D(&desc, initialData, &tex2D);
    DXThrowIfFailed(hr, "failed to create D3D11 2D-texture");
    return tex2D;
}

static ComPtr<ID3D11Texture3D> DXCreateTexture3D(
    ID3D11Device* device, const D3D11_TEXTURE3D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData = nullptr)
{
    ComPtr<ID3D11Texture3D> tex3D;
    auto hr = device->CreateTexture3D(&desc, initialData, &tex3D);
    DXThrowIfFailed(hr, "failed to create D3D11 3D-texture");
    return tex3D;
}

void D3D11Texture::CreateTexture1D(
    ID3D11Device* device, const D3D11_TEXTURE1D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    hwTexture_.tex1D = DXCreateTexture1D(device, desc, initialData);
    CreateDefaultSRV(device, srvDesc);
    SetResourceParams(desc.Format, { desc.Width, 1u, 1u }, desc.ArraySize);
}

void D3D11Texture::CreateTexture2D(
    ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    hwTexture_.tex2D = DXCreateTexture2D(device, desc, initialData);
    CreateDefaultSRV(device, srvDesc);
    SetResourceParams(desc.Format, { desc.Width, desc.Height, 1u }, desc.ArraySize);
}

void D3D11Texture::CreateTexture3D(
    ID3D11Device* device, const D3D11_TEXTURE3D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    hwTexture_.tex3D = DXCreateTexture3D(device, desc, initialData);
    CreateDefaultSRV(device, srvDesc);
    SetResourceParams(desc.Format, { desc.Width, desc.Height, desc.Depth }, 1);
}

void D3D11Texture::UpdateSubresource(
    ID3D11DeviceContext* context, UINT mipSlice, UINT arraySlice, const D3D11_BOX& dstBox,
    const ImageDescriptor& imageDesc, std::size_t threadCount)
{
    /* Get destination subresource index */
    auto dstSubresource = D3D11CalcSubresource(mipSlice, arraySlice, numMipLevels_);
    auto srcPitch       = DataTypeSize(imageDesc.dataType) * ImageFormatSize(imageDesc.format);

    /* Check if source image must be converted */
    auto dstTexFormat = DXGetTextureFormatDesc(format_);

    if (dstTexFormat.format != imageDesc.format || dstTexFormat.dataType != imageDesc.dataType)
    {
        /* Get source data stride */
        auto srcRowPitch    = (dstBox.right - dstBox.left)*srcPitch;
        auto srcDepthPitch  = (dstBox.bottom - dstBox.top)*srcRowPitch;
        auto imageSize      = (dstBox.back - dstBox.front)*srcDepthPitch;

        /* Convert image data from RGB to RGBA */
        auto tempData = ConvertImageBuffer(
            imageDesc.format, imageDesc.dataType,
            imageDesc.data, imageSize,
            dstTexFormat.format, dstTexFormat.dataType,
            threadCount
        );

        /* Get new source data stride */
        srcPitch        = DataTypeSize(dstTexFormat.dataType) * ImageFormatSize(dstTexFormat.format);
        srcRowPitch     = (dstBox.right - dstBox.left)*srcPitch;
        srcDepthPitch   = (dstBox.bottom - dstBox.top)*srcRowPitch;

        /* Update subresource with specified image data */
        context->UpdateSubresource(
            hwTexture_.resource.Get(), dstSubresource,
            &dstBox, tempData.get(), srcRowPitch, srcDepthPitch
        );
    }
    else
    {
        /* Get source data stride */
        auto srcRowPitch    = (dstBox.right - dstBox.left)*srcPitch;
        auto srcDepthPitch  = (dstBox.bottom - dstBox.top)*srcRowPitch;

        /* Update subresource with specified image data */
        context->UpdateSubresource(
            hwTexture_.resource.Get(), dstSubresource,
            &dstBox, imageDesc.data, srcRowPitch, srcDepthPitch
        );
    }
}

void D3D11Texture::CreateSubresourceCopyWithCPUAccess(
    ID3D11Device* device, ID3D11DeviceContext* context,
    D3D11HardwareTexture& textureCopy, UINT cpuAccessFlags, UINT mipLevel) const
{
    D3D11_RESOURCE_DIMENSION dimension;
    hwTexture_.resource->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            /* Create temporary 1D texture with a similar descriptor */
            D3D11_TEXTURE1D_DESC desc;
            hwTexture_.tex1D->GetDesc(&desc);
            {
                desc.Width          = (desc.Width << mipLevel);
                desc.MipLevels      = 1;
                desc.Usage          = D3D11_USAGE_STAGING;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE);
            }
            textureCopy.tex1D = DXCreateTexture1D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
        {
            /* Query and modify descriptor for 2D texture */
            D3D11_TEXTURE2D_DESC desc;
            hwTexture_.tex2D->GetDesc(&desc);
            {
                desc.Width          = (desc.Width << mipLevel);
                desc.Height         = (desc.Height << mipLevel);
                desc.MipLevels      = 1;
                desc.Usage          = D3D11_USAGE_STAGING;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE);
            }
            textureCopy.tex2D = DXCreateTexture2D(device, desc);
        }
        break;

        case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
        {
            /* Query and modify descriptor for 3D texture */
            D3D11_TEXTURE3D_DESC desc;
            hwTexture_.tex3D->GetDesc(&desc);
            {
                desc.Width          = (desc.Width << mipLevel);
                desc.Height         = (desc.Height << mipLevel);
                desc.Depth          = (desc.Depth << mipLevel);
                desc.MipLevels      = 1;
                desc.Usage          = D3D11_USAGE_STAGING;
                desc.BindFlags      = 0;
                desc.CPUAccessFlags = cpuAccessFlags;
                desc.MiscFlags      = (desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE);
            }
            textureCopy.tex3D = DXCreateTexture3D(device, desc);
        }
        break;
    }

    /* Copy subresource */
    context->CopySubresourceRegion(
        textureCopy.resource.Get(),
        0,
        0, 0, 0, // DstX, DstY, DstZ
        hwTexture_.resource.Get(),
        D3D11CalcSubresource(mipLevel, 0, numMipLevels_),
        nullptr
    );
}

void D3D11Texture::CreateSubresourceSRV(
    ID3D11Device* device, ID3D11ShaderResourceView** srvOutput,
    UINT baseMipLevel, UINT numMipLevels, UINT baseArrayLayer, UINT numArrayLayers)
{
    /* Create SRV for subresource */
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    {
        srvDesc.Format          = format_;
        srvDesc.ViewDimension   = D3D11Types::Map(GetType());

        switch (srvDesc.ViewDimension)
        {
            case D3D11_SRV_DIMENSION_TEXTURE1D:
                srvDesc.Texture1D.MostDetailedMip           = baseMipLevel;
                srvDesc.Texture1D.MipLevels                 = numMipLevels;
                break;

            case D3D11_SRV_DIMENSION_TEXTURE2D:
                srvDesc.Texture2D.MostDetailedMip           = baseMipLevel;
                srvDesc.Texture2D.MipLevels                 = numMipLevels;
                break;

            case D3D11_SRV_DIMENSION_TEXTURE3D:
                srvDesc.Texture3D.MostDetailedMip           = baseMipLevel;
                srvDesc.Texture3D.MipLevels                 = numMipLevels;
                break;

            case D3D11_SRV_DIMENSION_TEXTURECUBE:
                srvDesc.TextureCube.MostDetailedMip         = baseMipLevel;
                srvDesc.TextureCube.MipLevels               = numMipLevels;
                break;

            case D3D11_SRV_DIMENSION_TEXTURE1DARRAY:
                srvDesc.Texture1DArray.MostDetailedMip      = baseMipLevel;
                srvDesc.Texture1DArray.MipLevels            = numMipLevels;
                srvDesc.Texture1DArray.FirstArraySlice      = baseArrayLayer;
                srvDesc.Texture1DArray.ArraySize            = numArrayLayers;
                break;

            case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
                srvDesc.Texture2DArray.MostDetailedMip      = baseMipLevel;
                srvDesc.Texture2DArray.MipLevels            = numMipLevels;
                srvDesc.Texture2DArray.FirstArraySlice      = baseArrayLayer;
                srvDesc.Texture2DArray.ArraySize            = numArrayLayers;
                break;

            case D3D11_SRV_DIMENSION_TEXTURECUBEARRAY:
                srvDesc.TextureCubeArray.MostDetailedMip    = baseMipLevel;
                srvDesc.TextureCubeArray.MipLevels          = numMipLevels;
                srvDesc.TextureCubeArray.First2DArrayFace   = baseArrayLayer;
                srvDesc.TextureCubeArray.NumCubes           = numArrayLayers;
                break;

            case D3D11_SRV_DIMENSION_TEXTURE2DMS:
                break;

            case D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY:
                srvDesc.Texture2DMSArray.FirstArraySlice    = baseArrayLayer;
                srvDesc.Texture2DMSArray.ArraySize          = numArrayLayers;
                break;
        }
    }
    auto hr = device->CreateShaderResourceView(hwTexture_.resource.Get(), &srvDesc, srvOutput);
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resouce-view (SRV) for texture subresource");
}


/*
 * ====== Private: ======
 */

void D3D11Texture::CreateDefaultSRV(ID3D11Device* device, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    /* Create internal SRV for entire texture resource */
    auto hr = device->CreateShaderResourceView(
        hwTexture_.resource.Get(),
        srvDesc,
        srv_.ReleaseAndGetAddressOf()
    );
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resouce-view (SRV) for texture");
}

void D3D11Texture::SetResourceParams(DXGI_FORMAT format, const Extent3D& size, UINT arraySize)
{
    format_         = format;
    numMipLevels_   = NumMipLevels(size.width, size.height, size.height);
    numArrayLayers_ = arraySize;
}


} // /namespace LLGL



// ================================================================================
