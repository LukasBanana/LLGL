// MorphTargets HLSL shader
// Written by L. Hermanns 9/9/2026

// Start with cbuffer slot 3, as Metal will reserve the first three slots for the vertex buffers in VMorphTargetMesh() entry point
cbuffer SceneView : register(b3)
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4   lightVec;
    float4   baseColor;
};


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
    outp.texCoord   = inp.texCoord;
    outp.flipFace   = 1.0; // Default value for static mesh
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

struct AnimationState_t
{
    float interpolationFactor; // Interpolation factor for morph targets in range [0.0, 1.0]
    float invertXAxis;
};

#if __spirv__
[[vk::push_constant]] AnimationState_t animationState;
#else
cbuffer AnimationState : register(b0)
{
    AnimationState_t animationState;
}
#endif

void VMorphTargetMesh(MorphTargetVertexIn inp, out VertexOut outp)
{
    float3 interpolatedPosition = lerp(inp.positionA, inp.positionB, animationState.interpolationFactor);
    interpolatedPosition.x *= animationState.invertXAxis;
    outp.position   = mul(wvpMatrix, float4(interpolatedPosition, 1));

    float3 interpolatedNormal = normalize(lerp(inp.normalA, inp.normalB, animationState.interpolationFactor));
    outp.normal     = mul(wMatrix, float4(interpolatedNormal, 0)).xyz;

    outp.texCoord   = inp.texCoord;
    outp.flipFace   = animationState.invertXAxis;
}


// PIXEL SHADER SCENE

Texture2D colorMap : register(t4);
//Texture2D<float> alphaMask : register(t5);

SamplerState colorMapSampler : register(s6);
//SamplerState alphaMaskSampler : register(s7);

float4 PBlinnPhong(VertexOut inp, bool isFrontFace : SV_IsFrontFace) : SV_Target
{
    // Sample base color map and apply alpha mask
    float2 texCoord = lerp(float2(1.0f - inp.texCoord.x, inp.texCoord.y), inp.texCoord, isFrontFace);
    float4 color = colorMap.Sample(colorMapSampler, texCoord);
    float alpha = 1.0;//alphaMask.Sample(alphaMaskSampler, texCoord);

    // Apply lambert factor for simple shading
    float flipFace = inp.flipFace * lerp(+1.0, -1.0, isFrontFace);
    float NdotL = dot(lightVec.xyz, normalize(inp.normal * flipFace));
    float lighting = lerp(0.2, 1.0, NdotL);

    float3 texColor = lerp(baseColor.rgb, color.rgb, color.a);
    float4 finalColor = baseColor * float4(texColor, alpha);

    finalColor.rgb *= lighting;

    return finalColor;
}



