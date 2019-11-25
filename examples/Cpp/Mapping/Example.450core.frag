// GLSL texturing shader
#version 450 core

layout(binding = 0) uniform texture2D colorMap;
layout(binding = 1) uniform sampler colorMapSampler;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
	fragColor = texture(sampler2D(colorMap, colorMapSampler), vTexCoord);
}
