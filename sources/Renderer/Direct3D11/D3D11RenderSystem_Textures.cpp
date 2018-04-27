/*
 * D3D11RenderSystem_Textures.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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
            BuildGenericTexture1D(*texture, descD3D, imageDesc);
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            BuildGenericTexture2D(*texture, descD3D, imageDesc);
            break;
        case TextureType::Texture3D:
            BuildGenericTexture3D(*texture, descD3D, imageDesc);
            break;
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            BuildGenericTexture2D(*texture, descD3D, imageDesc);
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

TextureArray* D3D11RenderSystem::CreateTextureArray(std::uint32_t numTextures, Texture* const * textureArray)
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
            position.z  = static_cast<std::uint32_t>(subTextureDesc.textureCube.cubeFaceOffset);
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
            position.z  = subTextureDesc.textureCube.layerOffset * 6 + static_cast<std::uint32_t>(subTextureDesc.textureCube.cubeFaceOffset);
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

static void ValidateImageDataSize(std::size_t dataSize, std::size_t requiredDataSize)
{
    if (dataSize < requiredDataSize)
        throw std::invalid_argument("output image data buffer too small for texture read operation");
}

void D3D11RenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, ImageFormat imageFormat, DataType dataType, void* data, std::size_t dataSize)
{
    LLGL_ASSERT_PTR(data);
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
        /* Determine destination image size */
        auto dstPitch       = DataTypeSize(dataType) * ImageFormatSize(imageFormat);
        auto dstImageSize   = (size.x*size.y*size.z * dstPitch);

        /* Validate input size */
        ValidateImageDataSize(dataSize, dstImageSize);

        /* Convert mapped data into requested format */
        auto tempData = ConvertImageBuffer(
            srcTexFormat.format, srcTexFormat.dataType,
            mappedSubresource.pData, srcImageSize,
            imageFormat, dataType,
            GetConfiguration().threadCount
        );

        /* Copy temporary data into output buffer */
        ::memcpy(data, tempData.get(), dstImageSize);
    }
    else
    {
        /* Validate input size */
        ValidateImageDataSize(dataSize, srcImageSize);

        /* Copy mapped data directly into the output buffer */
        ::memcpy(data, mappedSubresource.pData, srcImageSize);
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

static UINT GetDXTextureMipLevels(const TextureDescriptor& desc)
{
    return ((desc.flags & TextureFlags::GenerateMips) != 0 ? 0 : 1);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476203(v=vs.85).aspx
static UINT GetDXTextureBindFlags(const TextureDescriptor& desc)
{
    UINT flags = D3D11_BIND_SHADER_RESOURCE;

    /* Render target binding flag is required for MIP-map generation and render target attachment */
    if ((desc.flags & TextureFlags::GenerateMips) != 0 || (desc.flags & TextureFlags::AttachmentUsage) != 0)
        flags |= D3D11_BIND_RENDER_TARGET;

    return flags;
}

static UINT GetDXTextureMiscFlags(const TextureDescriptor& desc)
{
    UINT flags = 0;

    if ((desc.flags & TextureFlags::GenerateMips) != 0)
        flags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
    if (IsCubeTexture(desc.type))
        flags |= D3D11_RESOURCE_MISC_TEXTURECUBE;

    return flags;
}

void D3D11RenderSystem::BuildGenericTexture1D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    /* Setup D3D texture descriptor and create D3D texture resouce */
    D3D11_TEXTURE1D_DESC descDX;
    {
        descDX.Width            = desc.texture1D.width;
        descDX.MipLevels        = GetDXTextureMipLevels(desc);
        descDX.ArraySize        = desc.texture1D.layers;
        descDX.Format           = D3D11Types::Map(desc.format);
        descDX.Usage            = D3D11_USAGE_DEFAULT;
        descDX.BindFlags        = GetDXTextureBindFlags(desc);
        descDX.CPUAccessFlags   = 0;
        descDX.MiscFlags        = GetDXTextureMiscFlags(desc);
    }
    textureD3D.CreateTexture1D(device_.Get(), descDX);

    /* Initialize texture image data */
    InitializeGpuTexture(
        textureD3D, desc.format, imageDesc,
        desc.texture1D.width, 1, 1, desc.texture1D.layers
    );
}

void D3D11RenderSystem::BuildGenericTexture2D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    /* Setup D3D texture descriptor and create D3D texture resouce */
    D3D11_TEXTURE2D_DESC descDX;
    {
        descDX.Width                = desc.texture2D.width;
        descDX.Height               = desc.texture2D.height;
        descDX.MipLevels            = GetDXTextureMipLevels(desc);
        descDX.ArraySize            = desc.texture2D.layers;
        descDX.Format               = D3D11Types::Map(desc.format);
        descDX.SampleDesc.Count     = 1;
        descDX.SampleDesc.Quality   = 0;
        descDX.Usage                = D3D11_USAGE_DEFAULT;
        descDX.BindFlags            = GetDXTextureBindFlags(desc);
        descDX.CPUAccessFlags       = 0;
        descDX.MiscFlags            = GetDXTextureMiscFlags(desc);
    }
    textureD3D.CreateTexture2D(device_.Get(), descDX);

    /* Initialize texture image data */
    InitializeGpuTexture(
        textureD3D, desc.format, imageDesc,
        desc.texture2D.width, desc.texture2D.height, 1, desc.texture2D.layers
    );
}

