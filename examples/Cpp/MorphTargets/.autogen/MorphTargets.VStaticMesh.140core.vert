#version 140

layout(std140) uniform SceneView
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 lightVec;
    vec4 baseColor;
};

struct type_PushConstant_DynamicState_t
{
    float texCoordScaleFront;
    float texCoordScaleBack;
    float interpolationFactor;
    float invertXAxis;
};

uniform type_PushConstant_DynamicState_t dynamicState;

in vec3 POSITION;
in vec3 NORMAL;
in vec2 TEXCOORD;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;
flat out float v_FLIPFACE;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    gl_Position = vec4(POSITION, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = (vec4(NORMAL, 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_TEXCOORD = ((TEXCOORD - vec2(0.5)) * dynamicState.texCoordScaleFront) + vec2(0.5);
    v_FLIPFACE = 1.0;
}

