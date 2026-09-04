#version 140

layout(std140) uniform SceneState
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 gravity;
    uvec2 gridSize;
    uvec2 _pad0;
    float damping;
    float dTime;
    float dStiffness;
    float _pad1;
    vec4 lightVec;
};

in vec4 POS;
in vec4 NORMAL;
in vec2 TEXCOORD;
out vec4 v_NORMAL;
out vec2 v_TEXCOORD;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = POS * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = NORMAL * spvWorkaroundRowMajor(wMatrix);
    v_TEXCOORD = TEXCOORD;
}

