/*
 * Testset.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testset.h"
#include <LLGL/Utils/ForRange.h>


using namespace LLGL;

namespace Testset
{


LLGL::ArrayView<LLGL::ColorRGBAub> GetColorsRgbaUb4()
{
    static const ColorRGBAub colorsRgbaUb4[4] =
    {
        ColorRGBAub{ 0xC0, 0x01, 0x12, 0xFF },
        ColorRGBAub{ 0x80, 0x12, 0x34, 0x90 },
        ColorRGBAub{ 0x13, 0x23, 0x56, 0x80 },
        ColorRGBAub{ 0x12, 0x34, 0x78, 0x70 },
    };
    return colorsRgbaUb4;
}

LLGL::ArrayView<LLGL::ColorRGBAub> GetColorsRgbaUb8()
{
    static const ColorRGBAub colorsRgbaUb8[8] =
    {
        ColorRGBAub{ 0xC0, 0x01, 0x12, 0xFF },
        ColorRGBAub{ 0x80, 0x12, 0x34, 0x90 },
        ColorRGBAub{ 0x13, 0x23, 0x56, 0x80 },
        ColorRGBAub{ 0x12, 0x34, 0x78, 0x70 },
        ColorRGBAub{ 0xF0, 0xB0, 0xAA, 0xBB },
        ColorRGBAub{ 0x50, 0x20, 0xAC, 0x0F },
        ColorRGBAub{ 0xAB, 0xCD, 0xEF, 0x01 },
        ColorRGBAub{ 0x66, 0x78, 0x23, 0x4C },
    };
    return colorsRgbaUb8;
}

LLGL::ArrayView<LLGL::ColorRGBf> GetColorsRgbF4()
{
    static const ColorRGBf colorsRgbF4[4] =
    {
        ColorRGBf{ 1.0f, 0.0f, 0.0f },
        ColorRGBf{ 0.0f, 1.0f, 0.0f },
        ColorRGBf{ 0.0f, 0.0f, 1.0f },
        ColorRGBf{ 0.1f, 0.2f, 0.3f },
    };
    return colorsRgbF4;
}

LLGL::ArrayView<LLGL::ColorRGBf> GetColorsRgbF8()
{
    static const ColorRGBf colorsRgbF8[8] =
    {
        ColorRGBf{ 1.00f, 0.00f, 0.00f },
        ColorRGBf{ 0.00f, 1.00f, 0.00f },
        ColorRGBf{ 0.00f, 0.00f, 1.00f },
        ColorRGBf{ 0.10f, 0.20f, 0.30f },
        ColorRGBf{ 0.35f, 0.42f, 0.83f },
        ColorRGBf{ 0.25f, 0.50f, 0.85f },
        ColorRGBf{ 0.30f, 0.40f, 1.70f },
        ColorRGBf{ 2.00f, 3.21f, 2.46f },
    };
    return colorsRgbF8;
}

LLGL::ArrayView<float> GetColorsRgF8()
{
    static const float colorsRgF8[8*2] =
    {
        1.00f, 0.00f,
        0.00f, 1.00f,
        0.45f, 0.67f,
        0.10f, 0.20f,
        0.35f, 0.42f,
        0.25f, 0.50f,
        0.30f, 0.40f,
        2.00f, 3.21f,
    };
    return colorsRgF8;
}

static std::uint8_t RandomUint8()
{
    return static_cast<std::uint8_t>(::rand() % 256);
}

std::vector<LLGL::ColorRGBAub> GenerateColorsRgbaUb(std::size_t count)
{
    std::vector<LLGL::ColorRGBAub> colors;
    colors.resize(count);
    for_range(i, count)
    {
        colors[i].r = RandomUint8();
        colors[i].g = RandomUint8();
        colors[i].b = RandomUint8();
        colors[i].a = RandomUint8();
    }
    return colors;
}

static float RandomFloat32()
{
    return (static_cast<float>(::rand()) / static_cast<float>(RAND_MAX));
}

std::vector<float> GenerateFloats(std::size_t count)
{
    std::vector<float> colors;
    count *= 2;
    colors.resize(count);
    for_range(i, count)
        colors[i] = RandomFloat32();
    return colors;
}


} // /namespace Testset



// ================================================================================
