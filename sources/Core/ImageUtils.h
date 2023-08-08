/*
 * ImageUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_IMAGE_UTILS_H
#define LLGL_IMAGE_UTILS_H


#include <LLGL/Export.h>
#include <cstdint>


namespace LLGL
{


struct Extent3D;

/* ----- Functions ----- */

// Copies the specified extent from the source image to the destination image buffer.
LLGL_EXPORT void BitBlit(
    const Extent3D& extent,
    std::uint32_t   bpp,
    char*           dst,
    std::uint32_t   dstRowStride,
    std::uint32_t   dstLayerStride,
    const char*     src,
    std::uint32_t   srcRowStride,
    std::uint32_t   srcLayerStride
);


} // /namespace LLGL


#endif



// ================================================================================
