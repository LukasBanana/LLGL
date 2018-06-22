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


static NSUInteger GetTextureWidth(const TextureDescriptor& desc)
{
    return desc.texture3D.width;
}

static NSUInteger GetTextureHeight(const TextureDescriptor& desc)
{
    if (desc.type == TextureType::Texture1D || desc.type == TextureType::Texture1DArray)
        return 1u;
    else
        return desc.texture3D.height;
}

static NSUInteger GetTextureDepth(const TextureDescriptor& desc)
{
    if (desc.type == TextureType::Texture3D)
        return desc.texture3D.width;
    else
        return 1u;
}

static MTLTextureUsage GetTextureUsage(const TextureDescriptor& desc)
{
    MTLTextureUsage usage = 0;
    
    if ((desc.flags & TextureFlags::SampleUsage) != 0)
        usage |= MTLTextureUsageShaderRead;
    if ((desc.flags & TextureFlags::AttachmentUsage) != 0)
        usage |= MTLTextureUsageRenderTarget;
    if ((desc.flags & TextureFlags::StorageUsage) != 0)
        usage |= MTLTextureUsageShaderWrite;
    
    return usage;
}

static void Convert(MTLTextureDescriptor* dst, const TextureDescriptor& src)
{
    dst.textureType         = MTTypes::ToMTLTextureType(src.type);
    dst.pixelFormat         = MTTypes::ToMTLPixelFormat(src.format);
    dst.width               = GetTextureWidth(src);
    dst.height              = GetTextureHeight(src);
    dst.depth               = GetTextureDepth(src);
    dst.mipmapLevelCount    = static_cast<NSUInteger>(NumMipLevels(src));
    dst.sampleCount         = static_cast<NSUInteger>(std::max(1u, src.texture2DMS.samples));
    dst.arrayLength         = static_cast<NSUInteger>(NumArrayLayers(src));
    dst.usage               = GetTextureUsage(src);
}

MTTexture::MTTexture(id<MTLDevice> device, const TextureDescriptor& desc) :
    Texture { desc.type }
{
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    Convert(texDesc, desc);
    native_ = [device newTextureWithDescriptor:texDesc];
}

MTTexture::~MTTexture()
{
    [native_ release];
}

Extent3D MTTexture::QueryMipLevelSize(std::uint32_t mipLevel) const
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

TextureDescriptor MTTexture::QueryDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type                = GetType();
    texDesc.flags               = 0;
    texDesc.format              = MTTypes::ToFormat([native_ pixelFormat]);
    
    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            texDesc.texture1D.width     = static_cast<std::uint32_t>([native_ width]);
            texDesc.texture1D.layers    = static_cast<std::uint32_t>([native_ arrayLength]);
            break;
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            texDesc.texture2D.width     = static_cast<std::uint32_t>([native_ width]);
            texDesc.texture2D.height    = static_cast<std::uint32_t>([native_ height]);
            texDesc.texture2D.layers    = static_cast<std::uint32_t>([native_ arrayLength]);
            break;
        case TextureType::Texture3D:
            texDesc.texture3D.width     = static_cast<std::uint32_t>([native_ width]);
            texDesc.texture3D.height    = static_cast<std::uint32_t>([native_ height]);
            texDesc.texture3D.depth     = static_cast<std::uint32_t>([native_ depth]);
            break;
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            texDesc.textureCube.width   = static_cast<std::uint32_t>([native_ width]);
            texDesc.textureCube.height  = static_cast<std::uint32_t>([native_ height]);
            texDesc.textureCube.layers  = static_cast<std::uint32_t>([native_ arrayLength]) / 6;
            break;
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            texDesc.texture2DMS.width   = static_cast<std::uint32_t>([native_ width]);
            texDesc.texture2DMS.height  = static_cast<std::uint32_t>([native_ height]);
            texDesc.texture2DMS.layers  = static_cast<std::uint32_t>([native_ arrayLength]);
            break;
    }

    return texDesc;
}

void MTTexture::Write(const SrcImageDescriptor& imageDesc, const Offset3D& offset, const Extent3D& extent)
{
    //TODO: convert data if necessary
    
    /* Replace region of native texture with source image data */
    MTLRegion region;
    MTTypes::Convert(region.origin, offset);
    MTTypes::Convert(region.size, extent);
    
    auto format = MTTypes::ToFormat([native_ pixelFormat]);

    [native_
        replaceRegion:  region
        mipmapLevel:    0
        withBytes:      imageDesc.data
        bytesPerRow:    ([native_ width] * FormatBitSize(format) / 8)
    ];
}


/*
 * ======= Private: =======
 */


} // /namespace LLGL



// ================================================================================
