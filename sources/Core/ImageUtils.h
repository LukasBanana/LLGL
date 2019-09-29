/*
 * ImageUtils.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_IMAGE_UTILS_H
#define LLGL_IMAGE_UTILS_H


#include <cstdint>


namespace LLGL
{


struct Extent3D;

/* ----- Functions ----- */

// Copies the specified extent from the source image to the destination image buffer.
void BitBlit(
    const Extent3D& extent,
    std::uint32_t   bpp,
    char*           dst,
    std::uint32_t   dstRowStride,
    std::uint32_t   dstDepthStride,
    const char*     src,
    std::uint32_t   srcRowStride,
    std::uint32_t   srcDepthStride
);


} // /namespace LLGL


#endif



// ================================================================================
