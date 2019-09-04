/*
 * MTTexture.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTTexture.h"
#include "../MTTypes.h"
#include <LLGL/TextureFlags.h>
#include <LLGL/ImageFlags.h>
#include <algorithm>


namespace LLGL
{


static NSUInteger GetTextureLayers(const TextureDescriptor& desc)
{
    if (IsCubeTexture(desc.type))
        return desc.arrayLayers / 6;
    else
        return desc.arrayLayers;
}

static MTLTextureUsage GetTextureUsage(const TextureDescriptor& desc)
{
    MTLTextureUsage usage = 0;

    if ((desc.bindFlags & BindFlags::Sampled) != 0)
        usage |= MTLTextureUsageShaderRead;
    if ((desc.bindFlags & BindFlags::Storage) != 0)
        usage |= MTLTextureUsageShaderWrite;
    if ((desc.bindFlags & (BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment)) != 0)
        usage |= MTLTextureUsageRenderTarget;

    return usage;
}

static MTLResourceOptions GetResourceOptions(const TextureDescriptor& desc)
{
    MTLResourceOptions opt = 0;

    if (IsDepthStencilFormat(desc.format))
        opt |= MTLResourceStorageModePrivate;
    else
        opt |= MTLResourceStorageModeManaged;

    return opt;
}

static void Convert(MTLTextureDescriptor* dst, const TextureDescriptor& src)
{
    dst.textureType         = MTTypes::ToMTLTextureType(src.type);
    dst.pixelFormat         = MTTypes::ToMTLPixelFormat(src.format);
    dst.width               = src.extent.width;
    dst.height              = src.extent.height;
    dst.depth               = src.extent.depth;
    dst.mipmapLevelCount    = static_cast<NSUInteger>(NumMipLevels(src));
    dst.sampleCount         = static_cast<NSUInteger>(std::max(1u, src.samples));
    dst.arrayLength         = GetTextureLayers(src);
    dst.usage               = GetTextureUsage(src);
    dst.resourceOptions     = GetResourceOptions(src);
}

MTTexture::MTTexture(id<MTLDevice> device, const TextureDescriptor& desc) :
    Texture { desc.type }
{
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    Convert(texDesc, desc);
    native_ = [device newTextureWithDescriptor:texDesc];
    [texDesc release];
}

MTTexture::~MTTexture()
{
    [native_ release];
}

Extent3D MTTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    auto w = static_cast<std::uint32_t>([native_ width]);
    auto h = static_cast<std::uint32_t>([native_ height]);
    auto d = static_cast<std::uint32_t>([native_ depth]);
    auto a = static_cast<std::uint32_t>([native_ arrayLength]);

    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            w = std::max(1u, w >> mipLevel);
            h = a;
            d = 1u;
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = a;
            break;
        case TextureType::Texture3D:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = std::max(1u, d >> mipLevel);
            break;
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = a * 6;
            break;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            w = std::max(1u, w >> mipLevel);
            h = std::max(1u, h >> mipLevel);
            d = a;
            break;
    }

    return Extent3D { w, h, d };
}

TextureDescriptor MTTexture::GetDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type            = GetType();
    texDesc.bindFlags       = 0;
    texDesc.cpuAccessFlags  = 0;
    texDesc.miscFlags       = 0;
    texDesc.mipLevels       = static_cast<std::uint32_t>([native_ mipmapLevelCount]);
    texDesc.format          = MTTypes::ToFormat([native_ pixelFormat]);
    texDesc.extent.width    = static_cast<std::uint32_t>([native_ width]);
    texDesc.extent.height   = static_cast<std::uint32_t>([native_ height]);
    texDesc.extent.depth    = static_cast<std::uint32_t>([native_ depth]);
    texDesc.arrayLayers     = static_cast<std::uint32_t>([native_ arrayLength]);
    texDesc.samples         = static_cast<std::uint32_t>([native_ sampleCount]);

    if (IsCubeTexture(GetType()))
        texDesc.arrayLayers *= 6;

    return texDesc;
}

void MTTexture::Write(const TextureRegion& textureRegion, SrcImageDescriptor imageDesc)
{
    /* Convert region to MTLRegion */
    MTLRegion region;
    MTTypes::Convert(region.origin, textureRegion.offset);
    MTTypes::Convert(region.size, textureRegion.extent);

    /* Get dimensions */
    auto        format          = MTTypes::ToFormat([native_ pixelFormat]);
    const auto& formatAttribs   = GetFormatAttribs(format);
    NSUInteger  bytesPerRow     = (region.size.width * formatAttribs.bitSize) / (formatAttribs.blockWidth * 8);
    NSUInteger  bytesPerSlice   = (region.size.height * bytesPerRow) / formatAttribs.blockHeight;

    /* Check if image data must be converted */
    ByteBuffer intermediateData;

    if (formatAttribs.bitSize > 0 && (formatAttribs.flags & FormatFlags::IsCompressed) == 0)
    {
        /* Convert image format (will be null if no conversion is necessary) */
        intermediateData = ConvertImageBuffer(imageDesc, formatAttribs.format, formatAttribs.dataType, /*cfg.threadCount*/0);
        if (intermediateData)
        {
            /* User converted tempoary buffer as image source */
            imageDesc.data = intermediateData.get();
        }
    }

    /* Replace region of native texture with source image data */
    auto byteAlignedData = reinterpret_cast<const std::int8_t*>(imageDesc.data);

    for (NSUInteger arrayLayer = 0; arrayLayer < textureRegion.subresource.numArrayLayers; ++arrayLayer)
    {
        [native_
            replaceRegion:  region
            mipmapLevel:    static_cast<NSUInteger>(textureRegion.subresource.baseMipLevel)
            slice:          (textureRegion.subresource.baseArrayLayer + arrayLayer)
            withBytes:      byteAlignedData
            bytesPerRow:    bytesPerRow
            bytesPerImage:  bytesPerSlice
        ];
        byteAlignedData += bytesPerSlice;
    }
}

id<MTLTexture> MTTexture::CreateSubresourceView(const TextureSubresource& subresource)
{
    NSUInteger firstLevel   = static_cast<NSUInteger>(subresource.baseMipLevel);
    NSUInteger numLevels    = static_cast<NSUInteger>(subresource.numMipLevels);
    NSUInteger firstSlice   = static_cast<NSUInteger>(subresource.baseArrayLayer);
    NSUInteger numSlices    = static_cast<NSUInteger>(subresource.numArrayLayers);

    return [native_
        newTextureViewWithPixelFormat:  [native_ pixelFormat]
        textureType:                    [native_ textureType]
        levels:                         NSMakeRange(firstLevel, numLevels)
        slices:                         NSMakeRange(firstSlice, numSlices)
    ];
}


} // /namespace LLGL



// ================================================================================
