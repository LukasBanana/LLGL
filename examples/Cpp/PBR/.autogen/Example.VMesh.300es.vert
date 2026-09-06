#version 300 es

layout(std140) uniform Settings
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uvec2 _pad1;
};

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
layout(location = 2) in vec3 TANGENT;
layout(location = 3) in vec3 BITANGENT;
layout(location = 4) in vec2 TEXCOORD;
out vec3 v_TANGENT;
out vec3 v_BITANGENT;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;
out vec4 v_WORLDPOS;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    vec4 _49 = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix);
    gl_Position = _49 * spvWorkaroundRowMajor(vpMatrix);
    v_TANGENT = normalize(vec4(TANGENT, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_BITANGENT = normalize(vec4(BITANGENT, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_NORMAL = normalize(vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_TEXCOORD = TEXCOORD;
    v_WORLDPOS = _49;
}

