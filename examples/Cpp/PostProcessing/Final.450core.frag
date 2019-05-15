// GLSL final fragment shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
	mat4	wvpMatrix;
	mat4	wMatrix;
	vec4	diffuse;
	vec4	glossiness;
	float	intensity;
};

layout(binding = 3) uniform texture2D colorMap;
layout(binding = 4) uniform texture2D glossMap;

layout(binding = 5) uniform sampler colorMapSampler;
layout(binding = 6) uniform sampler glossMapSampler;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
	// Show final result with color and gloss map
	fragColor =
		texture(sampler2D(colorMap, colorMapSampler), vTexCoord) +
		texture(sampler2D(glossMap, glossMapSampler), vTexCoord) * intensity;
}
