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
    int borderSamplerFront;
    int borderSamplerBack;
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
    highp vec2 _84;
    if (gl_FrontFacing)
    {
        _84 = ((v_TEXCOORD - vec2(0.5)) * dynamicState.texCoordScaleFront) + vec2(0.5);
    }
    else
    {
        _84 = ((vec2(1.0 - v_TEXCOORD.x, v_TEXCOORD.y) - vec2(0.5)) * dynamicState.texCoordScaleBack) + vec2(0.5);
    }
    highp vec4 _138;
    if (gl_FrontFacing)
    {
        highp vec4 _112;
        do
        {
            if (dynamicState.borderSamplerFront != 0)
            {
                highp vec2 _99 = step(vec2(0.0), _84) * step(_84, vec2(1.0));
                bvec4 _106 = bvec4(dynamicState.borderSamplerFront == 1);
                _112 = mix(vec4(_106.x ? vec4(0.0).x : vec4(0.0, 0.0, 0.0, 1.0).x, _106.y ? vec4(0.0).y : vec4(0.0, 0.0, 0.0, 1.0).y, _106.z ? vec4(0.0).z : vec4(0.0, 0.0, 0.0, 1.0).z, _106.w ? vec4(0.0).w : vec4(0.0, 0.0, 0.0, 1.0).w), texture(s_frontPageColorMapfrontPageSampler, _84), vec4(_99.x * _99.y));
                break;
            }
            _112 = texture(s_frontPageColorMapfrontPageSampler, _84);
            break;
        } while(false);
        _138 = _112;
    }
    else
    {
        highp vec4 _137;
        do
        {
            if (dynamicState.borderSamplerBack != 0)
            {
                highp vec2 _124 = step(vec2(0.0), _84) * step(_84, vec2(1.0));
                bvec4 _131 = bvec4(dynamicState.borderSamplerBack == 1);
                _137 = mix(vec4(_131.x ? vec4(0.0).x : vec4(0.0, 0.0, 0.0, 1.0).x, _131.y ? vec4(0.0).y : vec4(0.0, 0.0, 0.0, 1.0).y, _131.z ? vec4(0.0).z : vec4(0.0, 0.0, 0.0, 1.0).z, _131.w ? vec4(0.0).w : vec4(0.0, 0.0, 0.0, 1.0).w), texture(s_backPageColorMapbackPageSampler, _84), vec4(_124.x * _124.y));
                break;
            }
            _137 = texture(s_backPageColorMapbackPageSampler, _84);
            break;
        } while(false);
        _138 = _137;
    }
    highp vec4 _171 = (baseColor + vec4(texture(s_paperDetailMappaperDetailMapSampler, _84).xyz - vec3(0.5), 0.0)) * vec4(mix(baseColor.xyz, _138.xyz, vec3(_138.w)), 1.0);
    SV_Target = vec4(_171.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec.xyz, normalize(v_NORMAL * (v_FLIPFACE * mix(-1.0, 1.0, float(gl_FrontFacing)))))), _171.w);
}

