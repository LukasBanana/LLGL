// HLSL shader version 4.0 (for Direct3D 11/ 12)

struct InputVS
{
	float2 position : POSITION;
	float3 color : COLOR;
};

struct OutputVS
{
	float4 position : POSITION;
	float3 color : COLOR;
};

// Vertex shader main function
OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = float4(inp.position, 0, 1);
	outp.color = inp.color;
	return outp;
}

// Pixel shader main function
float4 PS(OutputVS inp) : COLOR0
{
	return float4(inp.color, 1);
};

