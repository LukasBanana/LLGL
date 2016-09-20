// HLSL tessellation shader


// Uniform buffer object (also named "Constant Buffer")
cbuffer Settings : register(b0)
{
	float4x4 wvpMatrix;
	float tessLevelInner;
	float tessLevelOuter;
	float twist;
	float _pad0;
};

struct OutputVS
{
	float3 position : WORLDPOS;
};

struct OutputHS
{
	float edges[4] : SV_TessFactor;
	float inner[2] : SV_InsideTessFactor;
};

struct OutputDS
{
	float4 position : SV_Position;
	float3 color : COLOR;
};


// VERTEX SHADER

OutputVS VS(float3 position : POSITION)
{
	OutputVS outp;
	outp.position = position;
	return outp;
}


// HULL SHADER

OutputHS PatchConstantFuncHS(InputPatch<OutputVS, 4> inp)
{
	OutputHS outp;
	
	#if 0
	outp.edges[0] = tessLevelOuter;
	outp.edges[1] = tessLevelOuter;
	outp.edges[2] = tessLevelOuter;
	outp.edges[3] = tessLevelOuter;
	
	outp.inner[0] = tessLevelInner;
	outp.inner[1] = tessLevelInner;
	#else
	outp.edges[0] = 5.0;
	outp.edges[1] = 5.0;
	outp.edges[2] = 5.0;
	outp.edges[3] = 5.0;
	
	outp.inner[0] = 5.0;
	outp.inner[1] = 5.0;
	#endif
	
	return outp;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFuncHS")]
[maxtessfactor(65.0)]//(64.0)]
OutputVS HS(InputPatch<OutputVS, 4> inp, uint id : SV_OutputControlPointID)
{
	OutputVS outp;
	outp.position = inp[id].position;
	return outp;
}


// DOMAIN SHADER

[domain("quad")]
OutputDS DS(OutputHS inp, float2 tessCoord : SV_DomainLocation, const OutputPatch<OutputVS, 4> patch)
{
	OutputDS outp;
	
	// Interpolate position
	float u = tessCoord.x;
	float v = tessCoord.y;
	
	float3 a = lerp(patch[0].position, patch[1].position, u);
	float3 b = lerp(patch[2].position, patch[3].position, u);
	
	float3 position = lerp(a, b, v);
	
	// Set final vertex color
	outp.color = (1.0 - position) * 0.5;
	
	// Apply twist rotation matrix (rotate around Y axis)
	#if 0
	float twistFactor = (position.y + 1.0) * 0.5;
	
	float s = sin(twist * twistFactor);
	float c = cos(twist * twistFactor);
	
	float3x3 rotation = float3x3(
		 c, 0, s,
		 0, 1, 0,
		-s, 0, c
	);
	
	position = mul(rotation, position);
	#endif
	
	// Transform vertex by the world-view-projection matrix chain
	#if 0
	outp.position = mul(wvpMatrix, float4(position, 1));
	#else
	
	float4x4 m = float4x4(
		1.81066000, 0, 0, 0,
		0, 2.41421342, 0, 0,
		0, 0, 1.00100100, 1,
		0, 0, -0.100100100, 0
	);
	outp.position = mul(m, float4(position, 1));
	#endif
	
	return outp;
}


// PIXEL SHADER

float4 PS(OutputDS inp) : SV_Target
{
	return float4(inp.color, 1);
}



