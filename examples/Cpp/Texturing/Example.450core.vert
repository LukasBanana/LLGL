// GLSL texturing vertex shader

#version 450

layout(std140, binding = 1) uniform Scene
{
	mat4 wvpMatrix;
	mat4 wMatrix;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vTexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
    vNormal = normalize((wMatrix * vec4(normal, 0)).xyz);
	vTexCoord = texCoord;
}
