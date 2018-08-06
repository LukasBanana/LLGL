// GLSL scene vertex shader

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
in vec3 normal;

out vec4 vWorldPos;
out vec4 vNormal;

void main()
{
    vWorldPos = wMatrix * vec4(position, 1);
	gl_Position = vpMatrix * vWorldPos;
    vNormal = normalize(wMatrix * vec4(normal, 0));
}
