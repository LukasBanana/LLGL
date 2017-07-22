/*
 * D3D11RenderSystem_Textures.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderSystem.h"
#include "D3D11Types.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../Assertion.h"
#include "../../Core/Helper.h"


namespace LLGL
{


/* ----- Textures ----- */

Texture* D3D11RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc)
{
    /* Create texture object and store type */
    auto texture = MakeUnique<D3D11Texture>(textureDesc.type);
    
    /* Modify number of layers */
    auto descD3D = textureDesc;

    switch (descD3D.type)
    {
        case TextureType::Texture1D:
            descD3D.texture1D.layers = 1;
            break;
        case TextureType::Texture2D:
            descD3D.texture2D.layers = 1;
            break;
        case TextureType::TextureCube:
            descD3D.textureCube.layers = 6;
            break;
        case TextureType::TextureCubeArray:
            descD3D.textureCube.layers *= 6;
            break;
        case TextureType::Texture2DMS:
            descD3D.texture2DMS.layers = 1;
            break;
        default:
            break;
    }

    /* Bulid generic texture */
    switch (descD3D.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            BuildGenericTexture1D(*texture, descD3D, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            BuildGenericTexture2D(*texture, descD3D, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
            break;
        case TextureType::Texture3D:
            BuildGenericTexture3D(*texture, descD3D, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS);
            break;
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            BuildGenericTexture2D(*texture, descD3D, imageDesc, D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE);
            break;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            BuildGenericTexture2DMS(*texture, descD3D);
            break;
        default:
            throw std::invalid_argument("failed to create texture with invalid texture type");
            break;
    }
    
    return TakeOwnership(textures_, std::move(texture));
}

TextureArray* D3D11RenderSystem::CreateTextureArray(unsigned int numTextures, Texture* const * textureArray)
{
    AssertCreateTextureArray(numTextures, textureArray);
    return TakeOwnership(textureArrays_, MakeUnique<D3D11TextureArray>(numTextures, textureArray));
}

void D3D11RenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

void D3D11RenderSystem::Release(TextureArray& textureArray)
{
    RemoveFromUniqueSet(textureArrays_, &textureArray);
}

TextureDescriptor D3D11RenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    /* Get D3D hardware texture resource */
    auto& textureD3D = LLGL_CAST(const D3D11Texture&, texture);
    const auto& hwTex = textureD3D.GetHardwareTexture();

    /* Initialize texture descriptor */
    TextureDescriptor texDesc;

    texDesc.type = texture.GetType();

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

void D3D11RenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    /* Determine update region */
    Gs::Vector3ui position, size;

    switch (texture.GetType())
    {
        case TextureType::Texture1D:
            position.x  = subTextureDesc.texture1D.x;
            size.x      = subTextureDesc.texture1D.width;
            size.y      = 1;
            size.z      = 1;
            break;

        case TextureType::Texture2D:
            position.x  = subTextureDesc.texture2D.x;
            position.y  = subTextureDesc.texture2D.y;
            size.x      = subTextureDesc.texture2D.width;
            size.y      = subTextureDesc.texture2D.height;
            size.z      = 1;
            break;

        case TextureType::Texture3D:
            position.x  = subTextureDesc.texture3D.x;
            position.y  = subTextureDesc.texture3D.y;
            position.z  = subTextureDesc.texture3D.z;
            size.x      = subTextureDesc.texture3D.width;
            size.y      = subTextureDesc.texture3D.height;
            size.z      = subTextureDesc.texture3D.depth;
            break;

        case TextureType::TextureCube:
            position.x  = subTextureDesc.textureCube.x;
            position.y  = subTextureDesc.textureCube.y;
            position.z  = static_cast<unsigned int>(subTextureDesc.textureCube.cubeFaceOffset);
            size.x      = subTextureDesc.textureCube.width;
            size.y      = subTextureDesc.textureCube.height;
            size.z      = 1;
            break;

        case TextureType::Texture1DArray:
            position.x  = subTextureDesc.texture1D.x;
            position.y  = subTextureDesc.texture1D.layerOffset;
            position.z  = 0;
            size.x      = subTextureDesc.texture1D.width;
            size.y      = subTextureDesc.texture1D.layers;
            size.z      = 1;
            break;

        case TextureType::Texture2DArray:
            position.x  = subTextureDesc.texture2D.x;
            position.y  = subTextureDesc.texture2D.y;
            position.z  = subTextureDesc.texture2D.layerOffset;
            size.x      = subTextureDesc.texture2D.width;
            size.y      = subTextureDesc.texture2D.height;
            size.z      = subTextureDesc.texture2D.layers;
            break;

        case TextureType::TextureCubeArray:
            position.x  = subTextureDesc.textureCube.x;
            position.y  = subTextureDesc.textureCube.y;
            position.z  = subTextureDesc.textureCube.layerOffset * 6 + static_cast<unsigned int>(subTextureDesc.textureCube.cubeFaceOffset);
            size.x      = subTextureDesc.textureCube.width;
            size.y      = subTextureDesc.textureCube.height;
            size.z      = subTextureDesc.textureCube.cubeFaces;
            break;

        default:
            /* Ignore multi-sample textures */
            return;
    }

    /* Update generic texture at determined region */
    UpdateGenericTexture(texture, subTextureDesc.mipLevel, 0, position, size, imageDesc);
}

void D3D11RenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer)
{
    LLGL_ASSERT_PTR(buffer);
    auto& textureD3D = LLGL_CAST(const D3D11Texture&, texture);

    /* Create a copy of the hardware texture with CPU read access */
    D3D11HardwareTexture hwTextureCopy;
    textureD3D.CreateSubresourceCopyWithCPUAccess(device_.Get(), context_.Get(), hwTextureCopy, D3D11_CPU_ACCESS_READ, mipLevel);
    
    /* Map subresource for reading */
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    auto hr = context_->Map(hwTextureCopy.resource.Get(), 0, D3D11_MAP_READ, 0, &mappedSubresource);
    DXThrowIfFailed(hr, "failed to map D3D11 texture copy resource");

    /* Query MIP-level size to determine image buffer size */
    auto size = texture.QueryMipLevelSize(mipLevel);

    /* Check if image buffer must be converted */
    auto srcTexFormat   = DXGetTextureFormatDesc(textureD3D.GetFormat());
    auto srcPitch       = DataTypeSize(srcTexFormat.dataType) * ImageFormatSize(srcTexFormat.format);
    auto srcImageSize   = (size.x*size.y*size.z * srcPitch);

    if (srcTexFormat.format != imageFormat || srcTexFormat.dataType != dataType)
    {
        /* Convert mapped data into requested format */
        auto tempData = ConvertImageBuffer(
            srcTexFormat.format, srcTexFormat.dataType,
            mappedSubresource.pData, srcImageSize,
            imageFormat, dataType,
            GetConfiguration().threadCount
        );

        /* Copy temporary data into output buffer */
        auto dstPitch       = DataTypeSize(dataType) * ImageFormatSize(imageFormat);
        auto dstImageSize   = (size.x*size.y*size.z * dstPitch);
        ::memcpy(buffer, tempData.get(), dstImageSize);
    }
    else
    {
        /* Copy mapped data directly into the output buffer */
        ::memcpy(buffer, mappedSubresource.pData, srcImageSize);
    }

    /* Unmap resource */
    context_->Unmap(hwTextureCopy.resource.Get(), 0);
}

