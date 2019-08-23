// HLSL shader version 4.0 (for Direct3D 11/ 12)

struct InputVS
{
	float2 position : POSITION;
	float3 color : COLOR;
};

struct OutputVS
{
	float4 position : SV_Position;
	float3 color : COLOR;
};

struct OutputGS
{
	float4 position : SV_Position;
	float3 color : COLOR;
	uint viewport : SV_ViewportArrayIndex;
};

// Vertex shader main function
OutputVS VS(InputVS inp)
{
	OutputVS outp;
	outp.position = float4(inp.position, 0, 1);
	outp.color = inp.color;
	return outp;
}

// Geometry shader main function
[maxvertexcount(6)]
void GS(triangle OutputVS inp[3], inout TriangleStream<OutputGS> streamOut)
{
	OutputGS outp;
	int i;

	// Write vertices to 1st viewport
	outp.viewport = 0;

	for (i = 0; i < 3; ++i)
	{
		outp.position = inp[i].position;
		outp.color = inp[i].color;
		streamOut.Append(outp);
	}

	streamOut.RestartStrip();

	// Write vertices to 2nd viewport
	outp.viewport = 1;

	for (i = 0; i < 3; ++i)
	{
		outp.position = inp[i].position;
		outp.position.xy = -outp.position.xy;
		outp.color = inp[i].color;
		streamOut.Append(outp);
	}

	streamOut.RestartStrip();
}

// Pixel shader main function
float4 PS(OutputGS inp) : SV_Target
{
	return float4(inp.color, 1);
};

