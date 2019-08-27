/*
 * GenerateMips1D.hlsl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GenerateMips.hlsli"


/* Classification of non-power-of-two (NPOT) textures: 0, 1 */
#define NPOT_TEXTURE_CLASS_X_EVEN   (0)
#define NPOT_TEXTURE_CLASS_X_ODD    (1)

#ifndef NPOT_TEXTURE_CLASS
#   define NPOT_TEXTURE_CLASS       (NPOT_TEXTURE_CLASS_X_EVEN)
#endif

#define BITMASK_X_EVEN              (0x01)
#define BITMASK_X_MULTIPLE_OF_4     (0x03)
#define BITMASK_X_MULTIPLE_OF_8     (0x07)
#define BITMASK_X_MULTIPLE_OF_16    (0x0F)
#define BITMASK_X_MULTIPLE_OF_32    (0x1F)


/* Current MIP-map level configuration */
cbuffer TextureDescriptor : register(b0)
{
    uint    baseMipLevel;   // Base MIP-map level of srcMipLevel
    uint    numMipLevels;   // Number of MIP-map levels to write: [1..4]
    float   texelSize;      // 1.0 / outMipLevel1.extent
    float   pad0;
};


/* Next 4 output MIP-map levels and source MIP-map level */
RWTexture1D<float4> dstMipLevel1        : register(u0);
RWTexture1D<float4> dstMipLevel2        : register(u1);
RWTexture1D<float4> dstMipLevel3        : register(u2);
RWTexture1D<float4> dstMipLevel4        : register(u3);
RWTexture1D<float4> dstMipLevel5        : register(u4);
RWTexture1D<float4> dstMipLevel6        : register(u5);
RWTexture1D<float4> dstMipLevel7        : register(u6);
RWTexture1D<float4> dstMipLevel8        : register(u7);
Texture1D<float4>   srcMipLevel         : register(t0);
SamplerState        linearClampSampler  : register(s0);


/* Primary compute kernel to up to 8 MIP-map levels at a time */
[RootSignature(
    "RootFlags(0),"
    "RootConstants(b0, num32BitConstants = 4),"
    "DescriptorTable(SRV(t0, numDescriptors = 1)),"
    "DescriptorTable(UAV(u0, numDescriptors = 8)),"
    "StaticSampler("
        "s0,"
        "addressU = TEXTURE_ADDRESS_CLAMP,"
        "addressV = TEXTURE_ADDRESS_CLAMP,"
        "addressW = TEXTURE_ADDRESS_CLAMP,"
        "filter = FILTER_MIN_MAG_MIP_LINEAR"
    ")"
)]
[numthreads(64, 1, 1)]
void GenerateMips1DCS(uint groupIndex : SV_GroupIndex, uint3 threadID : SV_DispatchThreadID)
{
    /* Sample source MIP-map level depending on the NPOT texture classification */
    #if NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_X_EVEN

    float uv1 = texelSize * (threadID.x + 0.5);
    float4 srcColor1 = srcMipLevel.SampleLevel(linearClampSampler, uv1, baseMipLevel);

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_X_ODD

    float uv1 = texelSize * (threadID.x + 0.25);
    float2 uvOffset = texelSize * 0.5;
    float4 srcColor1 = 0.5 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,            baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset, baseMipLevel)
    );

    #endif

    /* Write 1st output MIP-map level */
    dstMipLevel1[threadID.x] = PackLinearColor(srcColor1);

    if (numMipLevels == 1)
        return;

    /* Write 2nd output MIP-map level */
    StoreColor(groupIndex, srcColor1);
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_X_EVEN) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x01);
        srcColor1 = 0.5 * (srcColor1 + srcColor2);

        dstMipLevel2[threadID.x / 2] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 2)
        return;

    /* Write 3rd output MIP-map level */
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_X_MULTIPLE_OF_4) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x02);
        srcColor1 = 0.5 * (srcColor1 + srcColor2);

        dstMipLevel3[threadID.x / 4] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 3)
        return;

    /* Write 4th output MIP-map level */
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_X_MULTIPLE_OF_8) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x04);
        srcColor1 = 0.5 * (srcColor1 + srcColor2);

        dstMipLevel4[threadID.x / 8] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 4)
        return;

    /* Write 5th output MIP-map level */
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_X_MULTIPLE_OF_16) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x08);
        srcColor1 = 0.5 * (srcColor1 + srcColor2);

        dstMipLevel5[threadID.x / 16] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 5)
        return;

    /* Write 6th output MIP-map level */
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_X_MULTIPLE_OF_32) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x0F);
        srcColor1 = 0.5 * (srcColor1 + srcColor2);

        dstMipLevel6[threadID.x / 32] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 6)
        return;

    /* Write 7th output MIP-map level */
    GroupMemoryBarrierWithGroupSync();

    if (groupIndex == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x1F);
        srcColor1 = 0.5 * (srcColor1 + srcColor2);

        dstMipLevel7[threadID.x / 64] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }
}

