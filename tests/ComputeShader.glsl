// ComputeShader.glsl
// OpenGL Test Shader for LLGL
// 09/09/2016

#version 430

#define VEC_SIZE 128

layout(std430) buffer OutputBuffer
{
	vec4 vec[];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

void main()
{
	uint id = gl_LocalInvocationID.x;
	uint x = id*2;
	
	int size = VEC_SIZE;
	int offset = 1;
	
	while (offset*2 <= size)
	{
		if (id % offset == 0)
		{
			// Read sum or average from vector and write the result back
			//vec[x] = (vec[x] + vec[x + offset]);
			vec[x] = (vec[x]*0.5 + vec[x + offset]*0.5);
		}
		offset *= 2;
		memoryBarrierBuffer();
	}
}


