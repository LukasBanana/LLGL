#version 300 es
precision mediump float;
precision highp int;

struct type_PushConstant_ModelData
{
    highp vec3 lightVec;
    uint instance;
};

uniform type_PushConstant_ModelData model;

uniform highp sampler2D s_colorMapcolorMapSampler;

in highp vec3 v_NORMAL;
in highp vec2 v_TEXCOORD;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp vec4 _36 = texture(s_colorMapcolorMapSampler, v_TEXCOORD);
    highp float _40 = dot(normalize(v_NORMAL), model.lightVec);
    SV_Target = vec4(_36.xyz * (isnan(_40) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _40 : max(0.20000000298023223876953125, _40))), _36.w);
}

