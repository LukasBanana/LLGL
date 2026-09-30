#version 300 es

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
out vec4 v_NORMAL;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    gl_Position = (vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix)) * spvWorkaroundRowMajor(vpMatrix);
    v_NORMAL = vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix);
}

