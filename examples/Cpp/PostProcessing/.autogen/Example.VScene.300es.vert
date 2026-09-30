#version 300 es

layout(std140) uniform SceneSettings
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 diffuse;
    vec4 glossiness;
    vec3 lightDir;
    float intensity;
};

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
out vec3 v_NORMAL;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = (vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
}

