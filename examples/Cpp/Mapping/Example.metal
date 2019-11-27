// Metal texturing shader

#include <metal_stdlib>

using namespace metal;

struct InputVS
{
	float2 position [[attribute(0)]];
	float2 texCoord [[attribute(1)]];
};

struct OutputVS
{
	float4 position [[position]];
	float2 texCoord;
};


// VERTEX SHADER

vertex OutputVS VS(InputVS inp [[stage_in]])
{
	OutputVS outp;
	outp.position = float4(inp.position, 0, 1);
	outp.texCoord = inp.texCoord;
	return outp;
}


// PIXEL SHADER

fragment float4 PS(
    OutputVS            inp             [[stage_in]],
    texture2d<float>    colorMap        [[texture(0)]],
    sampler             samplerState    [[sampler(1)]])
{
	return colorMap.sample(samplerState, inp.texCoord);
}



