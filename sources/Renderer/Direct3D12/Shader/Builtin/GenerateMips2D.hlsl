/*
 * GenerateMips2D.hlsl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GenerateMips.hlsli"


/* Classification of non-power-of-two (NPOT) textures: 0, 1, 2, 3 */
#define NPOT_TEXTURE_CLASS_XY_EVEN      (0)
#define NPOT_TEXTURE_CLASS_X_ODD_Y_EVEN (1)
#define NPOT_TEXTURE_CLASS_X_EVEN_Y_ODD (2)
#define NPOT_TEXTURE_CLASS_XY_ODD       (3)

#ifndef NPOT_TEXTURE_CLASS
#   define NPOT_TEXTURE_CLASS           (NPOT_TEXTURE_CLASS_XY_EVEN)
#endif

#define BITMASK_XY_EVEN                 (0x09)
#define BITMASK_XY_MULTIPLE_OF_4        (0x1B)


/* Current MIP-map level configuration */
cbuffer TextureDescriptor : register(b0)
{
    uint    baseMipLevel;   // Base MIP-map level of srcMipLevel
    uint    numMipLevels;   // Number of MIP-map levels to write: [1..4]
    float2  texelSize;      // 1.0 / outMipLevel1.extent
};


/* Next 4 output MIP-map levels and source MIP-map level */
RWTexture2D<float4> dstMipLevel1        : register(u0);
RWTexture2D<float4> dstMipLevel2        : register(u1);
RWTexture2D<float4> dstMipLevel3        : register(u2);
RWTexture2D<float4> dstMipLevel4        : register(u3);
Texture2D<float4>   srcMipLevel         : register(t0);
SamplerState        linearClampSampler  : register(s0);


/* Primary compute kernel to up to 4 MIP-map levels at a time */
[RootSignature(
    "RootFlags(0),"
    "RootConstants(b0, num32BitConstants = 4),"
    "DescriptorTable(SRV(t0, numDescriptors = 1)),"
    "DescriptorTable(UAV(u0, numDescriptors = 4)),"
    "StaticSampler("
        "s0,"
        "addressU = TEXTURE_ADDRESS_CLAMP,"
        "addressV = TEXTURE_ADDRESS_CLAMP,"
        "addressW = TEXTURE_ADDRESS_CLAMP,"
        "filter = FILTER_MIN_MAG_MIP_LINEAR"
    ")"
)]
[numthreads(8, 8, 1)]
void GenerateMips2DCS(uint groupIndex : SV_GroupIndex, uint3 threadID : SV_DispatchThreadID)
{
    /* Sample source MIP-map level depending on the NPOT texture classification */
    #if NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_XY_EVEN

    float2 uv1 = texelSize * (threadID.xy + 0.5);
    float4 srcColor1 = srcMipLevel.SampleLevel(linearClampSampler, uv1, baseMipLevel);

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_X_ODD_Y_EVEN

    float2 uv1 = texelSize * (threadID.xy + float2(0.25, 0.5));
    float2 uvOffset = texelSize * float2(0.5, 0.0);
    float4 srcColor1 = 0.5 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,            baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset, baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_X_EVEN_Y_ODD

    float2 uv1 = texelSize * (threadID.xy + float2(0.5, 0.25));
    float2 uvOffset = texelSize * float2(0.0, 0.5);
    float4 srcColor1 = 0.5 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,            baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset, baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_XY_ODD

    float2 uv1 = texelSize * (threadID.xy + 0.25);
    float2 uvOffset = texelSize * 0.5;
    float4 srcColor1 = 0.25 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,                           baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float2(uvOffset.x, 0.0), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float2(0.0, uvOffset.y), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset,                baseMipLevel)
    );

    #endif

    /* Write 1st output MIP-map level */
    dstMipLevel1[threadID.xy] = PackLinearColor(srcColor1);

    if (numMipLevels == 1)
        return;

    /* Write 2nd output MIP-map level */
    StoreColor(groupIndex, srcColor1);
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_XY_EVEN) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x01);
        float4 srcColor3 = LoadColor(groupIndex + 0x08);
        float4 srcColor4 = LoadColor(groupIndex + 0x09);
        srcColor1 = 0.25 * (srcColor1 + srcColor2 + srcColor3 + srcColor4);

        dstMipLevel2[threadID.xy / 2] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 2)
        return;

    /* Write 3rd output MIP-map level */
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_XY_MULTIPLE_OF_4) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x02);
        float4 srcColor3 = LoadColor(groupIndex + 0x10);
        float4 srcColor4 = LoadColor(groupIndex + 0x12);
        srcColor1 = 0.25 * (srcColor1 + srcColor2 + srcColor3 + srcColor4);

        dstMipLevel3[threadID.xy / 4] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 3)
        return;

    /* Write 4th output MIP-map level */
    GroupMemoryBarrierWithGroupSync();

    if (groupIndex == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x04);
        float4 srcColor3 = LoadColor(groupIndex + 0x20);
        float4 srcColor4 = LoadColor(groupIndex + 0x24);
        srcColor1 = 0.25 * (srcColor1 + srcColor2 + srcColor3 + srcColor4);

        dstMipLevel4[threadID.xy / 8] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }
}

