/*
 * TestBufferToTextureCopy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include "Testset.h"


/*
Tests the CopyTextureFromBuffer() and CopyBufferFromTexture() functions starting from a buffer with various texture formats.
There is no rendering. The values are only validated via ReadBuffer().
*/
DEF_TEST( BufferToTextureCopy )
{
    const ArrayView<ColorRGBAub> colorsRgbaUb8 = Testset::GetColorsRgbaUb8();
    static_assert(sizeof(colorsRgbaUb8[0]) == 4, "sizeof(ColorRGBAub) must be 4");

    const ArrayView<float> colorsRgF8 = Testset::GetColorsRgF8();

    // Create buffers with initial image data to copy from
    BufferDescriptor buf1Desc;
    {
        buf1Desc.size       = sizeof(colorsRgbaUb8[0]) * colorsRgbaUb8.size();
        buf1Desc.bindFlags  = BindFlags::CopySrc;
    }
    CREATE_BUFFER(buf1, buf1Desc, "buf1{RgbaUb[8]}", colorsRgbaUb8.data());

    BufferDescriptor buf2Desc;
    {
        buf2Desc.size       = sizeof(colorsRgF8[0]) * colorsRgF8.size();
        buf2Desc.bindFlags  = BindFlags::CopySrc;
    }
    CREATE_BUFFER(buf2, buf2Desc, "buf2{RgF[8]}", colorsRgF8.data());

    auto CopyToTextureAndReadback = [this, buf1](
        const char* name, TextureType type, Format format, const Extent3D& extent, std::uint32_t mips, std::uint32_t layers, Buffer* srcBuf) -> TestResult
    {
        const std::uint64_t t0 = Timer::Tick();

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

        // Get source buffer size
        const std::size_t srcBufSize = static_cast<std::size_t>(srcBuf->GetDesc().size);
        if (srcBufSize == 0)
        {
            Log::Errorf("Source buffer cannot be empty for copy test of texture %s\n", name);
            return TestResult::FailedErrors;
        }

        // Create texture to copy from source buffer and to destination buffer
        const std::string texName = std::string("interm.") + name;
        TextureDescriptor texDesc;
        {
            texDesc.type        = type;
            texDesc.bindFlags   = BindFlags::CopySrc | BindFlags::CopyDst;
            texDesc.format      = format;
            texDesc.extent      = extent;
            texDesc.mipLevels   = mips;
            texDesc.arrayLayers = layers;
        }
        CREATE_TEXTURE(tex, texDesc, texName.c_str(), nullptr);

        // Create destination buffer to read back image data
        const char* nameWithoutTexPrefix = (::strncmp(name, "tex", 3) == 0 ? name + 3 : name);
        const std::string dstBufName = std::string("dst.buf") + nameWithoutTexPrefix;
        BufferDescriptor dstBufDesc;
        {
            dstBufDesc.size         = srcBufSize;
            dstBufDesc.bindFlags    = BindFlags::CopyDst;
        }
        CREATE_BUFFER(dstBuf, dstBufDesc, dstBufName.c_str(), nullptr);

        // Run test through all MIP-maps and array layers (should not be more than 2 each)
        const std::uint32_t texDims = NumTextureDimensions(type);

        std::vector<char> srcData, dstData;
        const bool formatAsFloats = IsFloatFormat(format);

        for_range(mip, mips)
        {
            for_range(layer, layers)
            {
                // Determine texture region to copy buffer to and from
                TextureRegion texRegion;
                {
                    texRegion.subresource   = TextureSubresource{ layer, mip };
                    texRegion.offset        = MakeOffset3D(texDims);
                    texRegion.extent        = MakeExtent3D(texDims);
                };

                // Copy source buffer to texture and back to destination buffer
                cmdBuffer->Begin();
                {
                    cmdBuffer->FillBuffer(*dstBuf, 0, FLIP_ENDIAN(0xDEADBEEF), srcBufSize);
                    cmdBuffer->CopyTextureFromBuffer(*tex, texRegion, *srcBuf, 0);
                    cmdBuffer->CopyBufferFromTexture(*dstBuf, 0, *tex, texRegion);
                }
                cmdBuffer->End();

                // Read back image data from destination buffer
                srcData.resize(srcBufSize);
                dstData.resize(srcBufSize);

                renderer->ReadBuffer(*srcBuf, 0, srcData.data(), srcBufSize);
                renderer->ReadBuffer(*dstBuf, 0, dstData.data(), srcBufSize);

                if (::memcmp(srcData.data(), dstData.data(), srcBufSize) != 0)
                {
                    const std::string srcDataStr = TestbedContext::FormatByteArray(srcData.data(), srcData.size(), 4, formatAsFloats);
                    const std::string dstDataStr = TestbedContext::FormatByteArray(dstData.data(), dstData.size(), 4, formatAsFloats);
                    Log::Errorf(
                        "Mismatch between data of texture %s [MIP %u, Layer %u] and copy result:\n"
                        " -> Expected: [%s]\n"
                        " -> Actual:   [%s]\n",
                        name, mip, layer, srcDataStr.c_str(), dstDataStr.c_str()
                    );
                    return TestResult::FailedMismatch;
                }
                else if (opt.sanityCheck)
                {
                    const std::string dataStr = TestbedContext::FormatByteArray(srcData.data(), srcData.size(), 4, formatAsFloats);
                    Log::Printf(
                        Log::ColorFlags::StdAnnotation,
                        "Sanity check for %s [MIP %u, Layer %u]:\n"
                        " -> Data: [%s]\n",
                        name, mip, layer, dataStr.c_str()
                    );
                }

                srcData.clear();
                dstData.clear();
            }
        }

        // Delete old resources
        renderer->Release(*tex);
        renderer->Release(*dstBuf);

        // Print duration
        if (opt.showTiming)
        {
            const std::uint64_t t1 = Timer::Tick();
            Log::Printf("Copy buffer to texture: %s ( %f ms )\n", name, TestbedContext::ToMillisecs(t0, t1));
        }

        return TestResult::Passed;
    };

    #define TEST_BUFFER_TO_TEXTURE_COPY(NAME, TYPE, FORMAT, EXTENT, MIPS, LAYERS, SRC)                                  \
        {                                                                                                               \
            TestResult result = CopyToTextureAndReadback((NAME), (TYPE), (FORMAT), (EXTENT), (MIPS), (LAYERS), (SRC));  \
            if (result != TestResult::Passed)                                                                           \
                return result;                                                                                          \
        }

    {
        TEST_BUFFER_TO_TEXTURE_COPY("tex{1D,RgbaUb,64w}",       TextureType::Texture1D,         Format::RGBA8UNorm, Extent3D(64,  1,  1), 2,   1, buf1);
        TEST_BUFFER_TO_TEXTURE_COPY("tex{1D,RgF,64w}",          TextureType::Texture1D,         Format::RG32Float,  Extent3D(64,  1,  1), 2,   1, buf2);
    }

    {
        TEST_BUFFER_TO_TEXTURE_COPY("tex{2D,RgbaUb,32wh}",      TextureType::Texture2D,         Format::RGBA8UNorm, Extent3D(32, 32,  1), 2,   1, buf1);
        TEST_BUFFER_TO_TEXTURE_COPY("tex{2D,RgF,32wh}",         TextureType::Texture2D,         Format::RG32Float,  Extent3D(32, 32,  1), 2,   1, buf2);
    }

    if (caps.features.has3DTextures)
    {
        TEST_BUFFER_TO_TEXTURE_COPY("tex{3D,RgbaUb,16whd}",     TextureType::Texture3D,         Format::RGBA8UNorm, Extent3D(16, 16, 16), 2,   1, buf1);
        TEST_BUFFER_TO_TEXTURE_COPY("tex{3D,RgF,16whd}",        TextureType::Texture3D,         Format::RG32Float,  Extent3D(16, 16, 16), 2,   1, buf2);
    }

    if (caps.features.hasCubeTextures)
    {
        TEST_BUFFER_TO_TEXTURE_COPY("tex{Cube,RgbaUb,16wh}",    TextureType::TextureCube,       Format::RGBA8UNorm, Extent3D(16, 16,  1), 2,   6, buf1);
        TEST_BUFFER_TO_TEXTURE_COPY("tex{Cube,RgF,16wh}",       TextureType::TextureCube,       Format::RG32Float,  Extent3D(16, 16,  1), 2,   6, buf2);
    }

    if (caps.features.hasArrayTextures)
    {
        TEST_BUFFER_TO_TEXTURE_COPY("tex{1D[2],RgbaUb,64w}",    TextureType::Texture1DArray,    Format::RGBA8UNorm, Extent3D(64,  1,  1), 2,   2, buf1);
        TEST_BUFFER_TO_TEXTURE_COPY("tex{1D[2],RgF,64w}",       TextureType::Texture1DArray,    Format::RG32Float,  Extent3D(64,  1,  1), 2,   2, buf2);

        TEST_BUFFER_TO_TEXTURE_COPY("tex{2D[2],RgbaUb,32wh}",   TextureType::Texture2DArray,    Format::RGBA8UNorm, Extent3D(32, 32,  1), 2,   2, buf1);
        TEST_BUFFER_TO_TEXTURE_COPY("tex{2D[2],RgF,32wh}",      TextureType::Texture2DArray,    Format::RG32Float,  Extent3D(32, 32,  1), 2,   2, buf2);
    }

    if (caps.features.hasCubeArrayTextures)
    {
        // Don't test RG32Float format here as some backends don't support this format-texture combination (such as OpenGL)
        TEST_BUFFER_TO_TEXTURE_COPY("tex{Cube[2],RgbaUb,16wh}", TextureType::TextureCubeArray,  Format::RGBA8UNorm, Extent3D(16, 16,  1), 2, 6*2, buf1);
    }

    // Delete old resources
    renderer->Release(*buf1);
    renderer->Release(*buf2);

    return TestResult::Passed;
}

