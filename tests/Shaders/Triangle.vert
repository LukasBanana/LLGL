#version 450

layout(location = 0) in vec2 coord;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in vec3 color;

layout(location = 0) out vec4 vColor;
layout(location = 1) out vec2 vTexCoord;

layout(binding = 2) uniform Matrices
{
	mat4 projection;
	mat4 modelView;
};

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = projection * modelView * vec4(coord, 0, 1);
	vTexCoord = texCoord;
	vColor = vec4(color, 1);
}
