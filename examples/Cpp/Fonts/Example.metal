// Metal font rendering shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexIn
{
    int2   position [[attribute(0)]];
    int2   texCoord [[attribute(1)]];
    float4 color    [[attribute(2)]];
};

struct VertexOut
{
    float4 position [[position]];
    float2 texCoord;
    float4 color;
};

struct Scene
{
    float4x4    projection;
    float2      glyphAtlasInvSize;
};

vertex VertexOut VS(
    VertexIn        inp   [[stage_in]],
    constant Scene& scene [[buffer(1)]])
{
    VertexOut outp;

    // Decompress vertex attributes
    float x = (float)inp.position.x;
    float y = (float)inp.position.y;
    float u = (float)inp.texCoord.x;
    float v = (float)inp.texCoord.y;

    // Write vertex output attributes
    outp.position = scene.projection * float4(x, y, 0, 1);
    outp.texCoord = scene.glyphAtlasInvSize * float2(u, v);
    outp.color    = inp.color;

    return outp;
}

fragment float4 PS(
    VertexOut        inp           [[stage_in]],
    texture2d<float> glyphTexture  [[texture(0)]],
    sampler          linearSampler [[sampler(0)]])
{
    return float4(inp.color.rgb, inp.color.a * glyphTexture.sample(linearSampler, inp.texCoord).a);
}

