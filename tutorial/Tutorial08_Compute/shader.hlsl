// HLSL compute shader

struct DataBlock
{
	float3 position;
	float3 color;
};

RWStructuredBuffer<DataBlock> container : register(u0);

// Compute shader main function
[numthreads(1, 1, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
	DataBlock data = container[threadID.x];
	
	data.position = float3(3, 2, 1);
	data.color *= 3.0;
	
	container[threadID.x] = data;
}