void D3D11RenderSystem::GenerateMips(Texture& texture)
{
    /* Generate MIP-maps for SRV of specified texture */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    context_->GenerateMips(textureD3D.GetSRV());
}


/*
 * ======= Private: =======
 */

void D3D11RenderSystem::BuildGenericTexture1D(
    D3D11Texture& textureD3D, const TextureDescriptor& descD3D, const ImageDescriptor* imageDesc, UINT miscFlags)
{
    /* Setup D3D texture descriptor */
    D3D11_TEXTURE1D_DESC texDesc;
    {
        texDesc.Width           = descD3D.texture1D.width;
        texDesc.MipLevels       = 0;
        texDesc.ArraySize       = descD3D.texture1D.layers;
        texDesc.Format          = D3D11Types::Map(descD3D.format);
        texDesc.Usage           = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags       = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags  = 0;
        texDesc.MiscFlags       = miscFlags;
    }

    /* Create D3D texture resource */
    textureD3D.CreateTexture1D(device_.Get(), texDesc);

    if (imageDesc)
    {
        /* Update only the first MIP-map level for each array slice */
        auto subImageDesc = *imageDesc;
        auto subImageStride = descD3D.texture1D.width * ImageFormatSize(subImageDesc.format) * DataTypeSize(subImageDesc.dataType);

        for (unsigned int arraySlice = 0; arraySlice < descD3D.texture1D.layers; ++arraySlice)
        {
            textureD3D.UpdateSubresource(
                context_.Get(), 0, 0,
                CD3D11_BOX(0, 0, 0, descD3D.texture1D.width, 1, 1),
                subImageDesc, GetConfiguration().threadCount
            );

            subImageDesc.buffer = reinterpret_cast<const char*>(subImageDesc.buffer) + subImageStride;
        }
    }
    else
    {
        //TODO -> fill texture with default data
    }
}

