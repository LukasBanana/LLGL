// GLSL model fragment shader

#version 140

layout(std140) uniform Settings
{
    mat4 wvpMatrix;
    mat4 wMatrix;
    vec4 color;
};

in vec3 vNormal;

out vec4 fragColor;

void main()
{
    vec3 lightDir = vec3(0, 0, -1);
    float NdotL = dot(lightDir, normalize(vNormal));
    float intensity = max(0.2, NdotL);
    fragColor = color * vec4(vec3(intensity), 1);
}
