#version 300 es

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

layout(location = 0) in vec3 POSITION;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    gl_Position = (vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix)) * spvWorkaroundRowMajor(vpMatrix);
}

