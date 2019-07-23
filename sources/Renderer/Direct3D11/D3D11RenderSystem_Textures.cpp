/*
 * D3D11RenderSystem_Textures.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderSystem.h"
#include "D3D11Types.h"
#include "../DXCommon/DXCore.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"


namespace LLGL
{


/* ----- Textures ----- */

Texture* D3D11RenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    /* Create texture object and store type */
    auto texture = MakeUnique<D3D11Texture>(textureDesc.type);

    /* Bulid generic texture */
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            CreateAndInitializeGpuTexture1D(*texture, textureDesc, imageDesc);
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            CreateAndInitializeGpuTexture2D(*texture, textureDesc, imageDesc);
            break;
        case TextureType::Texture3D:
            CreateAndInitializeGpuTexture3D(*texture, textureDesc, imageDesc);
            break;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            CreateAndInitializeGpuTexture2DMS(*texture, textureDesc);
            break;
        default:
            throw std::invalid_argument("failed to create texture with invalid texture type");
            break;
    }

    return TakeOwnership(textures_, std::move(texture));
}

void D3D11RenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

void D3D11RenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    switch (texture.GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            UpdateGenericTexture(
                texture,
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                CD3D11_BOX(
                    textureRegion.offset.x,
                    0,
                    0,
                    textureRegion.offset.x + static_cast<LONG>(textureRegion.extent.width),
                    static_cast<LONG>(textureRegion.subresource.numArrayLayers),
                    1
                ),
                imageDesc
            );
            break;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            UpdateGenericTexture(
                texture,
                textureRegion.subresource.baseMipLevel,
                textureRegion.subresource.baseArrayLayer,
                CD3D11_BOX(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    0,
                    textureRegion.offset.x + static_cast<LONG>(textureRegion.extent.width),
                    textureRegion.offset.y + static_cast<LONG>(textureRegion.extent.height),
                    static_cast<LONG>(textureRegion.subresource.numArrayLayers)
                ),
                imageDesc
            );
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            break;

        case TextureType::Texture3D:
            UpdateGenericTexture(
                texture,
                textureRegion.subresource.baseMipLevel,
                0,
                CD3D11_BOX(
                    textureRegion.offset.x,
                    textureRegion.offset.y,
                    textureRegion.offset.z,
                    textureRegion.offset.x + static_cast<LONG>(textureRegion.extent.width),
                    textureRegion.offset.y + static_cast<LONG>(textureRegion.extent.height),
                    textureRegion.offset.z + static_cast<LONG>(textureRegion.extent.depth)
                ),
                imageDesc
            );
            break;
    }
}

static void ValidateImageDataSize(std::size_t dataSize, std::size_t requiredDataSize)
{
    if (dataSize < requiredDataSize)
        throw std::invalid_argument("output image data buffer too small for texture read operation");
}

