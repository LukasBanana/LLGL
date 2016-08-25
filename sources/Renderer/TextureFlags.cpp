/*
 * TextureFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/TextureFlags.h>


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
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
