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
    #define TEST_ATTRIB(ATTR)       \
        if (lhs.ATTR != rhs.ATTR)   \
            return false

    auto CompareSamplerDescsEqual = [](const LLGL::SamplerDescriptor& lhs, const LLGL::SamplerDescriptor& rhs) -> bool
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
        return true;
    };

    #define TEST_SAMPLER_DESCS(LHS, RHS)                                \
        {                                                               \
            if (!CompareSamplerDescsEqual(LHS, RHS))                    \
            {                                                           \
                Log::Errorf("LLGL::Parse(%s) failed\n", #RHS);          \
                return TestResult::FailedMismatch;                      \
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
    auto CompareTextureSwizzlesEqual = [](const TextureSwizzleRGBA& lhs, const TextureSwizzleRGBA& rhs) -> bool
    {
        TEST_ATTRIB(r);
        TEST_ATTRIB(g);
        TEST_ATTRIB(b);
        TEST_ATTRIB(a);
        return true;
    };

    #define TEST_TEXTURE_SWIZZLE(LHS, RHS)                      \
        {                                                       \
            if (!CompareTextureSwizzlesEqual(LHS, RHS))         \
            {                                                   \
                Log::Errorf("LLGL::Parse(%s) failed\n", #RHS);  \
                return TestResult::FailedMismatch;              \
            }                                                   \
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

    // Test parsing PSO layout
    auto CompareBindingDescEqual = [](const BindingDescriptor& lhs, const BindingDescriptor& rhs) -> bool
    {
        TEST_ATTRIB(name      );
        TEST_ATTRIB(type      );
        TEST_ATTRIB(bindFlags );
        TEST_ATTRIB(stageFlags);
        TEST_ATTRIB(slot.index);
        TEST_ATTRIB(slot.set  );
        TEST_ATTRIB(arraySize );
        return true;
    };

    auto CompareUniformDescEqual = [](const UniformDescriptor& lhs, const UniformDescriptor& rhs) -> bool
    {
        TEST_ATTRIB(name     );
        TEST_ATTRIB(type     );
        TEST_ATTRIB(arraySize);
        return true;
    };

    auto CompareStaticSamplerDescEqual = [&](const StaticSamplerDescriptor& lhs, const StaticSamplerDescriptor& rhs) -> bool
    {
        TEST_ATTRIB(name      );
        TEST_ATTRIB(stageFlags);
        TEST_ATTRIB(slot.index);
        TEST_ATTRIB(slot.set  );
        return CompareSamplerDescsEqual(lhs.sampler, rhs.sampler);
    };

    auto CompareCombinedTextureSamplerDescEqual = [](const CombinedTextureSamplerDescriptor& lhs, const CombinedTextureSamplerDescriptor& rhs) -> bool
    {
        TEST_ATTRIB(name       );
        TEST_ATTRIB(textureName);
        TEST_ATTRIB(samplerName);
        TEST_ATTRIB(slot.index );
        TEST_ATTRIB(slot.set   );
        return true;
    };

    auto ComparePSOLayoutDescsEqual = [&](const PipelineLayoutDescriptor& lhs, const PipelineLayoutDescriptor& rhs) -> bool
    {
        if (lhs.heapBindings.size()             != rhs.heapBindings.size()      ||
            lhs.bindings.size()                 != rhs.bindings.size()          ||
            lhs.staticSamplers.size()           != rhs.staticSamplers.size()    ||
            lhs.uniforms.size()                 != rhs.uniforms.size()          ||
            lhs.combinedTextureSamplers.size()  != rhs.combinedTextureSamplers.size())
        {
            return false;
        }
        for_range(i, lhs.heapBindings.size())
        {
            if (!CompareBindingDescEqual(lhs.heapBindings[i], rhs.heapBindings[i]))
                return false;
        }
        for_range(i, lhs.bindings.size())
        {
            if (!CompareBindingDescEqual(lhs.bindings[i], rhs.bindings[i]))
                return false;
        }
        for_range(i, lhs.staticSamplers.size())
        {
            if (!CompareStaticSamplerDescEqual(lhs.staticSamplers[i], rhs.staticSamplers[i]))
                return false;
        }
        for_range(i, lhs.uniforms.size())
        {
            if (!CompareUniformDescEqual(lhs.uniforms[i], rhs.uniforms[i]))
                return false;
        }
        TEST_ATTRIB(barrierFlags);
        return true;
    };

    #define TEST_PARSE_PSO_LAYOUT(LAYOUT_CMP, LAYOUT_STR, ...)                   \
        {                                                                   \
            const char* psoLayoutStr = (LAYOUT_STR);                        \
            PipelineLayoutDescriptor psoLayoutParsed = Parse(psoLayoutStr , ## __VA_ARGS__); \
            if (!ComparePSOLayoutDescsEqual(psoLayoutParsed, (LAYOUT_CMP))) \
            {                                                               \
                Log::Errorf("LLGL::Parse(%s) failed\n", psoLayoutStr);      \
                return TestResult::FailedMismatch;                          \
            }                                                               \
        }

    SamplerDescriptor smplB;
    {
        smplB.minFilter = SamplerFilter::Nearest;
        smplB.magFilter = SamplerFilter::Nearest;
        smplB.mipMapFilter = SamplerFilter::Nearest;
    }
    PipelineLayoutDescriptor psoLayoutA;
    {
        psoLayoutA.heapBindings =
        {
            BindingDescriptor{ "Scene",       ResourceType::Buffer, BindFlags::ConstantBuffer, StageFlags::VertexStage, 0 },
            BindingDescriptor{ "outVertices", ResourceType::Buffer, BindFlags::Storage,        StageFlags::VertexStage, 0 },
        };
        psoLayoutA.bindings =
        {
            BindingDescriptor{ "texA",  ResourceType::Texture, BindFlags::Sampled, StageFlags::VertexStage | StageFlags::FragmentStage, 1, 2 },
            BindingDescriptor{ "texB",  ResourceType::Texture, BindFlags::Sampled, StageFlags::VertexStage | StageFlags::FragmentStage, 3 },
            BindingDescriptor{ "smplA", ResourceType::Sampler, 0,                  StageFlags::FragmentStage,                           4 },
        };
        psoLayoutA.staticSamplers =
        {
            StaticSamplerDescriptor{ "smplB", StageFlags::FragmentStage, 5, smplB },
        };
        psoLayoutA.uniforms =
        {
            UniformDescriptor{ "wvpMatrix", UniformType::Float4x4 },
            UniformDescriptor{ "offsets", UniformType::Int4, 3 },
            UniformDescriptor{ "origin", UniformType::Int4 },
        };
        psoLayoutA.combinedTextureSamplers =
        {
            CombinedTextureSamplerDescriptor{ "texB_smplA", "texB", "smplA", 4 },
            CombinedTextureSamplerDescriptor{ "texB_smplB", "texB", "smplB", 5 },
        };
        psoLayoutA.barrierFlags = BarrierFlags::StorageBuffer;
    }
    TEST_PARSE_PSO_LAYOUT(
        psoLayoutA,
        "heap{"
        "cbuffer(Scene@0):vert,"
        "rwbuffer(outVertices@0):vert,"
        "},"
        "texture(texA@1[2],texB@3):vert:frag,"
        "sampler(smplA@4):frag,"
        "sampler(smplB@5){filter=nearest}:frag,"
        "sampler<texB,smplA>(texB_smplA@4),"
        "sampler<texB,smplB>(texB_smplB@5),"
        "float4x4(wvpMatrix),"
        "int4(offsets[3],origin),"
        "barriers{rwbuffer},"
    );

    TEST_PARSE_PSO_LAYOUT(
        psoLayoutA,
        "\theap { \n"
        "\t\tcbuffer ( Scene @ 0 ) : vert ,\n"
        "\t\trwbuffer ( outVertices @ 0 ) : vert\n"
        "\t},\n"
        "\ttexture ( texA @ 1 [ %d ] , texB @ %d ) : vert : frag , \n"
        "\tsampler ( smplA @ 4 ) : frag,\n"
        "\tsampler ( smplB @ 5 ) { filter = nearest } : frag , \n"
        "\tsampler < texB , smplA > ( texB_smplA@4 ) , \n"
        "\tsampler < texB , smplB > ( texB_smplB@5 ) , \n"
        "\tfloat4x4 ( wvpMatrix ) , \n"
        "\tint4 ( offsets [ %d ] , origin ) , \n"
        "\tbarriers { rwbuffer }\n",
        2, 3, 3
    );

    return TestResult::Passed;
}

