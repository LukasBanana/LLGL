#version 450

layout(binding = 1, std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec3 lightDir;
    float shininess;
    vec4 viewPos;
    vec4 albedo;
};

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
layout(location = 0) out vec4 v_WORLDPOS;
layout(location = 1) out vec4 v_NORMAL;
layout(location = 2) out vec2 v_TEXCOORD;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _39 = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix);
    gl_Position = _39 * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _39;
    v_NORMAL = vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix);
    v_TEXCOORD = TEXCOORD;
}

