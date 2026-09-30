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
in vec3 NORMAL;
out vec4 v_WORLDPOS;
out vec4 v_NORMAL;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _33 = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix);
    gl_Position = _33 * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _33;
    v_NORMAL = vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix);
}

