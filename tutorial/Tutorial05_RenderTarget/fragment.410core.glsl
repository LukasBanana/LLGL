// GLSL model fragment shader

#version 410

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	int useTexture2DMS;
};

uniform sampler2D colorMap;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
    // Sample texel from standard texture
    fragColor = texture(colorMap, vTexCoord);
}
