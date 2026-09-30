#version 300 es

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 wMatrixInv;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 vpMatrixInv;
    vec3 lightDir;
    float shininess;
    vec3 viewPos;
    float threshold;
    vec3 albedo;
    float reflectance;
    ivec2 viewportExtent;
};

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec3 NORMAL;
out vec4 v_NDC;
out vec4 v_WORLDPOS;
out vec4 v_MODELPOS;
out vec4 v_NORMAL;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    vec4 _33 = vec4(POSITION, 1.0);
    vec4 _36 = _33 * spvWorkaroundRowMajor(wMatrix);
    vec4 _39 = _36 * spvWorkaroundRowMajor(vpMatrix);
    gl_Position = _39;
    v_NDC = _39 / vec4(_39.w);
    v_WORLDPOS = _36;
    v_MODELPOS = _33;
    v_NORMAL = vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix);
}

