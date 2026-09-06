#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform Settings
{
    layout(row_major) highp mat4 wMatrix;
    layout(row_major) highp mat4 vpMatrix;
    layout(row_major) highp mat4 vpShadowMatrix;
    highp vec4 lightDir;
    highp vec4 diffuse;
};

uniform highp sampler2DShadow s_shadowMapshadowMapSampler;

in highp vec4 v_WORLDPOS;
in highp vec4 v_NORMAL;
layout(location = 0) out highp vec4 SV_Target;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    highp vec4 _43 = v_WORLDPOS * spvWorkaroundRowMajor(vpShadowMatrix);
    highp vec3 _49 = ((_43 / vec4(_43.w)).xyz * vec3(0.5, -0.5, 0.5)) + vec3(0.5);
    highp float _62 = dot(normalize(v_NORMAL.xyz), -lightDir.xyz);
    SV_Target = vec4(diffuse.xyz * ((isnan(_62) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _62 : max(0.20000000298023223876953125, _62))) * mix(0.20000000298023223876953125, 1.0, texture(s_shadowMapshadowMapSampler, vec3(_49.xy, _49.z)))), 1.0);
}

