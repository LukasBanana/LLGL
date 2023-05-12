// HLSL model shader


// VERTEX SHADER

cbuffer Settings : register(b3)
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    int useTexture2DMS;
};

struct InputVS
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct OutputVS
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

OutputVS VS(InputVS inp)
{
    OutputVS outp;
    outp.position = mul(wvpMatrix, float4(inp.position, 1));
    outp.normal   = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
    outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

Texture2D colorMap : register(t2);

#ifdef ENABLE_CUSTOM_MULTISAMPLING
Texture2DMS<float4, 8> colorMapMS : register(t3);
#endif

SamplerState samplerState : register(s1);

float4 SampleColorMap(float2 texCoord)
{
    #ifdef ENABLE_CUSTOM_MULTISAMPLING
    if (useTexture2DMS)
    {
        // Load texel from multi-sample texture
        uint w, h, numSamples;
        colorMapMS.GetDimensions(w, h, numSamples);

        int2 tc = int2(
            (int)(texCoord.x * (float)w),
            (int)(texCoord.y * (float)h)
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
        return colorMap.Sample(samplerState, texCoord);
    }
}

float4 PS(OutputVS inp) : SV_Target
{
    float4 color = SampleColorMap(inp.texCoord);

	// Apply lambert factor for simple shading
	const float3 lightVec = float3(0, 0, -1);
	float NdotL = dot(lightVec, normalize(inp.normal));
	color.rgb *= lerp(0.2, 1.0, NdotL);

    return color;
}



