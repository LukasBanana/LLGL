#version 140

layout(std140) uniform Matrices
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
};

in vec3 POSITION;
in vec3 NORMAL;
in vec2 TEXCOORD;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = normalize((vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz);
    v_TEXCOORD = TEXCOORD;
}

