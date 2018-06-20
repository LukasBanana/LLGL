/*
 * TextureFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/TextureFlags.h>
#include <algorithm>
#include <cmath>


namespace LLGL
{


LLGL_EXPORT std::uint32_t NumMipLevels(std::uint32_t width, std::uint32_t height, std::uint32_t depth)
{
    auto maxSize = std::max({ width, height, depth });
    auto log2Size = static_cast<std::uint32_t>(std::log2(maxSize));
    return (1 + log2Size);
}

LLGL_EXPORT std::uint32_t NumMipLevels(const TextureDescriptor& textureDesc)
{
    if ((textureDesc.flags & TextureFlags::GenerateMips) != 0)
    {
        switch (textureDesc.type)
        {
            case TextureType::Texture1D:        return NumMipLevels(textureDesc.texture1D.width);
            case TextureType::Texture2D:        return NumMipLevels(textureDesc.texture2D.width, textureDesc.texture2D.height);
            case TextureType::Texture3D:        return NumMipLevels(textureDesc.texture3D.width, textureDesc.texture3D.height, textureDesc.texture3D.depth);
            case TextureType::TextureCube:      return NumMipLevels(textureDesc.textureCube.width, textureDesc.textureCube.height);
            case TextureType::Texture1DArray:   return NumMipLevels(textureDesc.texture1D.width);
            case TextureType::Texture2DArray:   return NumMipLevels(textureDesc.texture2D.width, textureDesc.texture2D.height);
            case TextureType::TextureCubeArray: return NumMipLevels(textureDesc.textureCube.width, textureDesc.textureCube.height);
            case TextureType::Texture2DMS:      return 1u;
            case TextureType::Texture2DMSArray: return 1u;
        }
    }
    return 1u;
}

LLGL_EXPORT std::uint32_t NumArrayLayers(const TextureDescriptor& textureDesc)
{
    switch (textureDesc.type)
    {
        case TextureType::Texture1DArray:   return textureDesc.texture1D.layers;
        case TextureType::Texture2DArray:   return textureDesc.texture2D.layers;
        case TextureType::TextureCubeArray: return textureDesc.textureCube.layers;
        case TextureType::Texture2DMSArray: return textureDesc.texture2DMS.layers;
        default:                            return 1u;
    }
}

std::uint32_t TextureBufferSize(const Format format, std::uint32_t numTexels)
{
    return ((FormatBitSize(format) * numTexels) / 8);
}

LLGL_EXPORT std::uint32_t TextureSize(const TextureDescriptor& textureDesc)
{
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:        return textureDesc.texture1D.width;
        case TextureType::Texture2D:        return textureDesc.texture2D.width * textureDesc.texture2D.height;
        case TextureType::Texture3D:        return textureDesc.texture3D.width * textureDesc.texture3D.height * textureDesc.texture3D.depth;
        case TextureType::TextureCube:      return textureDesc.textureCube.width * textureDesc.textureCube.height * 6;
        case TextureType::Texture1DArray:   return textureDesc.texture1D.width * textureDesc.texture1D.layers;
        case TextureType::Texture2DArray:   return textureDesc.texture2D.width * textureDesc.texture2D.height * textureDesc.texture2D.layers;
        case TextureType::TextureCubeArray: return textureDesc.textureCube.width * textureDesc.textureCube.height * 6 * textureDesc.textureCube.layers;
        case TextureType::Texture2DMS:      return textureDesc.texture2D.width * textureDesc.texture2D.height;
        case TextureType::Texture2DMSArray: return textureDesc.texture2D.width * textureDesc.texture2D.height * textureDesc.texture2D.layers;
        default:                            return 0;
    }
}

LLGL_EXPORT bool IsArrayTexture(const TextureType type)
{
    return (type >= TextureType::Texture1DArray && type != TextureType::Texture2DMS);
}

LLGL_EXPORT bool IsMultiSampleTexture(const TextureType type)
{
    return (type >= TextureType::Texture2DMS);
}

LLGL_EXPORT bool IsCubeTexture(const TextureType type)
{
    return (type == TextureType::TextureCube || type == TextureType::TextureCubeArray);
}


} // /namespace LLGL



// ================================================================================
