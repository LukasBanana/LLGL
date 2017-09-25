#version 450

layout(location = 0) in vec2 coord;
layout(location = 1) in vec3 color;

layout(location = 0) out vec4 vColor;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = vec4(coord, 0, 1);
	vColor = vec4(color, 1);
}
