/*
 * ImageUtils.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "ImageUtils.h"
#include <LLGL/Types.h>
#include <cstdint>
#include <cstring>


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
            for (std::uint32_t z = 0; z < extent.depth; ++z)
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
        for (std::uint32_t z = 0; z < extent.depth; ++z)
        {
            /* Copy current slice */
            for (std::uint32_t y = 0; y < extent.height; ++y)
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
