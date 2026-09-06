#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform BlurSettings
{
    highp vec2 blurShift;
};

uniform highp sampler2D s_glossMapglossMapSampler;

in highp vec2 v_TEXCOORD;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp vec2 _40 = blurShift * 5.0;
    highp vec2 _45 = blurShift * 4.0;
    highp vec2 _51 = blurShift * 3.0;
    highp vec2 _57 = blurShift * 2.0;
    SV_Target = ((((((((((texture(s_glossMapglossMapSampler, v_TEXCOORD - _40) * 0.014374000020325183868408203125) + (texture(s_glossMapglossMapSampler, v_TEXCOORD - _45) * 0.0358549989759922027587890625)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD - _51) * 0.072994001209735870361328125)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD - _57) * 0.1212809979915618896484375)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD - blurShift) * 0.16447199881076812744140625)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD) * 0.1820490062236785888671875)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD + blurShift) * 0.16447199881076812744140625)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD + _57) * 0.1212809979915618896484375)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD + _51) * 0.072994001209735870361328125)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD + _45) * 0.0358549989759922027587890625)) + (texture(s_glossMapglossMapSampler, v_TEXCOORD + _40) * 0.014374000020325183868408203125);
}

