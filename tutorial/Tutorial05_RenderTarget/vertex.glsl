// GLSL model vertex shader

#version 140

layout(std140) uniform Settings
{
	mat4 wvpMatrix;
};

in vec3 position;

out vec2 vTexCoord;

void main()
{
	gl_Position = wvpMatrix * vec4(position, 1);
	vTexCoord = position.xy;
}