void D3D11RenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc)
{
    LLGL_ASSERT_PTR(imageDesc.data);
    auto& textureD3D = LLGL_CAST(const D3D11Texture&, texture);

    /* Create a copy of the hardware texture with CPU read access */
    D3D11NativeTexture texCopy;
    textureD3D.CreateSubresourceCopyWithCPUAccess(device_.Get(), context_.Get(), texCopy, D3D11_CPU_ACCESS_READ, mipLevel);

    /* Map subresource for reading */
    D3D11_MAPPED_SUBRESOURCE mappedSubresource;
    auto hr = context_->Map(texCopy.resource.Get(), 0, D3D11_MAP_READ, 0, &mappedSubresource);
    DXThrowIfFailed(hr, "failed to map D3D11 texture copy resource");

    /* Query MIP-level size to determine image buffer size */
    auto size           = texture.QueryMipExtent(mipLevel);
    auto numTexels      = (size.width * size.height * size.depth);

    /* Check if image buffer must be converted */
    auto srcTexFormat   = DXGetTextureFormatDesc(textureD3D.GetFormat());
    auto srcPitch       = DataTypeSize(srcTexFormat.dataType) * ImageFormatSize(srcTexFormat.format);
    auto srcImageSize   = (numTexels * srcPitch);

    if (srcTexFormat.format != imageDesc.format || srcTexFormat.dataType != imageDesc.dataType)
    {
        /* Determine destination image size */
        auto dstPitch       = DataTypeSize(imageDesc.dataType) * ImageFormatSize(imageDesc.format);
        auto dstImageSize   = (numTexels * dstPitch);

        /* Validate input size */
        ValidateImageDataSize(imageDesc.dataSize, dstImageSize);

        /* Convert mapped data into requested format */
        auto tempData = ConvertImageBuffer(
            SrcImageDescriptor { srcTexFormat.format, srcTexFormat.dataType, mappedSubresource.pData, srcImageSize },
            imageDesc.format, imageDesc.dataType, GetConfiguration().threadCount
        );

        /* Copy temporary data into output buffer */
        ::memcpy(imageDesc.data, tempData.get(), dstImageSize);
    }
    else
    {
        /* Validate input size */
        ValidateImageDataSize(imageDesc.dataSize, srcImageSize);

        /* Copy mapped data directly into the output buffer */
        ::memcpy(imageDesc.data, mappedSubresource.pData, srcImageSize);
    }

    /* Unmap resource */
    context_->Unmap(texCopy.resource.Get(), 0);
}

void D3D11RenderSystem::GenerateMips(Texture& texture)
{
    /* Generate MIP-maps for the default SRV */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    if (auto srv = textureD3D.GetSRV())
    {
        /* Generate MIP-maps for default SRV */
        context_->GenerateMips(srv);
    }
    else
    {
        /* Generate MIP-maps with a temporary subresource SRV */
        GenerateMipsWithSubresourceSRV(textureD3D, 0, textureD3D.GetNumMipLevels(), 0, textureD3D.GetNumArrayLayers());
    }
}

void D3D11RenderSystem::GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers)
{
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);

    if ( baseMipLevel        == 0                              &&
         numMipLevels        == textureD3D.GetNumMipLevels()   &&
         baseArrayLayer      == 0                              &&
         numArrayLayers      == textureD3D.GetNumArrayLayers() &&
         textureD3D.GetSRV() != nullptr )
    {
        /* Generate MIP-maps for the default SRV */
        context_->GenerateMips(textureD3D.GetSRV());
    }
    else
    {
        /* Generate MIP-maps for a subresource SRV */
        GenerateMipsWithSubresourceSRV(textureD3D, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);
    }
}


/*
 * ======= Private: =======
 */

void D3D11RenderSystem::CreateAndInitializeGpuTexture1D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    /* Create native texture and initialize with image data */
    textureD3D.CreateTexture1D(device_.Get(), desc);
    InitializeGpuTexture(textureD3D, desc.format, imageDesc, desc.extent, desc.arrayLayers);
}

void D3D11RenderSystem::CreateAndInitializeGpuTexture2D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    /* Create native texture and initialize with image data */
    textureD3D.CreateTexture2D(device_.Get(), desc);
    InitializeGpuTexture(textureD3D, desc.format, imageDesc, desc.extent, desc.arrayLayers);
}

void D3D11RenderSystem::CreateAndInitializeGpuTexture3D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    /* Create native texture and initialize with image data */
    textureD3D.CreateTexture3D(device_.Get(), desc);
    InitializeGpuTexture(textureD3D, desc.format, imageDesc, desc.extent, 1);
}

void D3D11RenderSystem::CreateAndInitializeGpuTexture2DMS(D3D11Texture& textureD3D, const TextureDescriptor& desc)
{
    /* Create native texture */
    textureD3D.CreateTexture2D(device_.Get(), desc);
}

void D3D11RenderSystem::UpdateGenericTexture(
    Texture&                    texture,
    std::uint32_t               mipLevel,
    std::uint32_t               arrayLayer,
    const D3D11_BOX&            region,
    const SrcImageDescriptor&   imageDesc)
{
    /* Get D3D texture and update subresource */
    auto& textureD3D = LLGL_CAST(D3D11Texture&, texture);
    textureD3D.UpdateSubresource(
        context_.Get(),
        mipLevel,
        arrayLayer,
        region,
        imageDesc,
        GetConfiguration().threadCount
    );
}

