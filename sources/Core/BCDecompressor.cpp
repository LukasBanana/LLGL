/*
 * BCDecompressor.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "BCDecompressor.h"
#include <LLGL/Types.h>
#include <LLGL/Utils/ForRange.h>
#include <cstring>


namespace LLGL
{


static void DecompressRGBColor16Bit(std::uint8_t* dst, std::uint16_t src)
{
    dst[0] = ((src >> 12) & 0x000F) * 0xFF / 0x0F;
    dst[1] = ((src >>  6) & 0x001F) * 0xFF / 0x1F;
    dst[2] = ((src >>  1) & 0x000F) * 0xFF / 0x0F;
}

#if 0 //UNUSED
static void DecompressRGBAColor16Bit(std::uint8_t* dst, std::uint16_t src)
{
    DecompressRGBColor16Bit(dst, src);
    dst[3] = (src & 0x0001) * 0xFF;
}

static std::uint8_t DecompressAlphaChannel4Bit(std::uint32_t& src)
{
    auto alpha = static_cast<std::uint8_t >((src & 0x0000000F) * 0xFF / 0x0F);
    src >>= 4;
    return alpha;
}

static void Decompress16AlphaChannels4Bit(std::uint8_t* dst, std::uint32_t src0, std::uint32_t src1)
{
    for (int i = 0; i < 8; ++i)
    {
        dst[i    ] = DecompressAlphaChannel4Bit(src0);
        dst[i + 8] = DecompressAlphaChannel4Bit(src1);
    }
}
#endif

static std::uint8_t InterpolateColorComponent(std::uint16_t a, std::uint16_t b)
{
    if (a > b)
        return static_cast<std::uint8_t>((2 * a + b + 1) / 3);
    else
        return static_cast<std::uint8_t>((a + b) / 2);
}

static void InterpolateColor(std::uint8_t* dst, const std::uint8_t* src0, const std::uint8_t* src1)
{
    dst[0] = InterpolateColorComponent(src0[0], src1[0]);
    dst[1] = InterpolateColorComponent(src0[1], src1[1]);
    dst[2] = InterpolateColorComponent(src0[2], src1[2]);
}

DynamicByteArray DecompressBC1ToRGBA8UNorm(
    const Extent2D& extent,
    const char*     data,
    std::size_t     dataSize,
    unsigned        /*threadCount*/)
{
    /* Return null on invalid arguments */
    if (extent.width % 4 != 0 || extent.height % 4 != 0 || data == nullptr || dataSize < extent.width * extent.height / 2)
        return nullptr;

    DynamicByteArray dstImage{ extent.width * extent.height * 4, UninitializeTag{} };

    std::uint8_t* output = reinterpret_cast<std::uint8_t*>(dstImage.get());
    std::uint16_t compressedColor[2];
    std::uint8_t decompressedColor[4][3];

    const std::size_t formatByteSize = 4;

    for_range(y, extent.height / 4u)
    {
        for_range(x, extent.width / 4u)
        {
            /* Decompress two 16 bit colors */
            for_range(i, 2)
            {
                compressedColor[i] = *reinterpret_cast<const std::uint16_t*>(data);
                DecompressRGBColor16Bit(decompressedColor[i], compressedColor[i]);
                data += 2;
            }

            /* Generate two more colors by interpolating the previous two */
            InterpolateColor(decompressedColor[2], decompressedColor[0], decompressedColor[1]);
            InterpolateColor(decompressedColor[3], decompressedColor[1], decompressedColor[0]);

            /* Read palette bitfield */
            std::uint32_t palette = *reinterpret_cast<const std::uint32_t*>(data);
            data += 4;

            /* Generate 4x4 pixel block from palette bitmask */
            for_range(i, 16u)
            {
                /* Get palette index from last two bits */
                const std::uint32_t paletteIndex = palette & 0x00000003;
                palette >>= 2;

                /* Write out image color */
                const std::size_t outputOffset = (((y * 4 + i/4) * extent.width) + (x*4 + i%4)) * formatByteSize;
                output[outputOffset + 0] = decompressedColor[paletteIndex][0];
                output[outputOffset + 1] = decompressedColor[paletteIndex][1];
                output[outputOffset + 2] = decompressedColor[paletteIndex][2];
                output[outputOffset + 3] = 0xFF;
            }
        }
    }

    return dstImage;
}


} // /namespace LLGL



// ================================================================================
