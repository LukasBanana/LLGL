/*
 * Testset.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_TESTSET_H
#define LLGL_TESTSET_H


#include <LLGL/Container/ArrayView.h>
#include <LLGL/Utils/ColorRGBA.h>
#include <LLGL/Utils/ColorRGB.h>
#include <vector>


// Shifts the bits of the specified 32-bit integer to flip little-endian to big-endian and wise verse.
#define FLIP_ENDIAN(INT32)                  \
    (                                       \
        (((INT32) >> 24) & 0x000000FF) |    \
        (((INT32) >>  8) & 0x0000FF00) |    \
        (((INT32) <<  8) & 0x00FF0000) |    \
        (((INT32) << 24) & 0xFF000000)      \
    )


namespace Testset
{


LLGL::ArrayView<LLGL::ColorRGBAub> GetColorsRgbaUb4();
LLGL::ArrayView<LLGL::ColorRGBAub> GetColorsRgbaUb8();
LLGL::ArrayView<LLGL::ColorRGBf> GetColorsRgbF4();
LLGL::ArrayView<LLGL::ColorRGBf> GetColorsRgbF8();
LLGL::ArrayView<float> GetColorsRgF8();

std::vector<LLGL::ColorRGBAub> GenerateColorsRgbaUb(std::size_t count);
std::vector<float> GenerateFloats(std::size_t count);


} // /namespace Testset


#endif



// ================================================================================
