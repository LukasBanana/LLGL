/*
 * TestParse.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>


DEF_RITEST( ParseUtil )
{
    #define TEST_ATTRIB(ATTR)                   \
        if (lhs.ATTR != rhs.ATTR)               \
            return TestResult::FailedMismatch

    auto CompareSamplerDescs = [](const LLGL::SamplerDescriptor& lhs, const LLGL::SamplerDescriptor& rhs) -> TestResult
    {
        TEST_ATTRIB(addressModeU  );
        TEST_ATTRIB(addressModeV  );
        TEST_ATTRIB(addressModeW  );
        TEST_ATTRIB(minFilter     );
        TEST_ATTRIB(magFilter     );
        TEST_ATTRIB(mipMapFilter  );
        TEST_ATTRIB(mipMapEnabled );
        TEST_ATTRIB(mipMapLODBias );
        TEST_ATTRIB(minLOD        );
        TEST_ATTRIB(maxLOD        );
        TEST_ATTRIB(maxAnisotropy );
        TEST_ATTRIB(compareEnabled);
        TEST_ATTRIB(compareOp     );
        TEST_ATTRIB(borderColor[0]);
        TEST_ATTRIB(borderColor[1]);
        TEST_ATTRIB(borderColor[2]);
        TEST_ATTRIB(borderColor[3]);
        return TestResult::Passed;
    };

    #define TEST_SAMPLER_DESCS(LHS, RHS)                                \
        {                                                               \
            const TestResult result = CompareSamplerDescs(LHS, RHS);    \
            if (result != TestResult::Passed)                           \
            {                                                           \
                Log::Errorf("LLGL::Parse(%s) failed\n", #RHS);          \
                return result;                                          \
            }                                                           \
        }

    // Compare sampler descriptor with default initialization
    SamplerDescriptor samplerDesc0A;
    SamplerDescriptor samplerDesc0B = Parse("");
    TEST_SAMPLER_DESCS(samplerDesc0A, samplerDesc0B);

    // Compare sampler descriptor with various different values
    SamplerDescriptor samplerDesc1A;
    {
        samplerDesc1A.addressModeU      = SamplerAddressMode::Clamp;
        samplerDesc1A.addressModeV      = SamplerAddressMode::Clamp;
        samplerDesc1A.addressModeW      = SamplerAddressMode::MirrorOnce;
        samplerDesc1A.minFilter         = SamplerFilter::Nearest;
        samplerDesc1A.magFilter         = SamplerFilter::Nearest;
        samplerDesc1A.mipMapEnabled     = false;
        samplerDesc1A.mipMapLODBias     = 2.5f;
        samplerDesc1A.minLOD            = 2.0f;
        samplerDesc1A.maxLOD            = 5.0f;
        samplerDesc1A.maxAnisotropy     = 8;
        samplerDesc1A.compareEnabled    = true;
        samplerDesc1A.compareOp         = CompareOp::Less;
        samplerDesc1A.borderColor[3]    = 1.0f;
    }
    SamplerDescriptor samplerDesc1B = Parse(
        "address.uv=clamp,"
        "address.w=mirrorOnce,"
        "filter.min=nearest,"
        "filter.mag=nearest,"
        "filter.mip=none,"
        "compare=ls,"
        "anisotropy=0x8,"
        "lod.min=2,"
        "lod.max=5,"
        "lod.bias=2.5,"
        "border=black"
    );
    TEST_SAMPLER_DESCS(samplerDesc1A, samplerDesc1B);

    // Compare sampler descriptor with different values and whitespaces in source string
    SamplerDescriptor samplerDesc2A;
    {
        samplerDesc2A.addressModeU      = SamplerAddressMode::Border;
        samplerDesc2A.addressModeV      = SamplerAddressMode::Border;
        samplerDesc2A.addressModeW      = SamplerAddressMode::Border;
        samplerDesc2A.mipMapFilter      = SamplerFilter::Nearest;
        samplerDesc2A.compareEnabled    = true;
        samplerDesc2A.compareOp         = CompareOp::GreaterEqual;
        samplerDesc2A.mipMapLODBias     = 3.0f;
        samplerDesc2A.minLOD            = 0.25f;
        samplerDesc2A.maxLOD            = 10.0f;
        samplerDesc2A.borderColor[0]    = 1.0f;
        samplerDesc2A.borderColor[1]    = 1.0f;
        samplerDesc2A.borderColor[2]    = 1.0f;
        samplerDesc2A.borderColor[3]    = 1.0f;
    }
    SamplerDescriptor samplerDesc2B = Parse(
        "\taddress = border,\n"
        "\tfilter.mip = nearest,\n"
        "\tcompare = ge,\n"
        "\tlod.min = 0.25,\n"
        "\tlod.max = 10,\n"
        "\tlod.bias = 3,\n"
        "\tborder = white,\n"
    );
    TEST_SAMPLER_DESCS(samplerDesc2A, samplerDesc2B);

    // Test texture swizzling parser
    auto CompareTextureSwizzles = [](const TextureSwizzleRGBA& lhs, const TextureSwizzleRGBA& rhs) -> TestResult
    {
        TEST_ATTRIB(r);
        TEST_ATTRIB(g);
        TEST_ATTRIB(b);
        TEST_ATTRIB(a);
        return TestResult::Passed;
    };

    #define TEST_TEXTURE_SWIZZLE(LHS, RHS)                              \
        {                                                               \
            const TestResult result = CompareTextureSwizzles(LHS, RHS); \
            if (result != TestResult::Passed)                           \
            {                                                           \
                Log::Errorf("LLGL::Parse(%s) failed\n", #RHS);          \
                return result;                                          \
            }                                                           \
        }

    TextureSwizzleRGBA texSwizzle0A;
    {
        texSwizzle0A.r = TextureSwizzle::One;
        texSwizzle0A.g = TextureSwizzle::Zero;
        texSwizzle0A.b = TextureSwizzle::Red;
        texSwizzle0A.a = TextureSwizzle::Green;
    }
    TextureSwizzleRGBA texSwizzle0B = Parse("10rG");
    TEST_TEXTURE_SWIZZLE(texSwizzle0A, texSwizzle0B);

    TextureSwizzleRGBA texSwizzle1A;
    {
        texSwizzle1A.r = TextureSwizzle::Alpha;
        texSwizzle1A.g = TextureSwizzle::Blue;
        texSwizzle1A.b = TextureSwizzle::Green;
        texSwizzle1A.a = TextureSwizzle::Red;
    }
    TextureSwizzleRGBA texSwizzle1B = Parse("abgr");
    TEST_TEXTURE_SWIZZLE(texSwizzle1A, texSwizzle1B);

    TextureSwizzleRGBA texSwizzle1C = Parse("ABGR");
    TEST_TEXTURE_SWIZZLE(texSwizzle1A, texSwizzle1C);

    return TestResult::Passed;
}

