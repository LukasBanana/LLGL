// HLSL compute shader

struct DataBlock
{
	float4 position;
	float4 color;
};

RWStructuredBuffer<DataBlock> container : register(u0);

// Compute shader main function
[numthreads(1, 1, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
	DataBlock data = container[threadID.x];
	
	data.position *= 2.0;
	data.color *= 3.0;
	
	container[threadID.x] = data;
}

