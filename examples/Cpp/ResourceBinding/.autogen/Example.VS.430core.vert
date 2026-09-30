#version 430

struct Transform
{
    mat4 wMatrix;
};

layout(binding = 3, std140) uniform Scene
{
    layout(row_major) mat4 vpMatrix;
};

layout(binding = 1, std430) readonly buffer type_StructuredBuffer_Transform
{
    layout(row_major) Transform _m0[];
} transforms;

struct type_PushConstant_ModelData
{
    vec3 lightVec;
    uint instance;
};

uniform type_PushConstant_ModelData model;

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec2 TEXCOORD;
layout(location = 0) out vec4 v_WORLDPOS;
layout(location = 1) out vec3 v_NORMAL;
layout(location = 2) out vec2 v_TEXCOORD;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _54 = vec4(POSITION, 1.0) * transforms._m0[model.instance].wMatrix;
    gl_Position = _54 * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _54;
    v_NORMAL = (vec4(NORMAL, 0.0) * transforms._m0[model.instance].wMatrix).xyz;
    v_TEXCOORD = TEXCOORD;
}

