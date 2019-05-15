// GLSL scene fragment shader

#version 450 core

layout(std140, binding = 1) uniform SceneSettings
{
	mat4	wvpMatrix;
	mat4	wMatrix;
	vec4	diffuse;
	vec4	glossiness;
	float	intensity;
};

layout(location = 0) in vec3 vNormal;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outGloss;

void main()
{
	// Write simple lighting into 1st render target
	vec3 lightDir = vec3(0, 0, -1);
	vec3 normal = normalize(vNormal);
	
	float NdotL = max(0.4, dot(lightDir, normal));
	outColor = diffuse * vec4(vec3(NdotL), 1);
	
	// Write glossiness into 2nd render target
	outGloss = glossiness;
}
