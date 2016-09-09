// ComputeShader.glsl
// OpenGL Test Shader for LLGL
// 09/09/2016

#version 430

#define VEC_SIZE 128

layout(std430) buffer OutputBuffer
{
	vec4 vec[VEC_SIZE];
};

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;

void main()
{
	uint id = gl_LocalInvocationID.x;
	uint idx = id*2;
	
	int size = VEC_SIZE;
	int offset = 1;
	
	while (offset*2 <= size)
	{
		if (id % offset == 0)
		{
			vec[idx] = (vec[idx] + vec[idx + offset]);
			//vec[idx] = (vec[idx]*0.5 + vec[idx + offset]*0.5);
		}
		offset *= 2;
		memoryBarrierBuffer();
	}
}


