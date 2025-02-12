/*
 * ImageUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "ImageUtils.h"
#include "Assertion.h"
#include <LLGL/Types.h>
#include <LLGL/Utils/ForRange.h>
#include <cstdint>
#include <algorithm>
#include <string.h>


namespace LLGL
{


LLGL_EXPORT void BitBlit(
    const Extent3D& extent,
    std::uint32_t   bpp,
    char*           dst,
    std::uint32_t   dstRowStride,
    std::uint32_t   dstLayerStride,
    const char*     src,
    std::uint32_t   srcRowStride,
    std::uint32_t   srcLayerStride)
{
    const std::uint32_t rowLength   = bpp * extent.width;
    const std::uint32_t layerLength = rowLength * extent.height;

    /* Clamp strides to tightly packed lengths */
    dstRowStride = std::max(dstRowStride, rowLength);
    srcRowStride = std::max(srcRowStride, rowLength);

    const std::uint32_t dstLayerLength = dstRowStride * extent.height;
    const std::uint32_t srcLayerLength = srcRowStride * extent.height;

    LLGL_ASSERT(
        dstLayerStride == 0 || dstLayerStride >= dstLayerLength,
        "'dstLayerStride' must be 0 or at least %u, but %u was specified", dstLayerLength, dstLayerStride
    );
    LLGL_ASSERT(
        srcLayerStride == 0 || srcLayerStride >= srcLayerLength,
        "'srcLayerStride' must be 0 or at least %u, but %u was specified", srcLayerLength, srcLayerStride
    );

    dstLayerStride = std::max(dstLayerLength, std::max(dstLayerStride, layerLength));
    srcLayerStride = std::max(srcLayerLength, std::max(srcLayerStride, layerLength));

    if (srcRowStride == dstRowStride && rowLength == dstRowStride)
    {
        if (srcLayerStride == dstLayerStride && layerLength == dstLayerStride)
        {
            /* Copy region directly into output data */
            ::memcpy(dst, src, layerLength * extent.depth);
        }
        else
        {
            /* Copy region directly into output data */
            for_range(z, extent.depth)
            {
                /* Copy current slice */
                ::memcpy(dst, src, layerLength);

                /* Move pointers to next slice */
                dst += dstLayerStride;
                src += srcLayerStride;
            }
        }
    }
    else
    {
        /* Adjust depth strides */
        dstLayerStride -= dstRowStride * extent.height;
        srcLayerStride -= srcRowStride * extent.height;

        /* Copy region directly into output data */
        for_range(z, extent.depth)
        {
            /* Copy current slice */
            for_range(y, extent.height)
            {
                /* Copy current row */
                ::memcpy(dst, src, rowLength);

                /* Move pointers to next row */
                dst += dstRowStride;
                src += srcRowStride;
            }

            /* Move pointers to next slice */
            dst += dstLayerStride;
            src += srcLayerStride;
        }
    }
}


} // /namespace LLGL



// ================================================================================
