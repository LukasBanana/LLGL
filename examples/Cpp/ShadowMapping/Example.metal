// Metal shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct Settings
{
    float4x4 wMatrix;
    float4x4 vpMatrix;
    float4x4 vpShadowMatrix;
    float4 lightDir;
    float4 diffuse;
}
Settings;


// VERTEX SHADER SHADOW-MAP

typedef struct VShaderMapIn
{
    float3 position [[attribute(0)]];
}
VShaderMapIn;

typedef struct VShadowMapOut
{
    float4 position [[position]];
}
VShadowMapOut;

vertex VShadowMapOut VShadowMap(
    VShaderMapIn       inp      [[stage_in]],
    constant Settings& settings [[buffer(1)]])
{
    VShadowMapOut outp;
    outp.position = settings.vpShadowMatrix * (settings.wMatrix * float4(inp.position, 1));
    return outp;
}


// VERTEX SHADER SCENE

typedef struct VSceneIn
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
}
VSceneIn;

typedef struct VSceneOut
{
    float4 position [[position]];
    float4 worldPos;
    float4 normal;
}
VSceneOut;

vertex VSceneOut VScene(
    VSceneIn           inp      [[stage_in]],
    constant Settings& settings [[buffer(1)]])
{
    VSceneOut outp;
    outp.worldPos = settings.wMatrix * float4(inp.position, 1);
    outp.position = settings.vpMatrix * outp.worldPos;
    outp.normal   = settings.wMatrix * float4(inp.normal, 0);
    return outp;
}


// PIXEL SHADER SCENE

fragment float4 PScene(
    VSceneOut          inp              [[stage_in]],
    constant Settings& settings         [[buffer(1)]],
    depth2d<float>     shadowMap        [[texture(2)]],
    sampler            shadowMapSampler [[sampler(3)]])
{
    // Project world position into shadow-map space
    float4 shadowPos = settings.vpShadowMatrix * inp.worldPos;
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * float2(0.5, -0.5) + 0.5;
    
    // Sample shadow map
    float shadow = shadowMap.sample_compare(shadowMapSampler, shadowPos.xy, shadowPos.z);

    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    float NdotL = max(0.2, dot(normal, -settings.lightDir.xyz));
    
    // Set final output color
    shadow = mix(0.2, 1.0, shadow);
    return float4(settings.diffuse.rgb * (NdotL * shadow), 1.0);
}



