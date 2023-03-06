/*
 * BCDecompressor.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_BC_DECOMPRESSOR_H
#define LLGL_BC_DECOMPRESSOR_H


#include <LLGL/ImageFlags.h>
#include <cstdint>


namespace LLGL
{


struct Extent2D;

/* ----- Functions ----- */

/*
Returns an image buffer in the Format::RGBA8UNorm format for the specified BC1 encoded data, or null on failure
Width and height of the input image must be a multiple of 4.
*/
ByteBuffer DecompressBC1ToRGBA8UNorm(
    const Extent2D& extent,
    const char*     data,
    std::size_t     dataSize,
    unsigned        threadCount = 0
);


} // /namespace LLGL


#endif



// ================================================================================
