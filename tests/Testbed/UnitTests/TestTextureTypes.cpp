/*
 * TestTextureTypes.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( TextureTypes )
{
    TestResult result = TestResult::Passed;

    auto CreateDummyTextureAndMeasureTiming = [this](const char* name, TextureType type, const Extent3D& extent, std::uint32_t mips, std::uint32_t layers, std::uint32_t samples) -> TestResult
    {
        const std::uint64_t t0 = Timer::Tick();

        TextureDescriptor texDesc;
        {
            texDesc.type        = type;
            texDesc.extent      = extent;
            texDesc.mipLevels   = mips;
            texDesc.arrayLayers = layers;
            texDesc.samples     = samples;
        }
        TestResult result = CreateTexture(texDesc, name, nullptr);
        if (result != TestResult::Passed)
            return result;

        // Print duration
        if (opt.showTiming)
        {
            const std::uint64_t t1 = Timer::Tick();
            Log::Printf("Create texture: %s ( %f ms )\n", name, TestbedContext::ToMillisecs(t0, t1));
        }

        return TestResult::Passed;
    };

    #define CREATE_DUMMY(NAME, TYPE, EXTENT, MIPS, LAYERS, SAMPLES)                                                                 \
        {                                                                                                                           \
            TestResult intermResult = CreateDummyTextureAndMeasureTiming((NAME), (TYPE), (EXTENT), (MIPS), (LAYERS), (SAMPLES));    \
            if (intermResult != TestResult::Passed)                                                                                 \
            {                                                                                                                       \
                if (opt.greedy)                                                                                                     \
                    result = intermResult;                                                                                          \
                else                                                                                                                \
                    return intermResult;                                                                                            \
            }                                                                                                                       \
        }

    #define CREATE_DUMMY_SLOW(NAME, TYPE, EXTENT, MIPS, LAYERS, SAMPLES) \
        if (!opt.fastTest)                                                  \
        {                                                                \
            CREATE_DUMMY(NAME, TYPE, EXTENT, MIPS, LAYERS, SAMPLES);     \
        }

    ////////////// Texture1D //////////////

    CREATE_DUMMY     ("tex{1D,1w}",              TextureType::Texture1D, Extent3D(   1, 1, 1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 1);
    CREATE_DUMMY     ("tex{1D,1024w,full-mips}", TextureType::Texture1D, Extent3D(1024, 1, 1), /*mips:*/ 0, /*layers:*/ 1, /*samples:*/ 1);
    CREATE_DUMMY_SLOW("tex{1D,1024w,4-mips}",    TextureType::Texture1D, Extent3D(1024, 1, 1), /*mips:*/ 4, /*layers:*/ 1, /*samples:*/ 1);

    ////////////// Texture1DArray //////////////

    if (caps.features.hasArrayTextures)
    {
        CREATE_DUMMY     ("tex{1D[1],1w}",               TextureType::Texture1DArray, Extent3D(   1, 1, 1), /*mips:*/ 1, /*layers:*/    1, /*samples:*/ 1);
        CREATE_DUMMY     ("tex{1D[10],1w}",              TextureType::Texture1DArray, Extent3D(   1, 1, 1), /*mips:*/ 1, /*layers:*/   10, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{1D[64],1024w,full-mips}", TextureType::Texture1DArray, Extent3D(1024, 1, 1), /*mips:*/ 0, /*layers:*/   64, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{1D[1024],1024w,6-mips}",  TextureType::Texture1DArray, Extent3D(1024, 1, 1), /*mips:*/ 6, /*layers:*/ 1024, /*samples:*/ 1);
    }

    ////////////// Texture2D //////////////

    CREATE_DUMMY     ("tex{2D,1wh}",                 TextureType::Texture2D, Extent3D(   1,    1, 1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 1);
    CREATE_DUMMY     ("tex{2D,1024wh,full-mips}",    TextureType::Texture2D, Extent3D(1024, 1024, 1), /*mips:*/ 0, /*layers:*/ 1, /*samples:*/ 1);
    CREATE_DUMMY_SLOW("tex{2D,1024w,256h,3-mips}",   TextureType::Texture2D, Extent3D(1024,  256, 1), /*mips:*/ 4, /*layers:*/ 1, /*samples:*/ 1);
    CREATE_DUMMY_SLOW("tex{2D,800w,600h,full-mips}", TextureType::Texture2D, Extent3D( 800,  600, 1), /*mips:*/ 0, /*layers:*/ 1, /*samples:*/ 1);
    CREATE_DUMMY_SLOW("tex{2D,123w,456h,full-mips}", TextureType::Texture2D, Extent3D( 123,  456, 1), /*mips:*/ 0, /*layers:*/ 1, /*samples:*/ 1);

    ////////////// Texture2DMS //////////////

    if (caps.features.hasMultiSampleTextures)
    {
        CREATE_DUMMY     ("tex{2DMS,1wh}",        TextureType::Texture2DMS, Extent3D(   1,    1, 1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 1);
        CREATE_DUMMY     ("tex{2DMS,1024wh}",     TextureType::Texture2DMS, Extent3D(1024, 1024, 1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 2);
        CREATE_DUMMY_SLOW("tex{2DMS,1024w,256h}", TextureType::Texture2DMS, Extent3D(1024,  256, 1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 4);
        CREATE_DUMMY_SLOW("tex{2DMS,800w,600h}",  TextureType::Texture2DMS, Extent3D( 800,  600, 1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 8);
        CREATE_DUMMY_SLOW("tex{2DMS,123w,456h}",  TextureType::Texture2DMS, Extent3D( 123,  456, 1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 8);
    }

    ////////////// Texture2DArray //////////////

    if (caps.features.hasArrayTextures)
    {
        CREATE_DUMMY     ("tex{2D[1],1wh}",                  TextureType::Texture2DArray, Extent3D(   1,    1, 1), /*mips:*/ 1, /*layers:*/    1, /*samples:*/ 1);
        CREATE_DUMMY     ("tex{2D[1024],32wh}",              TextureType::Texture2DArray, Extent3D(  32,   32, 1), /*mips:*/ 0, /*layers:*/ 1024, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{2D[16],1024wh,full-mips}",    TextureType::Texture2DArray, Extent3D(1024, 1024, 1), /*mips:*/ 0, /*layers:*/   16, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{2D[64],1024w,256h,3-mips}",   TextureType::Texture2DArray, Extent3D(1024,  256, 1), /*mips:*/ 3, /*layers:*/   64, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{2D[32],800w,600h,full-mips}", TextureType::Texture2DArray, Extent3D( 800,  600, 1), /*mips:*/ 0, /*layers:*/   32, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{2D[13],123w,456h,full-mips}", TextureType::Texture2DArray, Extent3D( 123,  456, 1), /*mips:*/ 0, /*layers:*/   13, /*samples:*/ 1);
    }

    ////////////// Texture2DMSArray //////////////

    if (caps.features.hasMultiSampleArrayTextures)
    {
        CREATE_DUMMY     ("tex{2DMS[1],1wh,1x}",         TextureType::Texture2DMSArray, Extent3D(   1,    1, 1), /*mips:*/ 1, /*layers:*/    1, /*samples:*/ 1);
        CREATE_DUMMY     ("tex{2DMS[1024],32wh,2x}",     TextureType::Texture2DMSArray, Extent3D(  32,   32, 1), /*mips:*/ 1, /*layers:*/ 1024, /*samples:*/ 2);
        CREATE_DUMMY_SLOW("tex{2DMS[16],1024wh,4x}",     TextureType::Texture2DMSArray, Extent3D(1024, 1024, 1), /*mips:*/ 1, /*layers:*/   16, /*samples:*/ 4);
        CREATE_DUMMY_SLOW("tex{2DMS[64],1024w,256h,8x}", TextureType::Texture2DMSArray, Extent3D(1024,  256, 1), /*mips:*/ 1, /*layers:*/   64, /*samples:*/ 8);
        CREATE_DUMMY_SLOW("tex{2DMS[32],800w,600h,8x}",  TextureType::Texture2DMSArray, Extent3D( 800,  600, 1), /*mips:*/ 1, /*layers:*/   32, /*samples:*/ 8);
        CREATE_DUMMY_SLOW("tex{2DMS[13],123w,456h,8x}",  TextureType::Texture2DMSArray, Extent3D( 123,  456, 1), /*mips:*/ 1, /*layers:*/   13, /*samples:*/ 8);
    }

    ////////////// Texture3D //////////////

    if (caps.features.has3DTextures)
    {
        CREATE_DUMMY     ("tex{3D,1w,1h}",                    TextureType::Texture3D, Extent3D(   1,   1,   1), /*mips:*/ 1, /*layers:*/ 1, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{3D,256whd,full-mips}",         TextureType::Texture3D, Extent3D( 256, 256, 256), /*mips:*/ 0, /*layers:*/ 1, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{3D,1024w,256h,64d,4-mips}",    TextureType::Texture3D, Extent3D(1024, 256,  64), /*mips:*/ 4, /*layers:*/ 1, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{3D,800w,600h,32d,full-mips}",  TextureType::Texture3D, Extent3D( 800, 600,  32), /*mips:*/ 0, /*layers:*/ 1, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{3D,123w,456h,789d,full-mips}", TextureType::Texture3D, Extent3D( 123, 456, 789), /*mips:*/ 0, /*layers:*/ 1, /*samples:*/ 1);
    }

    ////////////// TextureCube //////////////

    if (caps.features.hasCubeTextures)
    {
        CREATE_DUMMY     ("tex{Cube,1wh}",             TextureType::TextureCube, Extent3D(  1,   1, 1), /*mips:*/ 1, /*layers:*/ 6, /*samples:*/ 1);
        CREATE_DUMMY     ("tex{Cube,32wh}",            TextureType::TextureCube, Extent3D( 32,  32, 1), /*mips:*/ 0, /*layers:*/ 6, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube,128wh,full-mips}", TextureType::TextureCube, Extent3D(128, 128, 1), /*mips:*/ 0, /*layers:*/ 6, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube,256h,3-mips}",     TextureType::TextureCube, Extent3D(256, 256, 1), /*mips:*/ 3, /*layers:*/ 6, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube,600wh,full-mips}", TextureType::TextureCube, Extent3D(600, 600, 1), /*mips:*/ 0, /*layers:*/ 6, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube,123wh,full-mips}", TextureType::TextureCube, Extent3D(123, 123, 1), /*mips:*/ 0, /*layers:*/ 6, /*samples:*/ 1);
    }

    ////////////// TextureCubeArray //////////////

    if (caps.features.hasCubeArrayTextures)
    {
        CREATE_DUMMY     ("tex{Cube[6],1wh}",              TextureType::TextureCubeArray, Extent3D(  1,   1, 1), /*mips:*/ 1, /*layers:*/   6, /*samples:*/ 1);
        CREATE_DUMMY     ("tex{Cube[600],32wh}",           TextureType::TextureCubeArray, Extent3D( 32,  32, 1), /*mips:*/ 0, /*layers:*/ 600, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube[18],128wh,full-mips}", TextureType::TextureCubeArray, Extent3D(128, 128, 1), /*mips:*/ 0, /*layers:*/  18, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube[60],256wh,3-mips}",    TextureType::TextureCubeArray, Extent3D(256, 256, 1), /*mips:*/ 3, /*layers:*/  60, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube[30],600wh,full-mips}", TextureType::TextureCubeArray, Extent3D(600, 600, 1), /*mips:*/ 0, /*layers:*/  30, /*samples:*/ 1);
        CREATE_DUMMY_SLOW("tex{Cube[12],123wh,full-mips}", TextureType::TextureCubeArray, Extent3D(123, 123, 1), /*mips:*/ 0, /*layers:*/  12, /*samples:*/ 1);
    }

    return result;
}

