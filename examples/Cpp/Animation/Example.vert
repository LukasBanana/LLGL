// GLSL vertex shader

#version 140

layout(std140) uniform Settings
{
    mat4    wMatrix;
    mat4    vpMatrix;
    vec3    lightDir;
    float   shininess;
    vec4    viewPos;
    vec4    albedo;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

out vec4 vWorldPos;
out vec4 vNormal;
out vec2 vTexCoord;

void main()
{
    vWorldPos   = wMatrix * vec4(position, 1);
    gl_Position = vpMatrix * vWorldPos;
    vNormal     = wMatrix * vec4(normal, 0);
    vTexCoord   = texCoord;
}
