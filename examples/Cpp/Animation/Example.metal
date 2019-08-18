// Metal model shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Settings
{
    float4x4    wMatrix;
    float4x4    vpMatrix;
    float4      lightDirAndShininess;
    float4      viewPos;
    float4      albedo;
};


// VERTEX SHADER SCENE

struct VIn
{
	float3 position [[attribute(0)]];
	float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VOut
{
	float4 position [[position]];
    float4 worldPos;
	float4 normal;
    float2 texCoord;
};

vertex VOut VS(VIn inp [[stage_in]], constant Settings& settings [[buffer(1)]])
{
    VOut outp;
    outp.worldPos   = settings.wMatrix * float4(inp.position, 1);
	outp.position	= settings.vpMatrix * outp.worldPos;
	outp.normal		= settings.wMatrix * float4(inp.normal, 0);
    outp.texCoord   = inp.texCoord;
    return outp;
}


// PIXEL SHADER SCENE

fragment float4 PS(
    VOut                inp             [[stage_in]],
    constant Settings&  settings        [[buffer(1)]],
    texture2d<float>    colorMap        [[texture(2)]],
    sampler             linearSampler   [[sampler(3)]])
{
    // Diffuse lighting
    float3  lightVec    = -settings.lightDirAndShininess.xyz;
    float3  normal      = normalize(inp.normal.xyz);
    float   NdotL       = mix(0.2, 1.0, max(0.0, dot(normal, lightVec)));
    float3  diffuse     = settings.albedo.rgb * NdotL;

    // Specular lighting
    float3  viewDir     = normalize(settings.viewPos.xyz - inp.worldPos.xyz);
    float3  halfVec     = normalize(viewDir + lightVec);
    float   NdotH       = dot(normal, halfVec);
    float3  specular    = (float3)pow(max(0.0, NdotH), settings.lightDirAndShininess.a);

    // Sample texture
    float4 color = colorMap.sample(linearSampler, inp.texCoord);

    // Set final output color
    return mix((float4)1, color, settings.albedo.a) * float4(diffuse + specular, 1.0);
}



