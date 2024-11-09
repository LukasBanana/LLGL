// HLSL shader for HelloGame example
// This file is part of the LLGL project

cbuffer Scene : register(b1)
{
    float4x4    vpMatrix;
    float4x4    vpShadowMatrix;
    float3      lightDir;
    float       shininess;
    float3      viewPos;
    float       shadowSizeInv;
    float3      warpCenter;
    float       warpIntensity;
    float3      bendDir;
    float       ambientItensity;
    float3      groundTint;
    float       groundScale;
    float3      lightColor;
    float       warpScaleInv;
};

struct Instance
{
    float3x4    wMatrix;
    float4      color;
};


// VERTEX SHADER INSTANCE

StructuredBuffer<Instance> instances : register(t2);

float3  worldOffset;
float   bendIntensity;
uint    firstInstance;

struct VertexIn
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VertexOut
{
    float4 position : SV_Position;
    float3 worldPos : WORLDPOS;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
    float4 color    : COLOR;
};

float WarpIntensityCurve(float d)
{
    return 1.0/(1.0 + d*d*warpScaleInv);
}

float3 MeshAnimation(float3 pos)
{
    return pos + bendDir * pos.y * bendIntensity;
}

float3 WarpPosition(float3 pos)
{
    float3 dir = pos - warpCenter;
    float dirLen = length(dir);
    float intensity = WarpIntensityCurve(dirLen);
    return pos + normalize(dir) * intensity * warpIntensity;
}

void VSInstance(VertexIn inp, uint instID : SV_InstanceID, out VertexOut outp)
{
    Instance inst = instances[instID + firstInstance];
    float4 modelPos = float4(MeshAnimation(inp.position), 1);
    outp.worldPos   = WarpPosition(mul(inst.wMatrix, modelPos) + worldOffset);
    outp.position   = mul(vpMatrix, float4(outp.worldPos, 1));
    outp.normal     = mul(inst.wMatrix, float4(inp.normal, 0));
    outp.texCoord   = inp.texCoord;
    outp.color      = inst.color;
}


// PIXEL SHADER INSTANCE

Texture2D shadowMap : register(t4);
SamplerComparisonState shadowMapSampler : register(s4);

float SampleShadowMapOffset(float3 worldPos, float2 offset)
{
    // Project world position into shadow-map space
    float4 shadowPos = mul(vpShadowMatrix, float4(worldPos, 1));
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;
    
    // Sample shadow map
    return shadowMap.SampleCmp(shadowMapSampler, shadowPos.xy + offset * shadowSizeInv, shadowPos.z);
}

float SampleShadowMapPCF(float2 screenPos, float3 worldPos)
{
    // Perform percentage closer filtering (PCF) as described here:
    // https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
    float2 offset = (float2)(frac(screenPos * 0.5) > 0.25);
    if (offset.y > 1.1)
        offset.y = 0.0;

    float shadowSamples =
    (
        SampleShadowMapOffset(worldPos, offset + float2(-1.5, +0.5)) +
        SampleShadowMapOffset(worldPos, offset + float2(+0.5, +0.5)) +
        SampleShadowMapOffset(worldPos, offset + float2(-1.5, -1.5)) +
        SampleShadowMapOffset(worldPos, offset + float2(+0.5, -1.5))
    );

    return shadowSamples * 0.25;
}

float4 PSInstance(VertexOut inp) : SV_Target
{
    // Get input color
    float4  albedo      = inp.color;

    // Diffuse lighting
    float3  lightVec    = -lightDir.xyz;
    float3  normal      = normalize(inp.normal);
    float   NdotL       = lerp(0.2, 1.0, max(0.0, dot(normal, lightVec)));
    float3  diffuse     = albedo.rgb * NdotL;

    // Specular lighting
    float3  viewDir     = normalize(viewPos - inp.worldPos);
    float3  halfVec     = normalize(viewDir + lightVec);
    float   NdotH       = dot(normal, halfVec);
    float3  specular    = (float3)pow(max(0.0, NdotH), shininess);

    // Apply shadow mapping
    float   shadow      = lerp(ambientItensity, 1.0, SampleShadowMapPCF(inp.position.xy, inp.worldPos));

    // Set final output color
    float3  light       = lightColor * (diffuse + specular) * shadow;
    return float4(light, albedo.a);
}


// VERTEX SHADER GROUND

struct GroundVertexOut
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
    float3 worldPos : WORLDPOS;
};

void VSGround(VertexIn inp, out GroundVertexOut outp)
{
    outp.position = mul(vpMatrix, float4(inp.position, 1));
    outp.texCoord = inp.texCoord;
    outp.worldPos = inp.position;
}


// PIXEL SHADER GROUND

Texture2D colorMap : register(t2);
SamplerState colorMapSampler : register(s2);

float4 PSGround(GroundVertexOut inp) : SV_Target
{
    // Sample color map
    float4  albedo = colorMap.Sample(colorMapSampler, inp.texCoord * groundScale) * float4(groundTint, 1);

    // Apply shadow mapping
    float   shadow = lerp(ambientItensity, 1.0, SampleShadowMapPCF(inp.position.xy, inp.worldPos));

    // Set final output color
    return float4(albedo.rgb * lightColor * shadow, albedo.a);
}

