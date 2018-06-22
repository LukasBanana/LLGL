/*
 * MTTexture.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTTexture.h"
#include "../MTTypes.h"
#include <LLGL/TextureFlags.h>
#include <algorithm>


namespace LLGL
{


static void Convert(MTLTextureDescriptor* dst, const TextureDescriptor& src)
{
    //TODO
}

MTTexture::MTTexture(id<MTLDevice> device, const TextureDescriptor& desc, const TextureType type) :
    Texture { type }
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


/*
 * ======= Private: =======
 */


} // /namespace LLGL



// ================================================================================
