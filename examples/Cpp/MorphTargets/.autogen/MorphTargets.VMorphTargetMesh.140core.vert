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

in vec3 POSITIONA;
in vec3 NORMALA;
in vec3 POSITIONB;
in vec3 NORMALB;
in vec2 TEXCOORD;
out vec3 v_NORMAL;
out vec2 v_TEXCOORD;
flat out float v_FLIPFACE;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec3 _48 = vec3(dynamicState.interpolationFactor);
    vec3 _49 = mix(POSITIONA, POSITIONB, _48);
    gl_Position = vec4(_49.x * dynamicState.invertXAxis, _49.yz, 1.0) * spvWorkaroundRowMajor(wvpMatrix);
    v_NORMAL = (vec4(normalize(mix(NORMALA, NORMALB, _48)), 0.0) * spvWorkaroundRowMajor(wMatrix)).xyz;
    v_TEXCOORD = TEXCOORD;
    v_FLIPFACE = dynamicState.invertXAxis;
}

