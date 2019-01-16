
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float4x4    vpMatrix;
    float4      viewPos;
    float4      fogColorAndDensity;
    float2      animVec;
}
Settings;

typedef struct
{
    float3      position    [[attribute(0)]];
    float2      texCoord    [[attribute(1)]];
    float3      color       [[attribute(2)]];
    float       arrayLayer  [[attribute(3)]];
    float4      wMatrix1    [[attribute(4)]];
    float4      wMatrix2    [[attribute(5)]];
    float4      wMatrix3    [[attribute(6)]];
    float4      wMatrix4    [[attribute(7)]];
}
VertexIn;

typedef struct
{
    float4  position [[position]];
    float4  worldPos;
    float3  color;
    float2  texCoord;
    uint    arrayLayer;
}
VertexOut;

vertex VertexOut VS(
    VertexIn             vert     [[stage_in]],
    constant Settings&   settings [[buffer(2)]],
    uint                 instID   [[instance_id]])
{
    VertexOut outp;
    
    float2 offset = settings.animVec * vert.position.y;
    
    float4 coord = float4(
        vert.position.x + offset.x,
        vert.position.y,
        vert.position.z + offset.y,
        1.0
    );

    float4x4 wMatrix = float4x4(
        vert.wMatrix1,
        vert.wMatrix2,
        vert.wMatrix3,
        vert.wMatrix4
    );

    outp.worldPos   = wMatrix * coord;
    outp.position   = settings.vpMatrix * outp.worldPos;
    outp.texCoord   = vert.texCoord;
    outp.arrayLayer = static_cast<uint>(vert.arrayLayer);
    outp.color      = vert.color;

    return outp;
}

fragment float4 PS(
    VertexOut               inp      [[stage_in]],
    constant Settings&      settings [[buffer(2)]],
    texture2d_array<float>  tex      [[texture(3)]],
    sampler                 smpl     [[sampler(4)]])
{
    // Sample albedo texture
    float4 color = tex.sample(smpl, inp.texCoord, inp.arrayLayer);

    // Apply alpha clipping
    if (color.a < 0.5)
        discard_fragment();

    // Compute fog density
    float viewDist = distance(settings.viewPos, inp.worldPos);
    float fog = viewDist*settings.fogColorAndDensity.a;
    fog = 1.0 - 1.0/exp(fog*fog);

    // Interpolate between albedo and fog color
    color.rgb = mix(color.rgb * inp.color, settings.fogColorAndDensity.rgb, fog);
    
    return color;
}
