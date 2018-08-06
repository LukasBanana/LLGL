// GLSL shadow-map vertex shader

#version 140

layout(std140) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    mat4 vpShadowMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

in vec3 position;

void main()
{
	gl_Position = vpShadowMatrix * (wMatrix * vec4(position, 1));
}
