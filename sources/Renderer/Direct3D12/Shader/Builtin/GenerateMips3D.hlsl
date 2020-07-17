/*
 * GenerateMips3D.hlsl
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GenerateMips.hlsli"


/* Classification of non-power-of-two (NPOT) textures: 0...7 */
#define NPOT_TEXTURE_CLASS_EVEN     (0)
#define NPOT_TEXTURE_CLASS_X_ODD    (1)
#define NPOT_TEXTURE_CLASS_Y_ODD    (2)
#define NPOT_TEXTURE_CLASS_XY_ODD   (3)
#define NPOT_TEXTURE_CLASS_Z_ODD    (4)
#define NPOT_TEXTURE_CLASS_XZ_ODD   (5)
#define NPOT_TEXTURE_CLASS_YZ_ODD   (6)
#define NPOT_TEXTURE_CLASS_XYZ_ODD  (7)

#ifndef NPOT_TEXTURE_CLASS
#   define NPOT_TEXTURE_CLASS       (NPOT_TEXTURE_CLASS_EVEN)
#endif

#define BITMASK_XYZ_EVEN            (0x15)


/* Current MIP-map level configuration */
cbuffer TextureDescriptor : register(b0)
{
    float3  texelSize;      // 1.0 / outMipLevel1.extent
    uint    baseMipLevel;   // Base MIP-map level of srcMipLevel
    uint    numMipLevels;   // Number of MIP-map levels to write: [1..4]
};


/* Next 4 output MIP-map levels and source MIP-map level */
RWTexture3D<float4> dstMipLevel1        : register(u0);
RWTexture3D<float4> dstMipLevel2        : register(u1);
RWTexture3D<float4> dstMipLevel3        : register(u2);
Texture3D<float4>   srcMipLevel         : register(t0);
SamplerState        linearClampSampler  : register(s0);


