/*
 * TestImageStrides.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include "Testset.h"
#include <LLGL/ImageFlags.h>
#include <LLGL/Utils/Image.h>
#include <LLGL/Utils/TypeNames.h>
#include <thread>


// This test ensures that the ImageView::rowStride field works in the image conversion functions.
DEF_RITEST( ImageStrides )
{
    // Manual testing
    const ArrayView<ColorRGBAub> testsetColors = Testset::GetColorsRgbaUb8();

    // Initialize source image with row padding
    const Extent3D imageExtent{ 4, 4, 4 };
    const std::uint32_t numPixelsPerRow     = 10;
    const std::uint32_t numPixelsPerLayer   = numPixelsPerRow*6;

    std::vector<LLGL::ColorRGBAub> srcData, expectedData;
    srcData.resize(numPixelsPerLayer * imageExtent.depth);
    expectedData.resize(imageExtent.width * imageExtent.height * imageExtent.depth);
    {
        for_range(z, imageExtent.depth)
        {
            for_range(y, imageExtent.height)
            {
                for_range(x, imageExtent.width)
                {
                    const std::uint32_t i = (z * imageExtent.height + y) * imageExtent.width + x;
                    const std::uint32_t j = z * numPixelsPerLayer + y * numPixelsPerRow + x;
                    srcData[j] = testsetColors[i % testsetColors.size()];
                    expectedData[i] = srcData[j].ToRGB().ToRGBA(); // Clear alpha channel;
                }
            }
        }
    }

    std::vector<LLGL::ColorRGBf> dstData;
    dstData.resize(imageExtent.width * imageExtent.height * imageExtent.depth);

    // Convert image with padding
    const ImageView srcImg
    {
        /*format:*/         ImageFormat::RGBA,
        /*dataType:*/       DataType::UInt8,
        /*data:*/           srcData.data(),
        /*dataSize:*/       srcData.size()*sizeof(srcData[0]),
        /*rowStride:*/      numPixelsPerRow*sizeof(srcData[0]),
        /*layerStride:*/    numPixelsPerLayer*sizeof(srcData[0])
    };
    const MutableImageView dstImg
    {
        /*format:*/     ImageFormat::RGB,
        /*dataType:*/   DataType::Float32,
        /*data:*/       dstData.data(),
        /*dataSize:*/   dstData.size()*sizeof(dstData[0])
    };

    ConvertImageBuffer(srcImg, dstImg, imageExtent);

    // Convert image back to tightly packed image
    DynamicByteArray dstImgRGBA8ub = ConvertImageBuffer(ImageView{ dstImg }, ImageFormat::RGBA, DataType::UInt8);

    const std::size_t expectedSize = expectedData.size()*sizeof(expectedData[0]);
    if (dstImgRGBA8ub.size() != expectedSize)
    {
        Log::Errorf(
            Log::ColorFlags::StdError,
            "Mismatch between converted image size (%zu) and padded input image (%zu)\n",
            dstImgRGBA8ub.size(), expectedSize
        );
        return TestResult::FailedMismatch;
    }

    if (::memcmp(dstImgRGBA8ub.data(), expectedData.data(), expectedSize) != 0)
    {
        const std::string expectedDataStr = TestbedContext::FormatByteArray(expectedData.data(), expectedSize, 4);
        const std::string actualDataStr = TestbedContext::FormatByteArray(dstImgRGBA8ub.data(), dstImgRGBA8ub.size(), 4);
        Log::Errorf(
            Log::ColorFlags::StdError,
            "Mismatch between converted image data and padded input image:\n"
            " -> Expected: [%s]\n"
            " -> Actual:   [%s]\n",
            expectedDataStr.c_str(), actualDataStr.c_str()
        );
        return TestResult::FailedMismatch;
    }
    else if (opt.sanityCheck)
    {
        const std::string actualDataStr = TestbedContext::FormatByteArray(dstImgRGBA8ub.data(), dstImgRGBA8ub.size(), 4);
        Log::Printf(
            Log::ColorFlags::StdAnnotation,
            "Sanity check for converted image data from padded input image:\n"
            " -> [%s]\n",
            actualDataStr.c_str()
        );
    }

    // Compare image with expected values
    for_range(z, imageExtent.depth)
    {
        for_range(y, imageExtent.height)
        {
            for_range(x, imageExtent.width)
            {
                const std::uint32_t i = (z * imageExtent.height + y) * imageExtent.width + x;
                const ColorRGBub srcCol = testsetColors[i % testsetColors.size()].ToRGB();
                const ColorRGBub dstCol = dstData[i].Cast<std::uint8_t>();
                if (srcCol != dstCol)
                {
                    const std::string srcColStr = FormatByteArray(srcCol.Ptr(), sizeof(srcCol));
                    const std::string dstColStr = FormatByteArray(dstCol.Ptr(), sizeof(srcCol));
                    Log::Errorf(
                        Log::ColorFlags::StdError,
                        "Mismatch between converted image and padded input image at (%u,%u):\n"
                        " -> Expected: [%s]\n"
                        " -> Actual:   [%s]\n",
                        x, y, srcColStr.c_str(), dstColStr.c_str()
                    );
                    return TestResult::FailedMismatch;
                }
            }
        }
    }

    return TestResult::Passed;
}


