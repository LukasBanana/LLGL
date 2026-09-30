#version 140

layout(std140) uniform SceneView
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
};

in vec3 POSITION;
in vec3 NORMAL;
in vec3 TANGENT;
in vec3 BITANGENT;
in vec2 TEXCOORD;
out vec3 v_TANGENT;
out vec3 v_BITANGENT;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;
out vec4 v_WORLDPOS;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _47 = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wMatrix);
    gl_Position = _47 * spvWorkaroundRowMajor(vpMatrix);
    v_TANGENT = normalize(vec4(TANGENT, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_BITANGENT = normalize(vec4(BITANGENT, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_NORMAL = normalize(vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_TEXCOORD = TEXCOORD;
    v_WORLDPOS = _47;
}

