#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 color;
    vec3 lightDir;
};

in vec3 POSITION;
in vec3 NORMAL;
out vec3 v_NORMAL;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = (vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
}

