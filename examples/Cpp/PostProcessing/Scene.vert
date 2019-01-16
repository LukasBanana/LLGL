// GLSL scene vertex shader

#version 140

layout(std140) uniform SceneSettings
{
    mat4    wvpMatrix;
    mat4    wMatrix;
    vec4    diffuse;
    vec4    glossiness;
    float   intensity;
};

in vec3 position;
in vec3 normal;

out vec3 vNormal;

void main()
{
    gl_Position = wvpMatrix * vec4(position, 1);
    vNormal = (wMatrix * vec4(normal, 0)).xyz;
}