void D3D11RenderSystem::InitializeGpuTexture(
    D3D11Texture&               textureD3D,
    const Format                format,
    const SrcImageDescriptor*   imageDesc,
    const Extent3D&             extent,
    std::uint32_t               arrayLayers)
{
    if (imageDesc)
    {
        /* Initialize texture with specified image descriptor */
        InitializeGpuTextureWithImage(textureD3D, format, *imageDesc, extent, arrayLayers);
    }
    else if (GetConfiguration().imageInitialization.enabled && !IsDepthStencilFormat(format))
    {
        /* Initialize texture with default image data */
        InitializeGpuTextureWithDefault(textureD3D, format, extent, arrayLayers);
    }
}

void D3D11RenderSystem::InitializeGpuTextureWithImage(
    D3D11Texture&       textureD3D,
    const Format        format,
    SrcImageDescriptor  imageDesc,
    const Extent3D&     extent,
    std::uint32_t       arrayLayers)
{
    /* Update only the first MIP-map level for each array layer */
    const auto bytesPerLayer =
    (
        extent.width                        *
        extent.height                       *
        extent.depth                        *
        ImageFormatSize(imageDesc.format)   *
        DataTypeSize(imageDesc.dataType)
    );

    /* Remap image data size for a single array layer to update each subresource individually */
    if (imageDesc.dataSize % arrayLayers != 0)
        throw std::invalid_argument("image data size is not a multiple of the layer count for D3D11 texture");

    imageDesc.dataSize /= arrayLayers;

    for (std::uint32_t layer = 0; layer < arrayLayers; ++layer)
    {
        /* Update subresource of current array layer */
        textureD3D.UpdateSubresource(
            context_.Get(),
            0, // mipLevel
            layer,
            CD3D11_BOX(0, 0, 0, extent.width, extent.height, extent.depth),
            imageDesc,
            GetConfiguration().threadCount
        );

        /* Move to next region of initial data */
        imageDesc.data = (reinterpret_cast<const std::int8_t*>(imageDesc.data) + bytesPerLayer);
    }
}

void D3D11RenderSystem::InitializeGpuTextureWithDefault(
    D3D11Texture&   textureD3D,
    const Format    format,
    const Extent3D& extent,
    std::uint32_t   arrayLayers)
{
    /* Find suitable image format for texture hardware format */
    SrcImageDescriptor imageDescDefault;
    if (FindSuitableImageFormat(format, imageDescDefault.format, imageDescDefault.dataType))
    {
        const auto& cfg = GetConfiguration();

        /* Generate default image buffer */
        const auto fillColor = cfg.imageInitialization.clearValue.color.Cast<double>();
        const auto imageSize = extent.width * extent.height * extent.depth;

        auto imageBuffer = GenerateImageBuffer(imageDescDefault.format, imageDescDefault.dataType, imageSize, fillColor);

        /* Update only the first MIP-map level for each array slice */
        imageDescDefault.data = imageBuffer.get();

        for (std::uint32_t layer = 0; layer < arrayLayers; ++layer)
        {
            textureD3D.UpdateSubresource(
                context_.Get(),
                0,
                layer,
                CD3D11_BOX(0, 0, 0, extent.width, extent.height, extent.depth),
                imageDescDefault,
                cfg.threadCount
            );
        }
    }
}

void D3D11RenderSystem::GenerateMipsWithSubresourceSRV(
    D3D11Texture& textureD3D,
    std::uint32_t baseMipLevel,
    std::uint32_t numMipLevels,
    std::uint32_t baseArrayLayer,
    std::uint32_t numArrayLayers)
{
    /* Generate MIP-maps for a subresource SRV */
    ComPtr<ID3D11ShaderResourceView> srv;
    textureD3D.CreateSubresourceSRV(device_.Get(), srv.GetAddressOf(), baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);
    context_->GenerateMips(srv.Get());
}


} // /namespace LLGL



// ================================================================================
