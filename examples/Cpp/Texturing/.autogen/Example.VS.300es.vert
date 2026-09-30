#version 300 es

layout(std140) uniform Scene
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec3 lightVec;
};

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = normalize((vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz);
    v_TEXCOORD = TEXCOORD;
}

