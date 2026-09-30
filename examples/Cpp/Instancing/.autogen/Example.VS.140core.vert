#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 vpMatrix;
    vec4 viewPos;
    vec3 fogColor;
    float fogDensity;
    vec2 animVec;
};

in vec3 POSITION;
in vec2 TEXCOORD;
in vec3 COLOR;
in float ARRAYLAYER;
in mat4 WMATRIX;
out vec4 v_WORLDPOS;
out vec3 v_TEXCOORD;
out vec3 v_COLOR;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec2 _42 = animVec * POSITION.y;
    vec4 _50 = WMATRIX * vec4(POSITION.x + _42.x, POSITION.y, POSITION.z + _42.y, 1.0);
    gl_Position = _50 * spvWorkaroundRowMajor(vpMatrix);
    v_WORLDPOS = _50;
    v_TEXCOORD = vec3(TEXCOORD, ARRAYLAYER);
    v_COLOR = COLOR;
}

