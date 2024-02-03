/*
 * TestTextureCopy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include "Testset.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


DEF_TEST( TextureCopy )
{
    // Define random color values for initial texture data
    const ArrayView<ColorRGBAub> colorsRgbaUb8 = Testset::GetColorsRgbaUb8();

    auto CreateTargetTexturesAndCopyImage = [this, &colorsRgbaUb8](const char* name, TextureType type, const Extent3D& extent, std::uint32_t mips, std::uint32_t layers) -> TestResult
    {
        const std::uint64_t t0 = Timer::Tick();

        auto ToNonArrayTextureType = [](TextureType type) -> TextureType
        {
            switch (type)
            {
                case TextureType::Texture1DArray:
                    return TextureType::Texture1D;
                case TextureType::Texture2DArray:
                case TextureType::TextureCube:
                case TextureType::TextureCubeArray:
                    return TextureType::Texture2D;
                default:
                    return type;
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
        const std::string srcTexName = std::string("src.") + name;
        TextureDescriptor srcTexDesc;
        {
            srcTexDesc.type         = type;
            srcTexDesc.bindFlags    = BindFlags::CopySrc;
            srcTexDesc.format       = Format::RGBA8UNorm;
            srcTexDesc.extent       = extent;
            srcTexDesc.mipLevels    = mips;
            srcTexDesc.arrayLayers  = layers;
        }
        CREATE_TEXTURE(srcTex, srcTexDesc, srcTexName.c_str(), nullptr);

        // Create intermediate texture to copy into
        const std::string interTexName = std::string("inter.") + name;
        TextureDescriptor interTexDesc;
        {
            interTexDesc.type           = ToNonArrayTextureType(type);
            interTexDesc.bindFlags      = BindFlags::CopySrc | BindFlags::CopyDst;
            interTexDesc.format         = Format::RGBA8UNorm;
            interTexDesc.extent         = extent;
            interTexDesc.mipLevels      = 1;
            interTexDesc.arrayLayers    = 1;
        }
        CREATE_TEXTURE(interTex, interTexDesc, interTexName.c_str(), nullptr);

        // Create destination texture to read the results from
        const std::string dstTexName = std::string("dst.") + name;
        TextureDescriptor dstTexDesc;
        {
            dstTexDesc.type         = type;
            dstTexDesc.bindFlags    = BindFlags::CopyDst;
            dstTexDesc.format       = Format::RGBA8UNorm;
            dstTexDesc.extent       = extent;
            dstTexDesc.mipLevels    = mips;
            dstTexDesc.arrayLayers  = layers;
        }
        CREATE_TEXTURE(dstTex, dstTexDesc, dstTexName.c_str(), nullptr);

        // Run test through all MIP-maps and array layers (should not be more than 2 each)
        const std::uint32_t texDims = NumTextureDimensions(interTexDesc.type);

        for_range(mip, mips)
        {
            for_range(layer, layers)
            {
                // Write image into source texture
                ImageView srcImage;
                {
                    srcImage.format     = ImageFormat::RGBA;
                    srcImage.dataType   = DataType::UInt8;
                    srcImage.data       = colorsRgbaUb8.data();
                    srcImage.dataSize   = sizeof(colorsRgbaUb8[0]) * colorsRgbaUb8.size();
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
                MutableImageView dstImage;
                {
                    dstImage.format     = ImageFormat::RGBA;
                    dstImage.dataType   = DataType::UInt8;
                    dstImage.data       = outputData;
                    dstImage.dataSize   = sizeof(outputData);
                }
                renderer->ReadTexture(*dstTex, texRegion, dstImage);

                // Evaluate results
                if (::memcmp(colorsRgbaUb8.data(), outputData, sizeof(outputData)) != 0)
                {
                    const std::string inputDataStr = TestbedContext::FormatByteArray(colorsRgbaUb8.data(), sizeof(colorsRgbaUb8[0]) * colorsRgbaUb8.size(), 4);
                    const std::string outputDataStr = TestbedContext::FormatByteArray(outputData, sizeof(outputData), 4);
                    Log::Errorf(
                        "Mismatch between data of texture %s [MIP %u, Layer %u] and copy result:\n"
                        " -> Expected: [%s]\n"
                        " -> Actual:   [%s]\n",
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

        // Print duration
        if (opt.showTiming)
        {
            const std::uint64_t t1 = Timer::Tick();
            Log::Printf("Copy texture: %s ( %f ms )\n", name, TestbedContext::ToMillisecs(t0, t1));
        }

        return TestResult::Passed;
    };

    #define TEST_TEXTURE_COPY(NAME, TYPE, EXTENT, MIPS, LAYERS)                                                 \
        {                                                                                                       \
            TestResult result = CreateTargetTexturesAndCopyImage((NAME), (TYPE), (EXTENT), (MIPS), (LAYERS));   \
            if (result != TestResult::Passed)                                                                   \
                return result;                                                                                  \
        }

    TEST_TEXTURE_COPY("tex{1D,64w}",  TextureType::Texture1D, Extent3D(64,  1,  1), 2, 1);
    TEST_TEXTURE_COPY("tex{2D,32wh}", TextureType::Texture2D, Extent3D(32, 32,  1), 2, 1);

    if (caps.features.has3DTextures)
        TEST_TEXTURE_COPY("tex{3D,16whd}", TextureType::Texture3D, Extent3D(16, 16, 16), 2, 1);

    if (caps.features.hasCubeTextures)
        TEST_TEXTURE_COPY("tex{Cube,16wh}", TextureType::TextureCube, Extent3D(16, 16, 1), 2, 6);

    if (caps.features.hasArrayTextures)
    {
        TEST_TEXTURE_COPY("tex{1D[2],64w}",  TextureType::Texture1DArray, Extent3D(64,  1, 1), 2, 2);
        TEST_TEXTURE_COPY("tex{2D[2],32wh}", TextureType::Texture2DArray, Extent3D(32, 32, 1), 2, 2);
    }

    if (caps.features.hasCubeArrayTextures)
        TEST_TEXTURE_COPY("tex{Cube[2],16w}", TextureType::TextureCubeArray, Extent3D(16, 16, 1), 2, 6*2);

    return TestResult::Passed;
}

