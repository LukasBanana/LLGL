// Metal shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;


// VERTEX SHADER

typedef struct
{
    float4x4    wvpMatrix;
    int         useTexture2DMS;
}
Settings;

typedef struct
{
    float3 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
}
VertexIn;

typedef struct
{
    float4 position [[position]];
    float2 texCoord;
}
VertexOut;

vertex VertexOut VS(
    VertexIn inp                [[stage_in]],
    constant Settings& settings [[buffer(3)]])
{
    VertexOut outp;
    outp.position = settings.wvpMatrix * float4(inp.position, 1);
    outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

fragment float4 PS(
    VertexOut           inp         [[stage_in]],
    texture2d<float>    colorMap    [[texture(2)]],
    sampler             smpl        [[sampler(1)]])
{
    // Sample texel from standard texture
    return colorMap.sample(smpl, inp.texCoord);
}



