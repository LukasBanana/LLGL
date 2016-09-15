/*
 * TextureFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/TextureFlags.h>
#include <cmath>


namespace LLGL
{


LLGL_EXPORT std::size_t ColorFormatSize(const ColorFormat colorFormat)
{
    switch (colorFormat)
    {
        case ColorFormat::Gray:         return 1;
        case ColorFormat::GrayAlpha:    return 2;
        case ColorFormat::RGB:          return 3;
        case ColorFormat::BGR:          return 3;
        case ColorFormat::RGBA:         return 4;
        case ColorFormat::BGRA:         return 4;
        case ColorFormat::Depth:        return 1;
        case ColorFormat::DepthStencil: return 2;
        case ColorFormat::Compressed:   return 0;
    }
    return 0;
}

LLGL_EXPORT int NumMipLevels(const Gs::Vector3i& textureSize)
{
    auto maxSize = std::max(textureSize.x, std::max(textureSize.y, textureSize.z));
    auto log2Size = static_cast<int>(std::log2(maxSize));
    return (1 + log2Size);
}

LLGL_EXPORT bool IsCompressedFormat(const TextureFormat format)
{
    return (format >= TextureFormat::RGB_DXT1);
}


} // /namespace LLGL



// ================================================================================
