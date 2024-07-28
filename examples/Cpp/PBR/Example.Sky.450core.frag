// PBR GLSL Skybox Fragment Shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4    cMatrix;
    mat4    vpMatrix;
    mat4    wMatrix;
    vec2    aspectRatio;
    float   mipCount;
    float   _pad0;
    vec4    lightDir;
    uint    skyboxLayer;
    uint    materialLayer;
    uvec2   _pad1;
};

// SKYBOX SHADER

layout(location = 0) in VSkyOut
{
    vec4 viewRay;
}
inp;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler smpl;
layout(binding = 3) uniform textureCubeArray skyBox;

void main()
{
    vec3 texCoord = normalize(cMatrix * inp.viewRay).xyz;
    outColor = texture(samplerCubeArray(skyBox, smpl), vec4(texCoord, float(skyboxLayer)));
}


