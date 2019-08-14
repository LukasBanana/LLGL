// GLSL vertex shader

#version 140

layout(std140) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec4 vNormal;
out vec2 vTexCoord;

void main()
{
    gl_Position = vpMatrix * (wMatrix * vec4(position, 1));
    vNormal     = wMatrix * vec4(normal, 0);
    vTexCoord   = texCoord;
}
