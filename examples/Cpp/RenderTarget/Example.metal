// Metal shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;


// VERTEX SHADER

typedef struct
{
    float4x4    wvpMatrix;
    float4x4    wMatrix;
    int         useTexture2DMS;
}
Settings;

typedef struct
{
    float3 position [[attribute(0)]];
    float3 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
}
VertexIn;

typedef struct
{
    float4 position [[position]];
    float3 normal;
    float2 texCoord;
}
VertexOut;

vertex VertexOut VS(
    VertexIn inp                [[stage_in]],
    constant Settings& settings [[buffer(3)]])
{
    VertexOut outp;
    outp.position = settings.wvpMatrix * float4(inp.position, 1);
    outp.normal   = normalize((settings.wMatrix * float4(inp.normal, 0)).xyz);
    outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

fragment float4 PS(
    VertexOut           inp         [[stage_in]],
    texture2d<float>    colorMap    [[texture(2)]],
    sampler             smpl        [[sampler(1)]])
{
    float4 color = colorMap.sample(smpl, inp.texCoord);

    // Apply lambert factor for simple shading
    const float3 lightVec = float3(0, 0, -1);
    float NdotL = dot(lightVec, normalize(inp.normal));
    color.rgb *= mix(0.2, 1.0, NdotL);

    return color;
}



