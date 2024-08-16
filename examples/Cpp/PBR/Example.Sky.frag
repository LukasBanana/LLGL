// PBR GLSL Skybox Fragment Shader

#version 410 core

uniform Settings
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

in VSkyOut
{
    vec4 viewRay;
}
inp;

out vec4 outColor;

uniform samplerCubeArray skyBox;

void main()
{
    vec3 texCoord = normalize(cMatrix * inp.viewRay).xyz;
    outColor = texture(skyBox, vec4(texCoord, float(skyboxLayer)));
}


