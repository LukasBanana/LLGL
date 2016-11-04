// GLSL final fragment shader

#version 420

layout(std140) uniform Settings
{
	mat4	wvpMatrix;
	mat4	wMatrix;
	vec4	diffuse;
	vec4	glossiness;
	float	intensity;
};

layout(binding=0) uniform sampler2D colorMap;
layout(binding=1) uniform sampler2D glossMap;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
	// Show final result with color and gloss map
	fragColor =
		texture(colorMap, vTexCoord) +
		texture(glossMap, vTexCoord) * intensity;
}
