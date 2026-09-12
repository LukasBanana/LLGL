// MorphTargets HLSL shader
// Written by L. Hermanns 9/9/2026

// GLES and WebGL don't support GL_CLAMP_TO_BORDER, so we emulate it in the shader
#ifndef EMULATE_BORDER_SAMPLER
#define EMULATE_BORDER_SAMPLER 0
#endif

#define BORDER_SAMPLER_MODE_NONE                0
#define BORDER_SAMPLER_MODE_BLACK_TRANSPARENT   1
#define BORDER_SAMPLER_MODE_BLACK               2

// Start with cbuffer slot 3, as Metal will reserve the first three slots for the vertex buffers in VMorphTargetMesh() entry point
cbuffer SceneView : register(b3)
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4   lightVec;
    float4   baseColor;
};

struct DynamicState_t
{
    float   texCoordScaleFront;
    float   texCoordScaleBack;

    // Interpolation factor for morph targets in range [0.0, 1.0]
    float   interpolationFactor;
    float   invertXAxis;

    // Only used for GLES and WebGL, but always include it to simplify uniform uploads
    int     borderSamplerFront;
    int     borderSamplerBack;
};

#if __spirv__
[[vk::push_constant]] DynamicState_t dynamicState;
#else
cbuffer DynamicState : register(b0)
{
    DynamicState_t dynamicState;
}
#endif

float2 TransformTexCoord(float2 texCoord, float scale)
{
    return (texCoord - (float2)0.5)*scale + (float2)0.5;
}

float4 SampleColorMap(Texture2D colorMap, SamplerState colorMapSampler, float2 texCoord, int borderSampler)
{
    #if EMULATE_BORDER_SAMPLER
    if (borderSampler != BORDER_SAMPLER_MODE_NONE)
    {
        // Use step functions to determine whether the texture-coordinates are inside or outside the [0, 1] range, to avoid dynamic branching
        float2 inside = step((float2)0.0, texCoord) * step(texCoord, (float2)1.0);
        float insideFactor = inside.x * inside.y;
        float4 sampledColor = colorMap.Sample(colorMapSampler, texCoord);
        float4 borderColor = (borderSampler == BORDER_SAMPLER_MODE_BLACK_TRANSPARENT ? float4(0, 0, 0, 0) : float4(0, 0, 0, 1));
        return lerp(borderColor, sampledColor, insideFactor);
    }
    #endif

    return colorMap.Sample(colorMapSampler, texCoord);
}


// VERTEX SHADER STATIC MESH

struct StaticMeshVertexIn
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
    nointerpolation float flipFace : FLIPFACE;
};

void VStaticMesh(StaticMeshVertexIn inp, out VertexOut outp)
{
    outp.position   = mul(wvpMatrix, float4(inp.position, 1));
    outp.normal     = mul(wMatrix, float4(inp.normal, 0)).xyz;
    outp.texCoord   = TransformTexCoord(inp.texCoord, dynamicState.texCoordScaleFront);
    outp.flipFace   = 1.0; // Default value for static mesh
}


// STATIC MESH PIXEL SHADER

Texture2D<float3> paperDetailMap : register(t4);
SamplerState paperDetailMapSampler : register(s5);

Texture2D colorMap : register(t6);
SamplerState colorMapSampler : register(s7);

float3 SampleDetailMap(float2 texCoord)
{
    // Sample detail map to simulate paper surface
    return paperDetailMap.Sample(paperDetailMapSampler, texCoord) - (float3)0.5;
}

float4 FinalShading(float4 color, float3 normal, float alpha, float3 paperDetail)
{
    float NdotL = dot(lightVec.xyz, normalize(normal));
    float lighting = lerp(0.2, 1.0, NdotL);

    float3 opaqueColor = lerp(baseColor.rgb, color.rgb, color.a);
    float4 finalColor = (baseColor + float4(paperDetail, 0)) * float4(opaqueColor, alpha);

    return float4(finalColor.rgb * lighting, finalColor.a);
}

float4 PStaticMesh(VertexOut inp) : SV_Target
{
    // Sample base color map and apply alpha mask
    float4 color = SampleColorMap(colorMap, colorMapSampler, inp.texCoord, dynamicState.borderSamplerFront);

    // Sample detail map to simulate paper surface
    float3 paperDetail = SampleDetailMap(inp.texCoord);

    // Apply lambert factor for simple shading
    const float alpha = 1.0;
    return FinalShading(color, inp.normal, alpha, paperDetail);
}


// VERTEX SHADER MORPH-TARGET MESH

struct MorphTargetVertexIn
{
    float3 positionA : POSITIONA;
    float3 normalA   : NORMALA;
    float3 positionB : POSITIONB;
    float3 normalB   : NORMALB;
    float2 texCoord  : TEXCOORD;
};

void VMorphTargetMesh(MorphTargetVertexIn inp, out VertexOut outp)
{
    float3 interpolatedPosition = lerp(inp.positionA, inp.positionB, dynamicState.interpolationFactor);
    interpolatedPosition.x *= dynamicState.invertXAxis;
    outp.position   = mul(wvpMatrix, float4(interpolatedPosition, 1));

    float3 interpolatedNormal = normalize(lerp(inp.normalA, inp.normalB, dynamicState.interpolationFactor));
    outp.normal     = mul(wMatrix, float4(interpolatedNormal, 0)).xyz;

    outp.texCoord   = inp.texCoord;
    outp.flipFace   = dynamicState.invertXAxis;
}


// MORPH-TARGET MESH PIXEL SHADER

Texture2D frontPageColorMap : register(t6);
SamplerState frontPageSampler : register(s7);

Texture2D backPageColorMap : register(t8);
SamplerState backPageSampler : register(s9);

float4 PMorphTargetMesh(VertexOut inp, bool isFrontFace : SV_IsFrontFace) : SV_Target
{
    float2 texCoord =
    (
        isFrontFace
            ? TransformTexCoord(inp.texCoord, dynamicState.texCoordScaleFront)
            : TransformTexCoord(float2(1.0 - inp.texCoord.x, inp.texCoord.y), dynamicState.texCoordScaleBack)
    );

    // Sample base color map and apply alpha mask
    float4 color =
    (
        isFrontFace
            ? SampleColorMap(frontPageColorMap, frontPageSampler, texCoord, dynamicState.borderSamplerFront)
            : SampleColorMap(backPageColorMap, backPageSampler, texCoord, dynamicState.borderSamplerBack)
    );
    float alpha = 1.0;//alphaMask.Sample(alphaMaskSampler, texCoord);

    // Sample detail map to simulate paper surface
    float3 paperDetail = SampleDetailMap(texCoord);

    // Apply lambert factor for simple shading
    float flipFace = inp.flipFace * lerp(-1.0, +1.0, isFrontFace);

    return FinalShading(color, inp.normal * flipFace, alpha, paperDetail);
}


