// HLSL font shader

struct Scene_t
{
    float4x4    projection;
    float2      glyphAtlasInvSize;
};

#if __spirv__
[[vk::push_constant]] Scene_t scene;
#else
cbuffer Scene : register(b1)
{
    Scene_t scene;
}
#endif

struct InputVS
{
    int2   position : POSITION;
    int2   texCoord : TEXCOORD;
    float4 color    : COLOR;
};

struct OutputVS
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
    float4 color    : COLOR;
};


// VERTEX SHADER

void VS(InputVS inp, out OutputVS outp)
{
    // Decompress vertex attributes
    float x = (float)inp.position.x;
    float y = (float)inp.position.y;
    float u = (float)inp.texCoord.x;
    float v = (float)inp.texCoord.y;

    // Write vertex output attributes
    outp.position = mul(scene.projection, float4(x, y, 0, 1));
    outp.texCoord = scene.glyphAtlasInvSize * float2(u, v);
    outp.color    = inp.color;
}


// PIXEL SHADER

Texture2D glyphTexture : register(t0);
SamplerState linearSampler : register(s2);

float4 PS(OutputVS inp) : SV_Target
{
    return float4(inp.color.rgb, inp.color.a * glyphTexture.Sample(linearSampler, inp.texCoord).a);
}



