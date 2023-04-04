/*
 * ImageUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "ImageUtils.h"
#include <LLGL/Types.h>
#include <LLGL/Misc/ForRange.h>
#include <cstdint>
#include <string.h>


namespace LLGL
{


void BitBlit(
    const Extent3D& extent,
    std::uint32_t   bpp,
    char*           dst,
    std::uint32_t   dstRowStride,
    std::uint32_t   dstDepthStride,
    const char*     src,
    std::uint32_t   srcRowStride,
    std::uint32_t   srcDepthStride)
{
    const auto rowStride    = bpp * extent.width;
    const auto depthStride  = rowStride * extent.height;

    if (srcRowStride == dstRowStride && rowStride == dstRowStride)
    {
        if (srcDepthStride == dstDepthStride && depthStride == dstDepthStride)
        {
            /* Copy region directly into output data */
            ::memcpy(dst, src, depthStride * extent.depth);
        }
        else
        {
            /* Copy region directly into output data */
            for_range(z, extent.depth)
            {
                /* Copy current slice */
                ::memcpy(dst, src, depthStride);

                /* Move pointers to next slice */
                dst += dstDepthStride;
                src += srcDepthStride;
            }
        }
    }
    else
    {
        /* Adjust depth stride */
        srcDepthStride -= srcRowStride * extent.height;

        /* Copy region directly into output data */
        for_range(z, extent.depth)
        {
            /* Copy current slice */
            for_range(y, extent.height)
            {
                /* Copy current row */
                ::memcpy(dst, src, rowStride);

                /* Move pointers to next row */
                dst += dstRowStride;
                src += srcRowStride;
            }

            /* Move pointers to next slice */
            src += srcDepthStride;
        }
    }
}


} // /namespace LLGL



// ================================================================================
