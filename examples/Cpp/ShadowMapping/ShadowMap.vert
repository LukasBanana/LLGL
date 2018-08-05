// GLSL model vertex shader

#version 140

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
	int useTexture2DMS;
};

in vec3 position;
in vec2 texCoord;

out vec2 vTexCoord;

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
	vTexCoord = texCoord;
}
