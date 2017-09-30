#version 450

layout(location = 0) in vec2 coord;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 vColor;

layout(std140, binding = 0) uniform Matrices
{
	mat4 projection;
};

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = projection * vec4(coord, 0, 1);
	vColor = vec4(color, 1);
}
