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
