#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform SceneView
{
    layout(row_major) highp mat4 wvpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec4 lightVec;
    highp vec4 baseColor;
};

struct type_PushConstant_DynamicState_t
{
    highp float texCoordScaleFront;
    highp float texCoordScaleBack;
    highp float interpolationFactor;
    highp float invertXAxis;
};

uniform type_PushConstant_DynamicState_t dynamicState;

uniform highp sampler2D s_frontPageColorMapfrontPageSampler;
uniform highp sampler2D s_backPageColorMapbackPageSampler;
uniform highp sampler2D s_paperDetailMappaperDetailMapSampler;

in highp vec3 v_NORMAL;
in highp vec2 v_TEXCOORD;
flat in highp float v_FLIPFACE;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp vec2 _74;
    if (gl_FrontFacing)
    {
        _74 = ((v_TEXCOORD - vec2(0.5)) * dynamicState.texCoordScaleFront) + vec2(0.5);
    }
    else
    {
        _74 = ((vec2(1.0 - v_TEXCOORD.x, v_TEXCOORD.y) - vec2(0.5)) * dynamicState.texCoordScaleBack) + vec2(0.5);
    }
    highp vec4 _86;
    if (gl_FrontFacing)
    {
        _86 = texture(s_frontPageColorMapfrontPageSampler, _74);
    }
    else
    {
        _86 = texture(s_backPageColorMapbackPageSampler, _74);
    }
    highp vec4 _119 = (baseColor + vec4(texture(s_paperDetailMappaperDetailMapSampler, _74).xyz - vec3(0.5), 0.0)) * vec4(mix(baseColor.xyz, _86.xyz, vec3(_86.w)), 1.0);
    SV_Target = vec4(_119.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec.xyz, normalize(v_NORMAL * (v_FLIPFACE * mix(-1.0, 1.0, float(gl_FrontFacing)))))), _119.w);
}

