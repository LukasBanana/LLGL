// GLSL model vertex shader

#version 450

layout(std140, binding = 3) uniform Settings
{
	mat4 wvpMatrix;
	mat4 wMatrix;
	int useTexture2DMS;
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
