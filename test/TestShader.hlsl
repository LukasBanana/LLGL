// TestShader.hlsl
// D3D12 Shader for LLGL
// 02/09/2016

/*cbuffer Settings : register(b0)
{
	float4x4 wvpMatrix;
};*/

//RWBuffer<float4> outputBuffer;

struct VertexIn
{
	float2 position : POSITION;
	float3 color : COLOR;
};

struct VertexOut
{
	float4 position : SV_Position;
	float4 color : COLOR;
};

VertexOut VS(VertexIn inp, uint id : SV_VertexID)
{
	VertexOut outp;
	
	//outp.position = mul(wvpMatrix, float4(inp.position, 1));
	
	#if 0
	
	outp.position = float4(inp.position, 0, 1);
	outp.color = float4(inp.color, 1);
	
	#else
	
	float2 p = (float2)0;
	float3 c = (float3)0;
	
	switch (id)
	{
		case 0: p = float2( 0,  1); c = float3(1, 0, 0); break;
		case 1: p = float2( 1, -1); c = float3(0, 1, 0); break;
		case 2: p = float2(-1, -1); c = float3(0, 0, 1); break;
	}
	
	outp.position = float4(p, 0, 1);
	//outp.color = float4(c, 1);
	outp.color = float4(inp.color, 1);
	
	#endif
	
	//outputBuffer[0] = outp.position;
	
	return outp;
}

float4 PS(VertexOut inp) : SV_Target
{
	return inp.color;
}


