#version 300 es

layout(std140) uniform SceneState
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 gravity;
    uvec2 gridSize;
    uvec2 _pad0;
    float damping;
    float dTime;
    float dStiffness;
    float _pad1;
    vec4 lightVec;
};

layout(location = 0) in vec4 POS;
layout(location = 1) in vec4 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
out vec4 v_NORMAL;
out vec2 v_TEXCOORD;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    gl_Position = POS * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = NORMAL * spvWorkaroundRowMajor(wMatrix);
    v_TEXCOORD = TEXCOORD;
}

