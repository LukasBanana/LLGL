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

uniform highp sampler2D s_colorMapcolorMapSampler;
uniform highp sampler2D s_paperDetailMappaperDetailMapSampler;

in highp vec3 v_NORMAL;
in highp vec2 v_TEXCOORD;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp vec4 _79;
    do
    {
        if (dynamicState.borderSamplerFront != 0)
        {
            highp vec2 _66 = step(vec2(0.0), v_TEXCOORD) * step(v_TEXCOORD, vec2(1.0));
            bvec4 _73 = bvec4(dynamicState.borderSamplerFront == 1);
            _79 = mix(vec4(_73.x ? vec4(0.0).x : vec4(0.0, 0.0, 0.0, 1.0).x, _73.y ? vec4(0.0).y : vec4(0.0, 0.0, 0.0, 1.0).y, _73.z ? vec4(0.0).z : vec4(0.0, 0.0, 0.0, 1.0).z, _73.w ? vec4(0.0).w : vec4(0.0, 0.0, 0.0, 1.0).w), texture(s_colorMapcolorMapSampler, v_TEXCOORD), vec4(_66.x * _66.y));
            break;
        }
        _79 = texture(s_colorMapcolorMapSampler, v_TEXCOORD);
        break;
    } while(false);
    highp vec4 _108 = (baseColor + vec4(texture(s_paperDetailMappaperDetailMapSampler, v_TEXCOORD).xyz - vec3(0.5), 0.0)) * vec4(mix(baseColor.xyz, _79.xyz, vec3(_79.w)), 1.0);
    SV_Target = vec4(_108.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec.xyz, normalize(v_NORMAL))), _108.w);
}

