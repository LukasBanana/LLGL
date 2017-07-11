/*
 * D3D11Texture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Texture.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D11Texture::D3D11Texture(const TextureType type) :
    Texture { type }
{
}

Gs::Vector3ui D3D11Texture::QueryMipLevelSize(unsigned int mipLevel) const
{
    Gs::Vector3ui size;

    if (hardwareTexture_.resource)
    {
        D3D11_RESOURCE_DIMENSION dimension;
        hardwareTexture_.resource->GetType(&dimension);

        switch (dimension)
        {
            case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
            {
                /* Query MIP-level size for 1D texture */
                D3D11_TEXTURE1D_DESC desc;
                hardwareTexture_.tex1D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width >> mipLevel), 1, 1);
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
            {
                /* Query MIP-level size for 2D texture */
                D3D11_TEXTURE2D_DESC desc;
                hardwareTexture_.tex2D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width >> mipLevel), (desc.Height >> mipLevel), 1);
            }
            break;

            case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
            {
                /* Query MIP-level size for 3D texture */
                D3D11_TEXTURE3D_DESC desc;
                hardwareTexture_.tex3D->GetDesc(&desc);

                if (mipLevel < desc.MipLevels)
                    size = Gs::Vector3ui((desc.Width >> mipLevel), (desc.Height >> mipLevel), (desc.Depth >> mipLevel));
            }
            break;
        }
    }

    return size;
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
    hardwareTexture_.tex1D = DXCreateTexture1D(device, desc, initialData);
    CreateSRVAndStoreSettings(device, desc.Format, { desc.Width, 1, 1 }, srvDesc);
}

void D3D11Texture::CreateTexture2D(
    ID3D11Device* device, const D3D11_TEXTURE2D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    hardwareTexture_.tex2D = DXCreateTexture2D(device, desc, initialData);
    CreateSRVAndStoreSettings(device, desc.Format, { desc.Width, desc.Height, 1 }, srvDesc);
}

void D3D11Texture::CreateTexture3D(
    ID3D11Device* device, const D3D11_TEXTURE3D_DESC& desc, const D3D11_SUBRESOURCE_DATA* initialData, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    hardwareTexture_.tex3D = DXCreateTexture3D(device, desc, initialData);
    CreateSRVAndStoreSettings(device, desc.Format, { desc.Width, desc.Height, desc.Depth }, srvDesc);
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
            imageDesc.buffer, imageSize,
            dstTexFormat.format, dstTexFormat.dataType,
            threadCount
        );

        /* Get new source data stride */
        srcPitch        = DataTypeSize(dstTexFormat.dataType) * ImageFormatSize(dstTexFormat.format);
        srcRowPitch     = (dstBox.right - dstBox.left)*srcPitch;
        srcDepthPitch   = (dstBox.bottom - dstBox.top)*srcRowPitch;

        /* Update subresource with specified image data */
        context->UpdateSubresource(
            hardwareTexture_.resource.Get(), dstSubresource,
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
            hardwareTexture_.resource.Get(), dstSubresource,
            &dstBox, imageDesc.buffer, srcRowPitch, srcDepthPitch
        );
    }
}

void D3D11Texture::CreateSubresourceCopyWithCPUAccess(
    ID3D11Device* device, ID3D11DeviceContext* context,
    D3D11HardwareTexture& textureCopy, UINT cpuAccessFlags, unsigned int mipLevel) const
{
    D3D11_RESOURCE_DIMENSION dimension;
    hardwareTexture_.resource->GetType(&dimension);

    switch (dimension)
    {
        case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
        {
            /* Create temporary 1D texture with a similar descriptor */
            D3D11_TEXTURE1D_DESC desc;
            hardwareTexture_.tex1D->GetDesc(&desc);
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
            hardwareTexture_.tex2D->GetDesc(&desc);
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
            hardwareTexture_.tex3D->GetDesc(&desc);
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
        hardwareTexture_.resource.Get(),
        D3D11CalcSubresource(mipLevel, 0, numMipLevels_),
        nullptr
    );
}


/*
 * ====== Private: ======
 */

void D3D11Texture::CreateSRV(ID3D11Device* device, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    auto hr = device->CreateShaderResourceView(hardwareTexture_.resource.Get(), srvDesc, srv_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 shader-resouce-view (SRV) for texture");
}

void D3D11Texture::CreateSRVAndStoreSettings(
    ID3D11Device* device, DXGI_FORMAT format, const Gs::Vector3ui& size, const D3D11_SHADER_RESOURCE_VIEW_DESC* srvDesc)
{
    /* Create SRV for D3D texture */
    CreateSRV(device, srvDesc);

    /* Store format and number of MIP-maps */
    format_         = format;
    numMipLevels_   = NumMipLevels(size.x, size.y, size.z);
}


} // /namespace LLGL



// ================================================================================
