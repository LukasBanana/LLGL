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
	
	outp.edges[0] = tessLevelOuter;
	outp.edges[1] = tessLevelOuter;
	outp.edges[2] = tessLevelOuter;
	outp.edges[3] = tessLevelOuter;
	
	outp.inner[0] = tessLevelInner;
	outp.inner[1] = tessLevelInner;
	
	return outp;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFuncHS")]
[maxtessfactor(64.0)]
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
	float twistFactor = (position.y + 1.0) * 0.5;
	
	float s = sin(twist * twistFactor);
	float c = cos(twist * twistFactor);
	
	float3x3 rotation = float3x3(
		c,  0, -s,
		0,  1,  0,
		s,  0,  c
	);
	
	position = mul(rotation, position);
	
	// Transform vertex by the world-view-projection matrix chain
	outp.position = mul(wvpMatrix, float4(position, 1));
	
	return outp;
}


// PIXEL SHADER

float4 PS(OutputDS inp) : SV_Target
{
	return float4(inp.color, 1);
}



