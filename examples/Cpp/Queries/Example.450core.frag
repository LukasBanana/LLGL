// GLSL model fragment shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4 wvpMatrix;
    mat4 wMatrix;
    vec4 color;
    vec3 lightDir;
};

layout(location = 0) in vec3 vNormal;

layout(location = 0) out vec4 fragColor;

void main()
{
    float NdotL = dot(lightDir, normalize(vNormal));
    float intensity = max(0.2, NdotL);
    fragColor = color * vec4(vec3(intensity), 1);
}
