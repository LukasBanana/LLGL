#version 450

#if 0

layout(location = 0) out vec4 vColor;

void main()
{
	const vec4 coords[] = vec4[](vec4(0, -0.5, 0, 1), vec4(0.5, 0.5, 0, 1), vec4(-0.5, 0.5, 0, 1));
	const vec4 colors[] = vec4[](vec4(1, 0, 0, 1), vec4(0, 1, 0, 1), vec4(0, 0, 1, 1));
	gl_Position = coords[gl_VertexIndex];
	vColor = colors[gl_VertexIndex];
}

#else

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

#endif
