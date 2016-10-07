// GLSL compute shader
#version 430

struct DataBlock
{
	vec4 position;
	vec4 color;
};

layout(std430) buffer OutputBuffer
{
	DataBlock container[];
};

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

// Compute shader main function
void main()
{
	uvec3 threadID = gl_GlobalInvocationID;
	
	DataBlock data = container[threadID.x];
	
	data.position *= 2.0;
	data.color *= 3.0;
	
	container[threadID.x] = data;
}


