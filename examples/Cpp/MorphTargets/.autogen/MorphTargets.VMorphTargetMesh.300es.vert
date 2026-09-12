#version 300 es

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

layout(location = 0) in vec3 POSITIONA;
layout(location = 1) in vec3 NORMALA;
layout(location = 2) in vec3 POSITIONB;
layout(location = 3) in vec3 NORMALB;
layout(location = 4) in vec2 TEXCOORD;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;
flat out float v_FLIPFACE;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    vec3 _48 = vec3(dynamicState.interpolationFactor);
    vec3 _49 = mix(POSITIONA, POSITIONB, _48);
    gl_Position = vec4(_49.x * dynamicState.invertXAxis, _49.yz, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = (vec4(normalize(mix(NORMALA, NORMALB, _48)), 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_TEXCOORD = TEXCOORD;
    v_FLIPFACE = dynamicState.invertXAxis;
}

