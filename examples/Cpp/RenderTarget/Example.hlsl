// HLSL model shader


// VERTEX SHADER

cbuffer Settings : register(b3)
{
    float4x4 wvpMatrix;
    int useTexture2DMS;
};

struct InputVS
{
    float3 position : POSITION;
    float2 texCoord : TEXCOORD;
};

struct OutputVS
{
    float4 position : SV_Position;
    float2 texCoord : TEXCOORD;
};

OutputVS VS(InputVS inp)
{
    OutputVS outp;
    outp.position = mul(wvpMatrix, float4(inp.position, 1));
    outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

Texture2D colorMap : register(t2);

#ifdef ENABLE_TEXTURE2DMS
Texture2DMS<float4, 8> colorMapMS : register(t3);
#endif

SamplerState samplerState : register(s1);

float4 PS(OutputVS inp) : SV_Target
{
    #ifdef ENABLE_TEXTURE2DMS
    if (useTexture2DMS)
    {
        // Load texel from multi-sample texture
        uint w, h, numSamples;
        colorMapMS.GetDimensions(w, h, numSamples);

        int2 tc = int2(
            (int)(inp.texCoord.x * (float)w),
            (int)(inp.texCoord.y * (float)h)
        );

        // Compute average of all samples
        float4 c = (float4)0.0;

        for (uint i = 0; i < numSamples; ++i)
        c += colorMapMS.Load(tc, i);

        c /= numSamples;

        return c;
    }
    else
    #endif
    {
        // Sample texel from standard texture
        return colorMap.Sample(samplerState, inp.texCoord);
    }
}



