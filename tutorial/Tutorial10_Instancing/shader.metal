
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

typedef struct
{
    float4x4    vpMatrix;
    float2      animationVector;
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
    float3      color;
    float       arrayLayer;
    float4      wMatrix1;
    float4      wMatrix2;
    float4      wMatrix3;
    float4      wMatrix4;
    //float4x4    wMatrix;
}
InstanceIn;

typedef struct
{
    float4  position [[position]];
    float3  color;
    float2  texCoord;
    uint    arrayLayer;
}
VertexOut;

vertex VertexOut VS(
    VertexIn             vert     [[stage_in]],
    //constant InstanceIn* inst     [[buffer(1)]],
    constant Settings&   settings [[buffer(3)]],
    uint                 instID   [[instance_id]])
{
    VertexOut outp;
    
    float2 offset = settings.animationVector * vert.position.y;
    
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
    
    //outp.position   = settings.vpMatrix * /*inst[instID].wMatrix * */coord;
    outp.position   = settings.vpMatrix * wMatrix * coord;
    outp.texCoord   = vert.texCoord;
    outp.arrayLayer = static_cast<uint>(vert.arrayLayer);
    outp.color      = vert.color;

    return outp;
}

fragment float4 PS(
    VertexOut               inp [[stage_in]],
    texture2d_array<float>  tex [[texture(1)]],
    sampler                 smpl [[sampler(2)]])
{
    // Sample texture color
    float4 color = tex.sample(smpl, inp.texCoord, inp.arrayLayer);
    
    if (color.a < 0.5)
        discard_fragment();
    
    //color.rgb *= inp.color;
    
    return color;
}
