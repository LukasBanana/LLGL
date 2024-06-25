/*
 * TestMipMaps.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


// Returns true if 'x' has only a single bit set to 1, i.e. 'x' is a power-of-two value
static bool IsPowerOfTwo(std::uint32_t x)
{
    bool firstBitSet = false;
    for (std::uint32_t i = 1; i != 0; i <<= 1)
    {
        if ((x & i) != 0)
        {
            if (firstBitSet)
                return false;
            firstBitSet = true;
        }
    }
    return firstBitSet;
}

static bool IsPowerOfTwoExtent(const Extent3D& extent)
{
    return (IsPowerOfTwo(extent.width) && IsPowerOfTwo(extent.height) && IsPowerOfTwo(extent.depth));
}

/*
This test doesn't render anything but only evaluates the MIP-map levels of the textures already loaded by the testbed.
Non-power-of-two (NPOT) textures are accepted to use different minification filters (such as box-filter, which can incur undersampling),
which requires a larger threshold when comparing with the reference images.
*/
DEF_TEST( MipMaps )
{
    TestResult result = TestResult::Passed;

    auto ReadMipMaps = [this](Texture* tex, const std::string& name) -> TestResult
    {
        TestResult result = TestResult::Passed;

        const TextureDescriptor texDesc = tex->GetDesc();

        /*
        Set a much higher threshold for non-power-of-two textures because some backends (GL and D3D12) use only a box-filter to compute MIP image reduction.
        NOTE:
          For future improvements, these backends could provide MIP-map generation via image-blit functionality to compute a perfect reduction filter (i.e. no undersampling).
          This could be enabled via a new MiscFlags entry, for example: MiscFlags::HighQualityMipFilter.
        */
        const bool isNpotTexture = !IsPowerOfTwoExtent(texDesc.extent);
        const int diffThreshold = (isNpotTexture ? 170 : 10);

        for_subrange(mip, 1, texDesc.mipLevels)
        {
            if (opt.fastTest && mip % 2 == 1)
                continue;

            // Read current MIP-map from input texture
            const Extent3D mipExtent = tex->GetMipExtent(mip);

            std::vector<ColorRGBub> mipData;
            mipData.resize(mipExtent.width * mipExtent.height);

            const TextureRegion texRegion{ TextureSubresource{ 0, mip }, Offset3D{}, mipExtent };

            MutableImageView dstImageView;
            {
                dstImageView.format     = ImageFormat::RGB;
                dstImageView.dataType   = DataType::UInt8;
                dstImageView.data       = mipData.data();
                dstImageView.dataSize   = sizeof(decltype(mipData)::value_type) * mipData.size();
            }
            renderer->ReadTexture(*tex, texRegion, dstImageView);

            // Save result and diff against reference
            const std::string mipName = name + "_Mip" + std::to_string(mip);
            SaveColorImage(mipData, Extent2D{ mipExtent.width, mipExtent.height }, mipName);

            const DiffResult diff = DiffImages(mipName, diffThreshold);

            TestResult intermediateResult = diff.Evaluate(mipName.c_str());
            if (intermediateResult != TestResult::Passed)
            {
                result = intermediateResult;
                if (!opt.greedy)
                    break;
            }
        }

        return result;
    };

    #define READ_MIPMAPS(TEX, NAME)                                 \
        {                                                           \
            TestResult intermResult = ReadMipMaps((TEX), (NAME));   \
            if (intermResult != TestResult::Passed)                 \
            {                                                       \
                if (opt.greedy)                                     \
                    result = intermResult;                          \
                else                                                \
                    return intermResult;                            \
            }                                                       \
        }

    READ_MIPMAPS(textures[TextureGrid10x10], "Grid10x10");
    READ_MIPMAPS(textures[TextureGradient], "Gradient");
    READ_MIPMAPS(textures[TexturePaintingA_NPOT], "PaintingA");

    return result;
}