void D3D11RenderSystem::BuildGenericTexture3D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    /* Setup D3D texture descriptor and create D3D texture resouce */
    D3D11_TEXTURE3D_DESC descDX;
    {
        descDX.Width            = desc.texture3D.width;
        descDX.Height           = desc.texture3D.height;
        descDX.Depth            = desc.texture3D.depth;
        descDX.MipLevels        = GetDXTextureMipLevels(desc);
        descDX.Format           = D3D11Types::Map(desc.format);
        descDX.Usage            = D3D11_USAGE_DEFAULT;
        descDX.BindFlags        = GetDXTextureBindFlags(desc);
        descDX.CPUAccessFlags   = 0;
        descDX.MiscFlags        = GetDXTextureMiscFlags(desc);
    }
    textureD3D.CreateTexture3D(device_.Get(), descDX);

    /* Initialize texture image data */
    InitializeGpuTexture(
        textureD3D, desc.format, imageDesc,
        desc.texture3D.width, desc.texture3D.height, desc.texture3D.depth, 1
    );
}

void D3D11RenderSystem::BuildGenericTexture2DMS(D3D11Texture& textureD3D, const TextureDescriptor& desc)
{
    /* Setup D3D texture descriptor and create D3D texture resouce */
    D3D11_TEXTURE2D_DESC descDX;
    {
        descDX.Width                = desc.texture2DMS.width;
        descDX.Height               = desc.texture2DMS.height;
        descDX.MipLevels            = 1;
        descDX.ArraySize            = desc.texture2DMS.layers;
        descDX.Format               = D3D11Types::Map(desc.format);
        descDX.SampleDesc.Count     = desc.texture2DMS.samples;
        descDX.SampleDesc.Quality   = 0;//(descD3D.texture2DMS.fixedSamples ? D3D11_CENTER_MULTISAMPLE_PATTERN : 0);
        descDX.Usage                = D3D11_USAGE_DEFAULT;
        descDX.BindFlags            = GetDXTextureBindFlags(desc);
        descDX.CPUAccessFlags       = 0;
        descDX.MiscFlags            = 0;
    }
    textureD3D.CreateTexture2D(device_.Get(), descDX);
}

void D3D11RenderSystem::UpdateGenericTexture(
    Texture& texture, std::uint32_t mipLevel, std::uint32_t layer,
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

void D3D11RenderSystem::InitializeGpuTexture(
    D3D11Texture& textureD3D, const TextureFormat format, const ImageDescriptor* imageDesc,
    std::uint32_t width, std::uint32_t height, std::uint32_t depth, std::uint32_t numLayers)
{
    if (imageDesc)
    {
        /* Initialize texture with specified image descriptor */
        InitializeGpuTextureWithImage(textureD3D, format, *imageDesc, width, height, depth, numLayers);
    }
    else if (GetConfiguration().imageInitialization.enabled && !IsDepthStencilFormat(format))
    {
        /* Initialize texture with default image data */
        InitializeGpuTextureWithDefault(textureD3D, format, width, height, depth, numLayers);
    }
}

void D3D11RenderSystem::InitializeGpuTextureWithImage(
    D3D11Texture& textureD3D, const TextureFormat format, ImageDescriptor imageDesc,
    std::uint32_t width, std::uint32_t height, std::uint32_t depth, std::uint32_t numLayers)
{
    /* Update only the first MIP-map level for each array slice */
    const auto layerStride = width * height * depth * imageDesc.GetElementSize();

    for (std::uint32_t arraySlice = 0; arraySlice < numLayers; ++arraySlice)
    {
        textureD3D.UpdateSubresource(
            context_.Get(), 0, arraySlice,
            CD3D11_BOX(0, 0, 0, width, height, depth),
            imageDesc, GetConfiguration().threadCount
        );

        imageDesc.data = reinterpret_cast<const char*>(imageDesc.data) + layerStride;
    }
}

void D3D11RenderSystem::InitializeGpuTextureWithDefault(
    D3D11Texture& textureD3D, const TextureFormat format,
    std::uint32_t width, std::uint32_t height, std::uint32_t depth, std::uint32_t numLayers)
{
    /* Find suitable image format for texture hardware format */
    ImageDescriptor imageDescDefault;
    if (FindSuitableImageFormat(format, imageDescDefault.format, imageDescDefault.dataType))
    {
        /* Generate default image buffer */
        const auto fillColor = GetConfiguration().imageInitialization.color.Cast<double>();
        const auto imageSize = width * height * depth;

        auto imageBuffer = GenerateImageBuffer(imageDescDefault.format, imageDescDefault.dataType, imageSize, fillColor);

        /* Update only the first MIP-map level for each array slice */
        imageDescDefault.data = imageBuffer.get();

        for (std::uint32_t arraySlice = 0; arraySlice < numLayers; ++arraySlice)
        {
            textureD3D.UpdateSubresource(
                context_.Get(), 0, arraySlice,
                CD3D11_BOX(0, 0, 0, width, height, depth),
                imageDescDefault, GetConfiguration().threadCount
            );
        }
    }
}


} // /namespace LLGL



// ================================================================================
