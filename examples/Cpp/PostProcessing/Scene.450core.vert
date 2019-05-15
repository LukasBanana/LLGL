// GLSL scene vertex shader

#version 450 core

layout(std140, binding = 1) uniform SceneSettings
{
	mat4	wvpMatrix;
	mat4	wMatrix;
	vec4	diffuse;
	vec4	glossiness;
	float	intensity;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

layout(location = 0) out vec3 vNormal;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
	vNormal = (wMatrix * vec4(normal, 0)).xyz;
}
