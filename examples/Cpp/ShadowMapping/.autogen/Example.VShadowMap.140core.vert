#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 vpShadowMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

in vec3 POSITION;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = (vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix)) * spvWorkaroundRowMajor(vpShadowMatrix);
}

