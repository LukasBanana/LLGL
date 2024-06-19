/*
 * Bindless.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

struct Scene
{
    float4x4    vpMatrix;
    float4x4    wMatrix;
    float4      solidColor;
    float3      lightVec;
    uint        materialId; // 16 bit texture index, 16 bit sampler index
};

struct VertexIn
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    ConstantBuffer<Scene> scene = ResourceDescriptorHeap[0];
    outp.position = mul(scene.vpMatrix, mul(scene.wMatrix, float4(inp.position, 1)));
    outp.normal   = normalize(mul(scene.wMatrix, float4(inp.normal, 0)).xyz);
    outp.texCoord = inp.texCoord;
}

float4 PSMain(VertexOut inp) : SV_Target
{
    ConstantBuffer<Scene> scene = ResourceDescriptorHeap[0];

    uint textureIndex = (scene.materialId >> 16) + 1;
    Texture2D colorMap = ResourceDescriptorHeap[textureIndex];

    uint samplerIndex = (scene.materialId & 0xFFFF) + 3;
    SamplerState linearSampler = SamplerDescriptorHeap[samplerIndex];

    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(scene.lightVec, normal));
    float shading = lerp(0.2, 1.0, NdotL);
    float4 albedo = colorMap.Sample(linearSampler, inp.texCoord);
    return scene.solidColor * albedo * float4((float3)shading, 1.0);
}