/* Primary compute kernel to generate up to 3 MIP-map levels at a time */
[RootSignature(
    "RootFlags(0),"
    "RootConstants(b0, num32BitConstants = 5),"
    "DescriptorTable(SRV(t0, numDescriptors = 1)),"
    "DescriptorTable(UAV(u0, numDescriptors = 3)),"
    "StaticSampler("
        "s0,"
        "addressU = TEXTURE_ADDRESS_CLAMP,"
        "addressV = TEXTURE_ADDRESS_CLAMP,"
        "addressW = TEXTURE_ADDRESS_CLAMP,"
        "filter = FILTER_MIN_MAG_MIP_LINEAR"
    ")"
)]
[numthreads(4, 4, 4)]
void GenerateMips3DCS(uint groupIndex : SV_GroupIndex, uint3 threadID : SV_DispatchThreadID)
{
    /* Sample source MIP-map level depending on the NPOT texture classification */
    #if NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_EVEN

    float3 uv1 = texelSize * (threadID + 0.5);
    float4 srcColor1 = srcMipLevel.SampleLevel(linearClampSampler, uv1, baseMipLevel);

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_X_ODD

    float3 uv1 = texelSize * (threadID + float3(0.25, 0.5, 0.5));
    float3 uvOffset = texelSize * float3(0.5, 0.0, 0.0);
    float4 srcColor1 = 0.5 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,            baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset, baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_Y_ODD

    float3 uv1 = texelSize * (threadID + float3(0.5, 0.25, 0.5));
    float3 uvOffset = texelSize * float3(0.0, 0.5, 0.0);
    float4 srcColor1 = 0.5 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,            baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset, baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_Z_ODD

    float3 uv1 = texelSize * (threadID + float3(0.5, 0.5, 0.25));
    float3 uvOffset = texelSize * float3(0.0, 0.0, 0.5);
    float4 srcColor1 = 0.5 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,            baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset, baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_XY_ODD

    float3 uv1 = texelSize * (threadID + float3(0.25, 0.25, 0.5));
    float3 uvOffset = texelSize * float3(0.5, 0.5, 0.0);
    float4 srcColor1 = 0.25 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,                                baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(uvOffset.x, 0.0, 0.0), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(0.0, uvOffset.y, 0.0), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset,                     baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_YZ_ODD

    float3 uv1 = texelSize * (threadID + float3(0.5, 0.25, 0.25));
    float3 uvOffset = texelSize * float3(0.0, 0.5, 0.5);
    float4 srcColor1 = 0.25 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,                                baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(0.0, uvOffset.y, 0.0), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(0.0, 0.0, uvOffset.z), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset,                     baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_XZ_ODD

    float3 uv1 = texelSize * (threadID + float3(0.25, 0.5, 0.25));
    float3 uvOffset = texelSize * float3(0.5, 0.0, 0.5);
    float4 srcColor1 = 0.25 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,                                baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(uvOffset.x, 0.0, 0.0), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(0.0, 0.0, uvOffset.z), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset,                     baseMipLevel)
    );

    #elif NPOT_TEXTURE_CLASS == NPOT_TEXTURE_CLASS_XYZ_ODD

    float3 uv1 = texelSize * (threadID + 0.25);
    float3 uvOffset = texelSize * 0.5;
    float4 srcColor1 = 0.125 * (
        srcMipLevel.SampleLevel(linearClampSampler, uv1,                                       baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(uvOffset.x, 0.0, 0.0),        baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(0.0, uvOffset.y, 0.0),        baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(0.0, 0.0, uvOffset.z),        baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(uvOffset.x, uvOffset.y, 0.0), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(0.0, uvOffset.y, uvOffset.z), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + float3(uvOffset.x, 0.0, uvOffset.z), baseMipLevel) +
        srcMipLevel.SampleLevel(linearClampSampler, uv1 + uvOffset,                            baseMipLevel)
    );

    #endif

    /* Write 1st output MIP-map level */
    dstMipLevel1[threadID] = PackLinearColor(srcColor1);

    if (numMipLevels == 1)
        return;

    /* Write 2nd output MIP-map level */
    StoreColor(groupIndex, srcColor1);
    GroupMemoryBarrierWithGroupSync();

    if ((groupIndex & BITMASK_XYZ_EVEN) == 0)
    {
        float4 srcColor2 = LoadColor(groupIndex + 0x01);
        float4 srcColor3 = LoadColor(groupIndex + 0x04);
        float4 srcColor4 = LoadColor(groupIndex + 0x05);
        float4 srcColor5 = LoadColor(groupIndex + 0x10);
        float4 srcColor6 = LoadColor(groupIndex + 0x11);
        float4 srcColor7 = LoadColor(groupIndex + 0x14);
        float4 srcColor8 = LoadColor(groupIndex + 0x15);
        srcColor1 = 0.125 * (srcColor1 + srcColor2 + srcColor3 + srcColor4 + srcColor5 + srcColor6 + srcColor7 + srcColor8);

        dstMipLevel2[threadID / 2] = PackLinearColor(srcColor1);
        StoreColor(groupIndex, srcColor1);
    }

    if (numMipLevels == 2)
        return;

    /* Write 3rd output MIP-map level */
    StoreColor(groupIndex, srcColor1);
    GroupMemoryBarrierWithGroupSync();

    if (groupIndex == 0)
    {
        float4 srcColor2 = LoadColor(0x02);
        float4 srcColor3 = LoadColor(0x08);
        float4 srcColor4 = LoadColor(0x0A);
        float4 srcColor5 = LoadColor(0x20);
        float4 srcColor6 = LoadColor(0x22);
        float4 srcColor7 = LoadColor(0x28);
        float4 srcColor8 = LoadColor(0x2A);
        srcColor1 = 0.125 * (srcColor1 + srcColor2 + srcColor3 + srcColor4 + srcColor5 + srcColor6 + srcColor7 + srcColor8);

        dstMipLevel3[threadID / 4] = PackLinearColor(srcColor1);
    }
}

