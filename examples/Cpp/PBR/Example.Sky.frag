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
    vec4 viewRay = vec4(inp.viewRay.xy, (gl_FrontFacing ? -1.0 : +1.0), 0.0);
    vec3 texCoord = normalize(cMatrix * viewRay).xyz;
    outColor = texture(skyBox, vec4(texCoord, float(skyboxLayer)));
}


