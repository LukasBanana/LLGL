#version 300 es

layout(std140) uniform Scene
{
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 vpShadowMatrix;
    vec3 lightDir;
    float shininess;
    vec3 viewPos;
    float shadowSizeInv;
    vec3 warpCenter;
    float warpIntensity;
    vec3 bendDir;
    float ambientItensity;
    vec3 groundTint;
    float groundScale;
    vec3 lightColor;
    float warpScaleInv;
};

layout(location = 0) in vec3 POSITION;
layout(location = 2) in vec2 TEXCOORD;
out vec2 v_TEXCOORD;
out vec3 v_WORLDPOS;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(vpMatrix);
    v_TEXCOORD = TEXCOORD;
    v_WORLDPOS = POSITION;
}

