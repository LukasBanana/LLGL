/*
 * TestTextureCopy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


DEF_TEST( TextureCopy )
{
    // Create small buffer with initial data and read access
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

    auto CreateTargetTexturesAndCopyImage = [this](const char* name, TextureType type, const Extent3D& extent, std::uint32_t mips, std::uint32_t layers) -> TestResult
    {
        auto ToNonArrayTextureType = [](TextureType type) -> TextureType
        {
            switch (type)
            {
                case TextureType::Texture1DArray:   return TextureType::Texture1D;
                case TextureType::Texture2DArray:   return TextureType::Texture2D;
                case TextureType::TextureCube:      return TextureType::Texture2D;
                case TextureType::TextureCubeArray: return TextureType::TextureCube;
                default:                            return type;
            }
        };

        auto MakeOffset3D = [](std::uint32_t dims) -> Offset3D
        {
            switch (dims)
            {
                case 1:     return Offset3D{ 4, 0, 0 };
                case 2:     return Offset3D{ 4, 3, 0 };
                case 3:     return Offset3D{ 4, 3, 2 };
                default:    return {};
            }
        };

        auto MakeExtent3D = [](std::uint32_t dims) -> Extent3D
        {
            switch (dims)
            {
                case 1:     return Extent3D{ 8, 1, 1 };
                case 2:     return Extent3D{ 4, 2, 1 };
                case 3:     return Extent3D{ 2, 2, 2 };
                default:    return {};
            }
        };

        // Create source texture
        TextureDescriptor srcTexDesc;
        {
            srcTexDesc.type         = type;
            srcTexDesc.bindFlags    = BindFlags::CopySrc;
            srcTexDesc.extent       = extent;
            srcTexDesc.mipLevels    = mips;
            srcTexDesc.arrayLayers  = layers;
        }
        CREATE_TEXTURE(srcTex, srcTexDesc, "srcTex", nullptr);

        // Create intermediate texture to copy into
        TextureDescriptor interTexDesc;
        {
            interTexDesc.type           = ToNonArrayTextureType(type);
            interTexDesc.bindFlags      = BindFlags::CopySrc | BindFlags::CopyDst;
            interTexDesc.extent         = extent;
            interTexDesc.mipLevels      = 1;
            interTexDesc.arrayLayers    = 1;
        }
        CREATE_TEXTURE(interTex, interTexDesc, "interTex", nullptr);

        // Create destination texture to read the results from
        TextureDescriptor dstTexDesc;
        {
            dstTexDesc.type         = type;
            dstTexDesc.bindFlags    = BindFlags::CopyDst;
            dstTexDesc.extent       = extent;
            dstTexDesc.mipLevels    = mips;
            dstTexDesc.arrayLayers  = layers;
        }
        CREATE_TEXTURE(dstTex, dstTexDesc, "dstTex", nullptr);

        // Run test through all MIP-maps and array layers (should not be more than 2 each)
        const auto texDims = NumTextureDimensions(interTexDesc.type);

        for_range(mip, mips)
        {
            for_range(layer, layers)
            {
                // Write image into source texture
                SrcImageDescriptor srcImage;
                {
                    srcImage.format     = ImageFormat::RGBA;
                    srcImage.dataType   = DataType::UInt8;
                    srcImage.data       = colorsRgbaUb8;
                    srcImage.dataSize   = sizeof(colorsRgbaUb8);
                }
                TextureRegion texRegion;
                {
                    texRegion.subresource   = TextureSubresource{ layer, mip };
                    texRegion.offset        = MakeOffset3D(texDims);
                    texRegion.extent        = MakeExtent3D(texDims);
                };
                renderer->WriteTexture(*srcTex, texRegion, srcImage);

                // Copy source into intermediate texture
                cmdBuffer->Begin();
                {
                    cmdBuffer->CopyTexture(*interTex, TextureLocation{ texRegion.offset }, *srcTex, TextureLocation{ texRegion.offset, layer, mip }, texRegion.extent);
                    cmdBuffer->CopyTexture(*dstTex, TextureLocation{ texRegion.offset, layer, mip }, *interTex, TextureLocation{ texRegion.offset }, texRegion.extent);
                }
                cmdBuffer->End();

                // Read results from destination texture
                ColorRGBAub outputData[8];
                DstImageDescriptor dstImage;
                {
                    dstImage.format     = ImageFormat::RGBA;
                    dstImage.dataType   = DataType::UInt8;
                    dstImage.data       = outputData;
                    dstImage.dataSize   = sizeof(outputData);
                }
                renderer->ReadTexture(*dstTex, texRegion, dstImage);

                // Evaluate results
                if (::memcmp(colorsRgbaUb8, outputData, sizeof(outputData)) != 0)
                {
                    const std::string inputDataStr = FormatByteArray(colorsRgbaUb8, sizeof(colorsRgbaUb8));
                    const std::string outputDataStr = FormatByteArray(outputData, sizeof(outputData));
                    Log::Errorf(
                        "Mismatch between data of texture %s [MIP %u, Layer %u] and copy result:\n -> Expected: [%s]\n -> Actual:   [%s] \n",
                        name, mip, layer, inputDataStr.c_str(), outputDataStr.c_str()
                    );
                    return TestResult::FailedMismatch;
                }
            }
        }

        // Delete old resources
        renderer->Release(*srcTex);
        renderer->Release(*interTex);
        renderer->Release(*dstTex);

        return TestResult::Passed;
    };

    #define TEST_TEXTURE_COPY(NAME, TYPE, EXTENT, MIPS, LAYERS)                                                 \
        {                                                                                                       \
            TestResult result = CreateTargetTexturesAndCopyImage((NAME), (TYPE), (EXTENT), (MIPS), (LAYERS));   \
            if (result != TestResult::Passed)                                                                   \
                return result;                                                                                  \
        }

    TEST_TEXTURE_COPY("tex{1D,64w}",    TextureType::Texture1D,  Extent3D(64,  1,  1), 2, 1);
    TEST_TEXTURE_COPY("tex{2D,32wh}",   TextureType::Texture2D,  Extent3D(32, 32,  1), 2, 1);

    if (caps.features.has3DTextures)
        TEST_TEXTURE_COPY("tex{3D,16whd}",  TextureType::Texture3D,  Extent3D(16, 16, 16), 2, 1);

    if (caps.features.hasCubeTextures)
        TEST_TEXTURE_COPY("tex{Cube,16wh}",  TextureType::TextureCube,  Extent3D(16, 16, 1), 2, 6);

    if (caps.features.hasArrayTextures)
    {
        TEST_TEXTURE_COPY("tex{1D[2],64w}",     TextureType::Texture1DArray, Extent3D(64,  1, 1), 2, 2);
        TEST_TEXTURE_COPY("tex{2D[2],32wh}",    TextureType::Texture2DArray, Extent3D(32, 32, 1), 2, 2);
    }

    return TestResult::Passed;
}

