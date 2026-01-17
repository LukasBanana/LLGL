
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;


// VERTEX SHADER

struct Scene
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float3   lightVec;
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
    float3 normal;
    float2 texCoord;
};

vertex VertexOut VS(
    VertexIn        inp   [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
    VertexOut outp;
    outp.position = scene.wvpMatrix * float4(inp.position, 1);
    outp.normal   = normalize((scene.wMatrix * float4(inp.normal, 0)).xyz);
    outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

fragment float4 PS(
    VertexOut           inp          [[stage_in]],
    constant Scene&     scene        [[buffer(1)]],
    texture2d<float>    colorMap     [[texture(2)]],
    sampler             samplerState [[sampler(3)]])
{
    float4 color = colorMap.sample(samplerState, inp.texCoord);

    // Apply lambert factor for simple shading
    float NdotL = dot(scene.lightVec, normalize(inp.normal));
    color.rgb *= mix(0.2, 1.0, NdotL);

    return color;
}
