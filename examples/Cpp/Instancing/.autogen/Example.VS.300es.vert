#version 300 es

layout(std140) uniform Settings
{
    layout(row_major) mat4 vpMatrix;
    vec4 viewPos;
    vec3 fogColor;
    float fogDensity;
    vec2 animVec;
};

layout(location = 0) in vec3 POSITION;
layout(location = 1) in vec2 TEXCOORD;
layout(location = 2) in vec3 COLOR;
layout(location = 3) in float ARRAYLAYER;
layout(location = 4) in mat4 WMATRIX;
out vec4 v_WORLDPOS;
out vec3 v_TEXCOORD;
out vec3 v_COLOR;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    vec2 _42 = animVec * POSITION.y;
    vec4 _50 = WMATRIX * vec4(POSITION.x + _42.x, POSITION.y, POSITION.z + _42.y, 1.0);
    gl_Position = _50 * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _50;
    v_TEXCOORD = vec3(TEXCOORD, ARRAYLAYER);
    v_COLOR = COLOR;
}

