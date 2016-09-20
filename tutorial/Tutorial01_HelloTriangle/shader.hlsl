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

// Vertex shader main function
OutputVS VS(InputVS inp, uint id : SV_VertexID)
{
	OutputVS outp;
	#if 1
	
	float2 p = (float2)0;
	float3 c = (float3)0;
	
	switch (id)
	{
		case 0:
			p = float2(0, 1);
			c = float3(1, 0, 0);
			break;
		case 1:
			p = float2(1, -1);
			c = float3(0, 1, 0);
			break;
		case 2:
			p = float2(-1, -1);
			c = float3(0, 0, 1);
			break;
	}
	
	outp.position = float4(p, 0, 1);
	outp.color = c;
	
	#else
	outp.position = float4(inp.position, 0, 1);
	outp.color = inp.color;
	#endif
	return outp;
}

// Pixel shader main function
float4 PS(OutputVS inp) : SV_Target
{
	return float4(inp.color, 1);
};

