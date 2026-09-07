#version 140

layout(std140) uniform Scene
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec3 lightVec;
};

in vec3 POSITION;
in vec3 NORMAL;
in vec2 TEXCOORD;
out vec3 v_NORMAL;
out vec4 v_COLOR;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = normalize((vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz);
    v_COLOR = vec4(TEXCOORD, 1.0, 1.0);
}

