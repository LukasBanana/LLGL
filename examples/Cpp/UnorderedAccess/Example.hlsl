// HLSL unordered acces shader


struct InputVS
{
	float2 position : POSITION;
	float2 texCoord : TEXCOORD;
};

struct OutputVS
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
};


// VERTEX SHADER

OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = float4(inp.position, 0, 1);
	outp.texCoord = inp.texCoord;
	return outp;
}


// PIXEL SHADER

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

float4 PS(OutputVS inp) : SV_Target
{
	return tex.Sample(texSampler, inp.texCoord);
}


// COMPUTE SHADER

RWTexture2D<float4> texOut : register(u1);

[numthreads(1, 1, 1)]
void CS(uint3 id : SV_DispatchThreadID)
{
    float2 tc = ((float2)id.xy) * 0.1;
    texOut[id.xy] = tex.Load(id) * (sin(tc.x)*0.2 + 1.0);
}



