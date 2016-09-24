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


LLGL_EXPORT std::size_t ImageFormatSize(const ImageFormat imageFormat)
{
    switch (imageFormat)
    {
        case ImageFormat::R:                return 1;
        case ImageFormat::RG:               return 2;
        case ImageFormat::RGB:              return 3;
        case ImageFormat::BGR:              return 3;
        case ImageFormat::RGBA:             return 4;
        case ImageFormat::BGRA:             return 4;
        case ImageFormat::Depth:            return 1;
        case ImageFormat::DepthStencil:     return 2;
        case ImageFormat::CompressedRGB:    return 0;
        case ImageFormat::CompressedRGBA:   return 0;
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

LLGL_EXPORT bool IsCompressedFormat(const ImageFormat format)
{
    return (format >= ImageFormat::CompressedRGB);
}

LLGL_EXPORT bool IsDepthStencilFormat(const ImageFormat format)
{
    return (format == ImageFormat::Depth || format == ImageFormat::DepthStencil);
}


} // /namespace LLGL



// ================================================================================
