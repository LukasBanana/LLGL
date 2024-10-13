// HLSL shader for HelloGame example
// This file is part of the LLGL project

cbuffer Scene : register(b1)
{
    float4x4    vpMatrix;
    float3      lightDir;
    float       shininess;
    float4      viewPos;
    float3      warpCenter;
    float       warpIntensity;
};

struct Instance
{
    float3x4    wMatrix;
    float4      color;
};


// VERTEX SHADER SCENE

StructuredBuffer<Instance> instances : register(t2);

float3 worldOffset;
uint firstInstance;

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
    return 1.0/(1.0 + d*d);
}

float3 WarpPosition(float3 pos)
{
    float3 dir = pos - warpCenter;
    float dirLen = length(dir);
    float intensity = WarpIntensityCurve(dirLen);
    return pos + normalize(dir) * intensity * warpIntensity;
}

void VSMain(VertexIn inp, uint instID : SV_InstanceID, out VertexOut outp)
{
    Instance inst = instances[instID + firstInstance];
    outp.worldPos   = WarpPosition(mul(inst.wMatrix, float4(inp.position + worldOffset, 1)));
    outp.position   = mul(vpMatrix, float4(outp.worldPos, 1));
    outp.normal     = mul(inst.wMatrix, float4(inp.normal, 0));
    outp.texCoord   = inp.texCoord;
    outp.color      = inst.color;
}


// PIXEL SHADER SCENE

float4 PSMain(VertexOut inp) : SV_Target
{
    // Sample texture
    float4  albedo      = inp.color;

    // Diffuse lighting
    float3  lightVec    = -lightDir.xyz;
    float3  normal      = normalize(inp.normal);
    float   NdotL       = lerp(0.2, 1.0, max(0.0, dot(normal, lightVec)));
    float3  diffuse     = albedo.rgb * NdotL;

    // Specular lighting
    float3  viewDir     = normalize(viewPos.xyz - inp.worldPos);
    float3  halfVec     = normalize(viewDir + lightVec);
    float   NdotH       = dot(normal, halfVec);
    float3  specular    = (float3)pow(max(0.0, NdotH), shininess);

    // Set final output color
    return float4(diffuse + specular, albedo.a);
}