void D3D11RenderSystem::BuildGenericTexture2D(
    D3D11Texture& textureD3D, const TextureDescriptor& descD3D, const ImageDescriptor* imageDesc, UINT miscFlags)
{
    /* Setup D3D texture descriptor and create D3D texture resouce */
    D3D11_TEXTURE2D_DESC texDesc;
    {
        texDesc.Width               = descD3D.texture2D.width;
        texDesc.Height              = descD3D.texture2D.height;
        texDesc.MipLevels           = 0;
        texDesc.ArraySize           = descD3D.texture2D.layers;
        texDesc.Format              = D3D11Types::Map(descD3D.format);
        texDesc.SampleDesc.Count    = 1;
        texDesc.SampleDesc.Quality  = 0;
        texDesc.Usage               = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags      = 0;
        texDesc.MiscFlags           = miscFlags;
    }
    textureD3D.CreateTexture2D(device_.Get(), texDesc);

    if (imageDesc)
    {
        /* Update only the first MIP-map level for each array slice */
        auto subImageDesc = *imageDesc;
        auto subImageStride = descD3D.texture2D.width * descD3D.texture2D.height * subImageDesc.GetElementSize();

        for (unsigned int arraySlice = 0; arraySlice < descD3D.texture2D.layers; ++arraySlice)
        {
            textureD3D.UpdateSubresource(
                context_.Get(), 0, arraySlice,
                CD3D11_BOX(0, 0, 0, descD3D.texture2D.width, descD3D.texture2D.height, 1),
                subImageDesc, GetConfiguration().threadCount
            );

            subImageDesc.buffer = reinterpret_cast<const char*>(subImageDesc.buffer) + subImageStride;
        }
    }
    else if (GetConfiguration().defaultImageEnabled)
    {
        if (IsDepthStencilFormat(descD3D.format))
        {
            //TODO -> fill texture with default depth
            //GetConfiguration().defaultImageColor
        }
        else
        {
            //TODO -> fill texture with default color
            //GetConfiguration().defaultImageDepth
        }
    }
}

void D3D11RenderSystem::BuildGenericTexture3D(
    D3D11Texture& textureD3D, const TextureDescriptor& descD3D, const ImageDescriptor* imageDesc, UINT miscFlags)
{
    /* Setup D3D texture descriptor and create D3D texture resouce */
    D3D11_TEXTURE3D_DESC texDesc;
    {
        texDesc.Width           = descD3D.texture3D.width;
        texDesc.Height          = descD3D.texture3D.height;
        texDesc.Depth           = descD3D.texture3D.depth;
        texDesc.MipLevels       = 0;
        texDesc.Format          = D3D11Types::Map(descD3D.format);
        texDesc.Usage           = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags       = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags  = 0;
        texDesc.MiscFlags       = miscFlags;
    }
    textureD3D.CreateTexture3D(device_.Get(), texDesc);

    if (imageDesc)
    {
        textureD3D.UpdateSubresource(
            context_.Get(), 0, 0,
            CD3D11_BOX(0, 0, 0, descD3D.texture3D.width, descD3D.texture3D.height, descD3D.texture3D.depth),
            *imageDesc, GetConfiguration().threadCount
        );
    }
    else
    {
        //TODO -> fill texture with default data
    }
}

void D3D11RenderSystem::BuildGenericTexture2DMS(D3D11Texture& textureD3D, const TextureDescriptor& descD3D)
{
    /* Setup D3D texture descriptor and create D3D texture resouce */
    D3D11_TEXTURE2D_DESC texDesc;
    {
        texDesc.Width               = descD3D.texture2DMS.width;
        texDesc.Height              = descD3D.texture2DMS.height;
        texDesc.MipLevels           = 1;
        texDesc.ArraySize           = descD3D.texture2DMS.layers;
        texDesc.Format              = D3D11Types::Map(descD3D.format);
        texDesc.SampleDesc.Count    = descD3D.texture2DMS.samples;
        texDesc.SampleDesc.Quality  = 0;
        texDesc.Usage               = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        texDesc.CPUAccessFlags      = 0;
        texDesc.MiscFlags           = 0;
    }
    textureD3D.CreateTexture2D(device_.Get(), texDesc);
}

void D3D11RenderSystem::UpdateGenericTexture(
    Texture& texture, unsigned int mipLevel, unsigned int layer,
    const Gs::Vector3ui& position, const Gs::Vector3ui& size, const ImageDescriptor& imageDesc)
{
    /* Get D3D texture and update subresource */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.UpdateSubresource(
        context_.Get(), static_cast<UINT>(mipLevel), layer,
        CD3D11_BOX(
            position.x, position.y, position.z,
            position.x + size.x, position.y + size.y, position.z + size.z
        ),
        imageDesc, GetConfiguration().threadCount
    );
}


} // /namespace LLGL



// ================================================================================
