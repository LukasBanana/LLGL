/*
 * TestTextureToBufferCopy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include "Testset.h"


/*
Tests the CopyTextureFromBuffer() and CopyBufferFromTexture() functions starting from a texture with various texture formats.
There is no rendering. The values are only validated via ReadTexture().
*/
DEF_TEST( TextureToBufferCopy )
{
    auto CopyToBufferAndReadback = [this](
        const char* name, TextureType type, Format format, const Extent3D& extent, std::uint32_t mips, std::uint32_t layers, const ImageView& srcImage) -> TestResult
    {
        const std::uint64_t t0 = Timer::Tick();

        const bool formatAsFloats = IsFloatFormat(format);

        auto NonArrayMipExtent = [](TextureType type, const Extent3D& extent, std::uint32_t mip) -> Extent3D
        {
            switch (type)
            {
                case TextureType::Texture1DArray:
                    return GetMipExtent(TextureType::Texture1D, extent, mip);
                case TextureType::Texture2DArray:
                case TextureType::TextureCube:
                case TextureType::TextureCubeArray:
                case TextureType::Texture2DMSArray:
                    return GetMipExtent(TextureType::Texture2D, extent, mip);
                default:
                    return GetMipExtent(type, extent, mip);
            }
        };

        auto FlattenExtent2D = [](const Extent3D& extent) -> Extent3D
        {
            return Extent3D{ extent.width, extent.height * extent.depth, 1u };
        };

        // Get source texture descriptor
        const FormatAttributes  formatAttribs       = GetFormatAttribs(format);
        const std::uint32_t     numTexelsPerLayer   = extent.width * extent.height * extent.depth;
        const std::uint32_t     numTexelsMip0       = numTexelsPerLayer * layers;
        const std::size_t       bufSize             = static_cast<std::size_t>(GetMemoryFootprint(format, numTexelsPerLayer)); // GPU buffer size
        const std::size_t       imgSizeMip0         = static_cast<std::size_t>(GetMemoryFootprint(srcImage.format, srcImage.dataType, numTexelsMip0)); // CPU image buffer size

        if (srcImage.dataSize < imgSizeMip0)
        {
            Log::Errorf(
                "Initial data size (%" PRIu64 ") is too small for texture %s (%" PRIu64 ")\n",
                srcImage.dataSize, imgSizeMip0
            );
            return TestResult::FailedErrors;
        }

        // Create source texture with initial image data to copy from
        TextureDescriptor srcTexDesc;
        {
            srcTexDesc.type         = type;
            srcTexDesc.bindFlags    = BindFlags::CopySrc | BindFlags::Sampled | BindFlags::ColorAttachment;
            srcTexDesc.format       = format;
            srcTexDesc.extent       = extent;
            srcTexDesc.mipLevels    = mips;
            srcTexDesc.arrayLayers  = layers;
        }
        CREATE_TEXTURE(srcTex, srcTexDesc, name, &srcImage);

        // First check that image data was written correctly to source texture
        DynamicByteArray srcImageFeedbackData = DynamicByteArray{ imgSizeMip0 };
        MutableImageView srcImageFeedback;
        {
            srcImageFeedback.format     = srcImage.format;
            srcImageFeedback.dataType   = srcImage.dataType;
            srcImageFeedback.data       = srcImageFeedbackData.data();
            srcImageFeedback.dataSize   = srcImageFeedbackData.size();
        }
        TextureRegion srcTexFullRegion;
        {
            srcTexFullRegion.subresource    = TextureSubresource{ 0, layers, 0, 1 };
            srcTexFullRegion.extent         = extent;
        }
        renderer->ReadTexture(*srcTex, srcTexFullRegion, srcImageFeedback);

        if (::memcmp(srcImage.data, srcImageFeedback.data, imgSizeMip0) != 0)
        {
            const std::string initialDataStr = TestbedContext::FormatByteArray(srcImage.data, imgSizeMip0, 4, formatAsFloats);
            const std::string readbackDataStr = TestbedContext::FormatByteArray(srcImageFeedback.data, imgSizeMip0, 4, formatAsFloats);
            Log::Errorf(
                "Mismatch between initial data of texture %s and readback result:\n"
                " -> Expected: [%s]\n"
                " -> Actual:   [%s]\n",
                name, initialDataStr.c_str(), readbackDataStr.c_str()
            );
            return TestResult::FailedErrors;
        }

        // Create buffer to copy from source texture and to destination texture
        const std::string bufName = std::string("interm.") + name;
        BufferDescriptor bufDesc;
        {
            bufDesc.size        = bufSize;
            bufDesc.bindFlags   = BindFlags::CopySrc | BindFlags::CopyDst;
        }
        CREATE_BUFFER(buf, bufDesc, bufName.c_str(), nullptr);

        // Create destination texture
        const std::string dstTexName = std::string("dst.") + name;
        TextureDescriptor dstTexDesc;
        {
            dstTexDesc.type         = TextureType::Texture2D;
            dstTexDesc.bindFlags    = BindFlags::CopyDst;
            dstTexDesc.miscFlags    = MiscFlags::NoInitialData;
            dstTexDesc.format       = srcTexDesc.format;
            dstTexDesc.extent       = FlattenExtent2D(extent);
            dstTexDesc.arrayLayers  = 1;
            dstTexDesc.mipLevels    = 1;
        }
        CREATE_TEXTURE(dstTex, dstTexDesc, dstTexName.c_str(), nullptr);

        // Run test through all MIP-maps and array layers (should not be more than 2 each)
        std::vector<char> srcTexImage;
        std::vector<char> dstTexImage;

        for_range(mip, srcTexDesc.mipLevels)
        {
            for_range(layer, srcTexDesc.arrayLayers)
            {
                // Determine texture region to copy buffer from
                TextureRegion srcRegion;
                {
                    srcRegion.subresource   = TextureSubresource{ layer, mip };
                    srcRegion.offset        = Offset3D{};
                    srcRegion.extent        = NonArrayMipExtent(type, extent, mip);
                };

                // Determine texture region to copy buffer to
                TextureRegion dstRegion;
                {
                    dstRegion.subresource   = TextureSubresource{ 0, 0 };
                    dstRegion.offset        = Offset3D{};
                    dstRegion.extent        = FlattenExtent2D(srcRegion.extent);
                };

                // Copy source texture to buffer and back to destination texture
                cmdBuffer->Begin();
                {
                    cmdBuffer->FillBuffer(*buf, 0, FLIP_ENDIAN(0xDEADBEEF), bufDesc.size);
                    cmdBuffer->CopyBufferFromTexture(*buf, 0, *srcTex, srcRegion);
                    cmdBuffer->CopyTextureFromBuffer(*dstTex, dstRegion, *buf, 0);
                }
                cmdBuffer->End();

                // Read back image data from destination texture and compare it with source texture image
                const std::uint32_t numMipTexels    = srcRegion.extent.width * srcRegion.extent.height * srcRegion.extent.depth;
                const std::size_t   subBufSize      = GetMemoryFootprint(format, numMipTexels);

                srcTexImage.resize(subBufSize);
                MutableImageView srcTexImageView;
                {
                    srcTexImageView.format      = formatAttribs.format;
                    srcTexImageView.dataType    = formatAttribs.dataType;
                    srcTexImageView.data        = srcTexImage.data();
                    srcTexImageView.dataSize    = srcTexImage.size();
                }

                dstTexImage.resize(subBufSize);
                MutableImageView dstTexImageView;
                {
                    dstTexImageView.format      = formatAttribs.format;
                    dstTexImageView.dataType    = formatAttribs.dataType;
                    dstTexImageView.data        = dstTexImage.data();
                    dstTexImageView.dataSize    = dstTexImage.size();
                }

                renderer->ReadTexture(*srcTex, srcRegion, srcTexImageView);
                renderer->ReadTexture(*dstTex, dstRegion, dstTexImageView);

                if (::memcmp(srcTexImage.data(), dstTexImage.data(), subBufSize) != 0)
                {
                    const std::string srcDataStr = TestbedContext::FormatByteArray(srcTexImage.data(), srcTexImage.size(), 4, formatAsFloats);
                    const std::string dstDataStr = TestbedContext::FormatByteArray(dstTexImage.data(), dstTexImage.size(), 4, formatAsFloats);
                    Log::Errorf(
                        Log::ColorFlags::StdError,
                        "Mismatch between data of texture %s [MIP %u, Layer %u] and copy result:\n"
                        " -> Expected: [%s]\n"
                        " -> Actual:   [%s]\n",
                        name, mip, layer, srcDataStr.c_str(), dstDataStr.c_str()
                    );
                    return TestResult::FailedMismatch;
                }
                else if (opt.sanityCheck)
                {
                    const std::string dataStr = TestbedContext::FormatByteArray(srcTexImage.data(), srcTexImage.size(), 4, formatAsFloats);
                    Log::Printf(
                        Log::ColorFlags::StdAnnotation,
                        "Sanity check for texture %s [MIP %u, Layer %u]:\n"
                        " -> Data: [%s]\n",
                        name, mip, layer, dataStr.c_str()
                    );
                }

                srcTexImage.clear();
                dstTexImage.clear();
            }
        }

        // Delete old resources
        renderer->Release(*srcTex);
        renderer->Release(*buf);
        renderer->Release(*dstTex);

        // Print duration
        if (opt.showTiming)
        {
            const std::uint64_t t1 = Timer::Tick();
            Log::Printf("Copy texture to buffer: %s ( %f ms )\n", name, TestbedContext::ToMillisecs(t0, t1));
        }

        return TestResult::Passed;
    };

    // Generate random image data sets
    #define SRC_IMAGE_DESC(DECL, FORMAT, TYPE, SRC) \
        const ImageView DECL{ (FORMAT), (TYPE), (SRC).data(), (SRC).size() * sizeof((SRC)[0]) }

    static std::vector<ColorRGBAub> colorsRgbaUb64 = Testset::GenerateColorsRgbaUb(64);
    SRC_IMAGE_DESC(srcImageRgbaUb64, ImageFormat::RGBA, DataType::UInt8,   colorsRgbaUb64);

    static std::vector<float> colorsRgF64 = Testset::GenerateFloats(64 * 2);
    SRC_IMAGE_DESC(srcImageRgF64,    ImageFormat::RG,   DataType::Float32, colorsRgF64   );

    static std::vector<ColorRGBAub> colorsRgbaUb96 = Testset::GenerateColorsRgbaUb(96);
    SRC_IMAGE_DESC(srcImageRgbaUb96, ImageFormat::RGBA, DataType::UInt8,   colorsRgbaUb96);

    static std::vector<float> colorsRF96 = Testset::GenerateFloats(96);
    SRC_IMAGE_DESC(srcImageRF96,     ImageFormat::R,    DataType::Float32, colorsRF96    );

    #define TEST_TEXTURE_TO_BUFFER_COPY(NAME, TYPE, FORMAT, EXTENT, MIPS, LAYERS, SRC)                                  \
        {                                                                                                               \
            TestResult result = CopyToBufferAndReadback((NAME), (TYPE), (FORMAT), (EXTENT), (MIPS), (LAYERS), (SRC));   \
            if (result != TestResult::Passed)                                                                           \
                return result;                                                                                          \
        }

    {
        TEST_TEXTURE_TO_BUFFER_COPY("tex{1D,RgbaUb,64w}",       TextureType::Texture1D,         Format::RGBA8UNorm, Extent3D(64,  1,  1), 2,   1, srcImageRgbaUb64);
        TEST_TEXTURE_TO_BUFFER_COPY("tex{1D,RgF,64w}",          TextureType::Texture1D,         Format::RG32Float,  Extent3D(64,  1,  1), 2,   1, srcImageRgF64   );
    }

    {
        TEST_TEXTURE_TO_BUFFER_COPY("tex{2D,RgbaUb,8wh}",       TextureType::Texture2D,         Format::RGBA8UNorm, Extent3D( 8,  8,  1), 2,   1, srcImageRgbaUb64);
        TEST_TEXTURE_TO_BUFFER_COPY("tex{2D,RgF,8wh}",          TextureType::Texture2D,         Format::RG32Float,  Extent3D( 8,  8,  1), 2,   1, srcImageRgF64   );
    }

    if (caps.features.has3DTextures)
    {
        TEST_TEXTURE_TO_BUFFER_COPY("tex{3D,RgbaUb,4whd}",      TextureType::Texture3D,         Format::RGBA8UNorm, Extent3D( 4,  4,  4), 2,   1, srcImageRgbaUb64);
        TEST_TEXTURE_TO_BUFFER_COPY("tex{3D,RgF,4whd}",         TextureType::Texture3D,         Format::RG32Float,  Extent3D( 4,  4,  4), 2,   1, srcImageRgF64   );
    }

    if (caps.features.hasCubeTextures)
    {
        TEST_TEXTURE_TO_BUFFER_COPY("tex{Cube,RgbaUb,4wh}",     TextureType::TextureCube,       Format::RGBA8UNorm, Extent3D( 4,  4,  1), 2,   6, srcImageRgbaUb96);
        TEST_TEXTURE_TO_BUFFER_COPY("tex{Cube,RF,4wh}",         TextureType::TextureCube,       Format::R32Float,   Extent3D( 4,  4,  1), 2,   6, srcImageRF96    );
    }

    if (caps.features.hasArrayTextures)
    {
        TEST_TEXTURE_TO_BUFFER_COPY("tex{1D[2],RgbaUb,32w}",    TextureType::Texture1DArray,    Format::RGBA8UNorm, Extent3D(32,  1,  1), 2,   2, srcImageRgbaUb64);
        TEST_TEXTURE_TO_BUFFER_COPY("tex{1D[2],RgF,32w}",       TextureType::Texture1DArray,    Format::RG32Float,  Extent3D(32,  1,  1), 2,   2, srcImageRgF64   );

        TEST_TEXTURE_TO_BUFFER_COPY("tex{2D[2],RgbaUb,8w,4h}",  TextureType::Texture2DArray,    Format::RGBA8UNorm, Extent3D( 8,  4,  1), 2,   2, srcImageRgbaUb64);
        TEST_TEXTURE_TO_BUFFER_COPY("tex{2D[2],RgF,8w,4h}",     TextureType::Texture2DArray,    Format::RG32Float,  Extent3D( 8,  4,  1), 2,   2, srcImageRgF64   );
    }

    if (caps.features.hasCubeArrayTextures)
    {
        // Don't test RG32Float format here as some backends don't support this format-texture combination (such as OpenGL)
        TEST_TEXTURE_TO_BUFFER_COPY("tex{Cube[2],RgbaUb,2wh}",  TextureType::TextureCubeArray,  Format::RGBA8UNorm, Extent3D( 2,  2,  1), 2, 6*2, srcImageRgbaUb64);
    }

    return TestResult::Passed;
}

