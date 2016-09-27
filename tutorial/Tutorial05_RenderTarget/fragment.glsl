// GLSL model fragment shader

#version 140

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
};

uniform sampler2D colorMap;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
	fragColor = texture(colorMap, vTexCoord);
}
