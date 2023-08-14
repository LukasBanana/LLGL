/*
 * TestTextureWriteAndRead.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <stdlib.h>


DEF_TEST( TextureWriteAndRead )
{
    const ColorRGBAub colorsRgbaUb4[4] =
    {
        ColorRGBAub{ 0xC0, 0x01, 0x12, 0xFF },
        ColorRGBAub{ 0x80, 0x12, 0x34, 0x90 },
        ColorRGBAub{ 0x13, 0x23, 0x56, 0x80 },
        ColorRGBAub{ 0x12, 0x34, 0x78, 0x70 },
    };

    static RandomColorSet colorsRgbaUb16;
    colorsRgbaUb16.Generate(16);

    auto CreateTextureAndTestImageData = [this](const char* name, const TextureDescriptor& texDesc, const TextureRegion& region, const void* data, std::size_t dataSize) -> TestResult
    {
        // Create texture object
        Texture* tex = nullptr;
        TestResult result = CreateTexture(texDesc, name, &tex);
        if (result != TestResult::Passed)
            return result;

        // Write texture data
        SrcImageDescriptor srcImage;
        {
            srcImage.format     = ImageFormat::RGBA;
            srcImage.dataType   = DataType::UInt8;
            srcImage.data       = data;
            srcImage.dataSize   = dataSize;
        }
        renderer->WriteTexture(*tex, region, srcImage);

        // Read texture data
        std::vector<char> outputData;
        outputData.resize(dataSize, char(0xFF));

        DstImageDescriptor dstImage;
        {
            dstImage.format     = srcImage.format;
            dstImage.dataType   = srcImage.dataType;
            dstImage.data       = outputData.data();
            dstImage.dataSize   = outputData.size();
        }
        renderer->ReadTexture(*tex, region, dstImage);

        // Release temporary texture
        renderer->Release(*tex);

        // Match input with output texture data
        if (::memcmp(data, outputData.data(), dataSize) != 0)
        {
            const std::string inputDataStr = FormatByteArray(data, dataSize);
            const std::string outputDataStr = FormatByteArray(outputData.data(), dataSize);
            Log::Errorf(
                "Mismatch between data of texture %s and initial data:\n -> Expected: [%s]\n -> Actual:   [%s] \n",
                name, inputDataStr.c_str(), outputDataStr.c_str()
            );
            return TestResult::FailedMismatch;
        }

        return TestResult::Passed;
    };

    #define TEST_IMAGE_DATA(NAME, DESC, REGION, DATA, SIZE)                                                 \
        {                                                                                                   \
            TestResult result = CreateTextureAndTestImageData((NAME), (DESC), (REGION), (DATA), (SIZE));    \
            if (result != TestResult::Passed)                                                               \
                return result;                                                                              \
        }

    ////////////// Texture2D //////////////

    TextureDescriptor tex2DDesc_1x1;
    {
        tex2DDesc_1x1.type          = TextureType::Texture2D;
        tex2DDesc_1x1.bindFlags     = BindFlags::CopySrc; //TODO: should not be required for ReadTexture function
        tex2DDesc_1x1.format        = Format::RGBA8UNorm;
        tex2DDesc_1x1.extent.width  = 1;
        tex2DDesc_1x1.extent.height = 1;
        tex2DDesc_1x1.mipLevels     = 1;
    }

    TEST_IMAGE_DATA(
        "tex2D{2D,1wh}:{single-texel-access}",
        tex2DDesc_1x1,
        TextureRegion( TextureSubresource{ 0, 0 }, Offset3D{ 0, 0, 0 }, Extent3D{ 1, 1, 1 } ),
        &(colorsRgbaUb4[0]),
        sizeof(ColorRGBAub)
    );

    TextureDescriptor tex2DDesc_4x4;
    {
        tex2DDesc_4x4.type          = TextureType::Texture2D;
        tex2DDesc_4x4.bindFlags     = BindFlags::CopySrc; //TODO: should not be required for ReadTexture function
        tex2DDesc_4x4.format        = Format::RGBA8UNorm;
        tex2DDesc_4x4.extent.width  = 4;
        tex2DDesc_4x4.extent.height = 4;
        tex2DDesc_4x4.mipLevels     = 0;
    }

    TEST_IMAGE_DATA(
        "tex2D{2D,4wh}:{single-texel-access}",
        tex2DDesc_4x4,
        TextureRegion( TextureSubresource{ 0, 1 }, Offset3D{ 1, 1, 0 }, Extent3D{ 1, 1, 1 } ),
        &(colorsRgbaUb4[0]),
        sizeof(ColorRGBAub)
    );

    TEST_IMAGE_DATA(
        "tex2D{2D,4wh}:{MIP0-full-access}",
        tex2DDesc_4x4,
        TextureRegion( TextureSubresource{ 0, 0 }, Offset3D{ 0, 0, 0 }, Extent3D{ 4, 4, 1 } ),
        colorsRgbaUb16.colors.data(),
        colorsRgbaUb16.colors.size() * sizeof(ColorRGBAub)
    );

    ////////////// Texture2DArray //////////////

    if (caps.features.hasArrayTextures)
    {
        TextureDescriptor tex2DArrayDesc_8x4x2;
        {
            tex2DArrayDesc_8x4x2.type           = TextureType::Texture2DArray;
            tex2DArrayDesc_8x4x2.bindFlags      = BindFlags::CopySrc; //TODO: should not be required for ReadTexture function
            tex2DArrayDesc_8x4x2.format         = Format::RGBA8UNorm;
            tex2DArrayDesc_8x4x2.extent.width   = 8;
            tex2DArrayDesc_8x4x2.extent.height  = 4;
            tex2DArrayDesc_8x4x2.arrayLayers    = 2;
            tex2DArrayDesc_8x4x2.mipLevels      = 2;
        }

        TEST_IMAGE_DATA(
            "tex2DArray{2D[2],8w,4h}:{MIP1-full-access}",
            tex2DArrayDesc_8x4x2,
            TextureRegion( TextureSubresource{ 0, 2, 1, 1 }, Offset3D{ 0, 0, 0 }, Extent3D{ 4, 2, 1 } ),
            colorsRgbaUb16.colors.data(),
            colorsRgbaUb16.colors.size() * sizeof(ColorRGBAub)
        );

        TEST_IMAGE_DATA(
            "tex2DArray{2D[2],8w,4h}:{1-layer-access}",
            tex2DArrayDesc_8x4x2,
            TextureRegion( TextureSubresource{ 1, 1 }, Offset3D{ 1, 0, 0 }, Extent3D{ 2, 2, 1 } ),
            colorsRgbaUb4,
            sizeof(colorsRgbaUb4)
        );

        TEST_IMAGE_DATA(
            "tex2DArray{2D[2],8w,4h}:{2-layer-access}",
            tex2DArrayDesc_8x4x2,
            TextureRegion( TextureSubresource{ 0, 2, 1, 1 }, Offset3D{ 1, 0, 0 }, Extent3D{ 2, 1, 1 } ),
            colorsRgbaUb4,
            sizeof(colorsRgbaUb4)
        );
    }

    ////////////// Texture3D //////////////

    if (caps.features.has3DTextures)
    {
        TextureDescriptor tex3DDesc_4x4x4;
        {
            tex3DDesc_4x4x4.type            = TextureType::Texture3D;
            tex3DDesc_4x4x4.bindFlags       = BindFlags::CopySrc; //TODO: should not be required for ReadTexture function
            tex3DDesc_4x4x4.format          = Format::RGBA8UNorm;
            tex3DDesc_4x4x4.extent.width    = 4;
            tex3DDesc_4x4x4.extent.height   = 4;
            tex3DDesc_4x4x4.extent.depth    = 4;
            tex3DDesc_4x4x4.mipLevels       = 2;
        }

        TEST_IMAGE_DATA(
            "tex3D{3D,8whd}:{MIP1-full-access}",
            tex3DDesc_4x4x4,
            TextureRegion( TextureSubresource{ 0, 1 }, Offset3D{ 0, 0, 0 }, Extent3D{ 2, 2, 2 } ),
            colorsRgbaUb16.colors.data(),
            8 * sizeof(ColorRGBAub)
        );

        TEST_IMAGE_DATA(
            "tex3D{3D,8whd}:{1-slice-access}",
            tex3DDesc_4x4x4,
            TextureRegion( TextureSubresource{ 0, 0 }, Offset3D{ 0, 0, 2 }, Extent3D{ 4, 4, 1 } ),
            colorsRgbaUb16.colors.data(),
            colorsRgbaUb16.colors.size() * sizeof(ColorRGBAub)
        );

        TEST_IMAGE_DATA(
            "tex3D{3D,8whd}:{2-slice-access}",
            tex3DDesc_4x4x4,
            TextureRegion( TextureSubresource{ 0, 0 }, Offset3D{ 1, 1, 1 }, Extent3D{ 2, 2, 2 } ),
            colorsRgbaUb16.colors.data(),
            8 * sizeof(ColorRGBAub)
        );
    }

    return TestResult::Passed;
}

