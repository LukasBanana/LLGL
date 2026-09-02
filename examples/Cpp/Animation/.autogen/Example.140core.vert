#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec3 lightDir;
    float shininess;
    vec4 viewPos;
    vec4 albedo;
};

in vec3 POSITION;
in vec3 NORMAL;
in vec2 TEXCOORD;
out vec4 v_WORLDPOS;
out vec4 v_NORMAL;
out vec2 v_TEXCOORD;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _39 = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix);
    gl_Position = _39 * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _39;
    v_NORMAL = vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix);
    v_TEXCOORD = TEXCOORD;
}

