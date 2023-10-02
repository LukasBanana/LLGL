/*
 * TestParse.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>


TestResult TestbedContext::TestParseSamplerDesc()
{
    #define TEST_SAMPLER_ATTRIB(ATTR)           \
        if (lhs.ATTR != rhs.ATTR)               \
            return TestResult::FailedMismatch

    auto CompareSamplerDescs = [](const LLGL::SamplerDescriptor& lhs, const LLGL::SamplerDescriptor& rhs) -> TestResult
    {
        TEST_SAMPLER_ATTRIB(addressModeU  );
        TEST_SAMPLER_ATTRIB(addressModeV  );
        TEST_SAMPLER_ATTRIB(addressModeW  );
        TEST_SAMPLER_ATTRIB(minFilter     );
        TEST_SAMPLER_ATTRIB(magFilter     );
        TEST_SAMPLER_ATTRIB(mipMapFilter  );
        TEST_SAMPLER_ATTRIB(mipMapEnabled );
        TEST_SAMPLER_ATTRIB(mipMapLODBias );
        TEST_SAMPLER_ATTRIB(minLOD        );
        TEST_SAMPLER_ATTRIB(maxLOD        );
        TEST_SAMPLER_ATTRIB(maxAnisotropy );
        TEST_SAMPLER_ATTRIB(compareEnabled);
        TEST_SAMPLER_ATTRIB(compareOp     );
        TEST_SAMPLER_ATTRIB(borderColor[0]);
        TEST_SAMPLER_ATTRIB(borderColor[1]);
        TEST_SAMPLER_ATTRIB(borderColor[2]);
        TEST_SAMPLER_ATTRIB(borderColor[3]);
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

    return TestResult::Passed;
}

