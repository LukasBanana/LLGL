// HLSL texturing shader

cbuffer Scene : register(b1)
{
    float4x4    projection;
    float2      glyphAtlasInvSize;
}

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
    outp.position = mul(projection, float4(x, y, 0, 1));
    outp.texCoord = glyphAtlasInvSize * float2(u, v);
    outp.color    = inp.color;
}


// PIXEL SHADER

Texture2D glyphTexture : register(t0);
SamplerState linearSampler : register(s0);

float4 PS(OutputVS inp) : SV_Target
{
    return float4(inp.color.rgb, inp.color.a * glyphTexture.Sample(linearSampler, inp.texCoord).a);
}



