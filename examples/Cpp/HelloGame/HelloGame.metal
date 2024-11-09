// Metal shader for HelloGame example
// This file is part of the LLGL project

#include <metal_stdlib>

using namespace metal;


struct Scene
{
    float4x4        vpMatrix;
    float4x4        vpShadowMatrix;
    packed_float3   lightDir;
    float           shininess;
    packed_float3   viewPos;
    float           shadowSizeInv;
    packed_float3   warpCenter;
    float           warpIntensity;
    packed_float3   bendDir;
    float           ambientItensity;
    packed_float3   groundTint;
    float           groundScale;
    packed_float3   lightColor;
    float           warpScaleInv;
};

struct Instance
{
    float4 wMatrixRow0;
    float4 wMatrixRow1;
    float4 wMatrixRow2;
    float4 color;
};


// VERTEX SHADER INSTANCE

struct Globals
{
    float3  worldOffset;
    float   bendIntensity;
    uint    firstInstance;
};

struct VertexIn
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 worldPos;
    float3 normal;
    float2 texCoord;
    float4 color;
};

float WarpIntensityCurve(float d, float warpScaleInv)
{
    return 1.0/(1.0 + d*d*warpScaleInv);
}

float3 MeshAnimation(float3 pos, float3 bendDir, float bendIntensity)
{
    return pos + bendDir * pos.y * bendIntensity;
}

float3 WarpPosition(constant Scene& scene, float3 pos)
{
    float3 dir = pos - scene.warpCenter;
    float dirLen = length(dir);
    float intensity = WarpIntensityCurve(dirLen, scene.warpScaleInv);
    return pos + normalize(dir) * intensity * scene.warpIntensity;
}

// Unpacks three float4 matrix rows to mimic D3D's compact matrix memory layout
float4x3 UnpackRowMajor3x4Matrix(float4 row0, float4 row1, float4 row2)
{
    return float4x3(
        float3(row0.x, row0.y, row0.z),
        float3(row0.w, row1.x, row1.y),
        float3(row1.z, row1.w, row2.x),
        float3(row2.y, row2.z, row2.w)
    );
}

vertex VertexOut VSInstance(
    VertexIn                inp         [[stage_in]],
    uint                    instID      [[instance_id]],
    constant Scene&         scene       [[buffer(1)]],
    device const Instance*  instances   [[buffer(2)]],
    constant Globals&       globals     [[buffer(5)]])
{
    VertexOut outp;
    Instance inst = instances[instID + globals.firstInstance];
    float4x3 wMatrix = UnpackRowMajor3x4Matrix(inst.wMatrixRow0, inst.wMatrixRow1, inst.wMatrixRow2);
    float4 modelPos = float4(MeshAnimation(inp.position, scene.bendDir, globals.bendIntensity), 1);
    outp.worldPos   = WarpPosition(scene, wMatrix * modelPos + globals.worldOffset);
    outp.position   = scene.vpMatrix * float4(outp.worldPos, 1);
    outp.normal     = wMatrix * float4(inp.normal, 0);
    outp.texCoord   = inp.texCoord;
    outp.color      = inst.color;
    return outp;
}


// PIXEL SHADER INSTANCE

float SampleShadowMapOffset(
    depth2d<float>  shadowMap,
    sampler         shadowMapSampler,
    constant Scene& scene,
    float3          worldPos,
    float2          offset)
{
    // Project world position into shadow-map space
    float4 shadowPos = scene.vpShadowMatrix * float4(worldPos, 1);
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;
    
    // Sample shadow map
    return shadowMap.sample_compare(shadowMapSampler, shadowPos.xy + offset * scene.shadowSizeInv, shadowPos.z);
}

float SampleShadowMapPCF(
    depth2d<float>  shadowMap,
    sampler         shadowMapSampler,
    constant Scene& scene,
    float2          screenPos,
    float3          worldPos)
{
    // Perform percentage closer filtering (PCF) as described here:
    // https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
    float2 offset = (float2)(fract(screenPos * 0.5) > 0.25);
    if (offset.y > 1.1)
        offset.y = 0.0;

    float shadowSamples =
    (
        SampleShadowMapOffset(shadowMap, shadowMapSampler, scene, worldPos, offset + float2(-1.5, +0.5)) +
        SampleShadowMapOffset(shadowMap, shadowMapSampler, scene, worldPos, offset + float2(+0.5, +0.5)) +
        SampleShadowMapOffset(shadowMap, shadowMapSampler, scene, worldPos, offset + float2(-1.5, -1.5)) +
        SampleShadowMapOffset(shadowMap, shadowMapSampler, scene, worldPos, offset + float2(+0.5, -1.5))
    );

    return shadowSamples * 0.25;
}

fragment float4 PSInstance(
    VertexOut           inp                 [[stage_in]],
    constant Scene&     scene               [[buffer(1)]],
    constant Globals&   globals             [[buffer(5)]],
    depth2d<float>      shadowMap           [[texture(4)]],
    sampler             shadowMapSampler    [[sampler(4)]])
{
    // Get input color
    float4  albedo      = inp.color;

    // Diffuse lighting
    float3  lightVec    = -scene.lightDir.xyz;
    float3  normal      = normalize(inp.normal);
    float   NdotL       = mix(0.2, 1.0, max(0.0, dot(normal, lightVec)));
    float3  diffuse     = albedo.rgb * NdotL;

    // Specular lighting
    float3  viewDir     = normalize(scene.viewPos - inp.worldPos);
    float3  halfVec     = normalize(viewDir + lightVec);
    float   NdotH       = dot(normal, halfVec);
    float3  specular    = (float3)pow(max(0.0, NdotH), scene.shininess);

    // Apply shadow mapping
    float   shadow      = mix(scene.ambientItensity, 1.0, SampleShadowMapPCF(shadowMap, shadowMapSampler, scene, inp.position.xy, inp.worldPos));

    // Set final output color
    float3  light       = scene.lightColor * (diffuse + specular) * shadow;
    return float4(light, albedo.a);
}


// VERTEX SHADER GROUND

struct GroundVertexOut
{
    float4 position [[position]];
    float2 texCoord;
    float3 worldPos;
};

vertex GroundVertexOut VSGround(
    VertexIn        inp     [[stage_in]],
    constant Scene& scene   [[buffer(1)]])
{
    GroundVertexOut outp;
    outp.position = scene.vpMatrix * float4(inp.position, 1);
    outp.texCoord = inp.texCoord;
    outp.worldPos = inp.position;
    return outp;
}


// PIXEL SHADER GROUND

fragment float4 PSGround(
    GroundVertexOut     inp                 [[stage_in]],
    constant Scene&     scene               [[buffer(1)]],
    texture2d<float>    colorMap            [[texture(2)]],
    sampler             colorMapSampler     [[sampler(2)]],
    depth2d<float>      shadowMap           [[texture(4)]],
    sampler             shadowMapSampler    [[sampler(4)]])
{
    // Sample color map
    float4  albedo = colorMap.sample(colorMapSampler, inp.texCoord * scene.groundScale) * float4(scene.groundTint, 1);

    // Apply shadow mapping
    float   shadow = mix(scene.ambientItensity, 1.0, SampleShadowMapPCF(shadowMap, shadowMapSampler, scene, inp.position.xy, inp.worldPos));

    // Set final output color
    return float4(albedo.rgb * scene.lightColor * shadow, albedo.a);
}

