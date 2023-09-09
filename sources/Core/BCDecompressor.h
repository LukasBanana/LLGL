/*
 * BCDecompressor.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_BC_DECOMPRESSOR_H
#define LLGL_BC_DECOMPRESSOR_H


#include <LLGL/Types.h>
#include <LLGL/Container/DynamicArray.h>
#include <cstddef>


namespace LLGL
{


struct Extent2D;

/* ----- Functions ----- */

/*
Returns an image buffer in the Format::RGBA8UNorm format for the specified BC1 encoded data, or null on failure
Width and height of the input image must be a multiple of 4.
*/
DynamicByteArray DecompressBC1ToRGBA8UNorm(
    const Extent2D& extent,
    const char*     data,
    std::size_t     dataSize,
    unsigned        threadCount = 0
);


} // /namespace LLGL


#endif



// ================================================================================
