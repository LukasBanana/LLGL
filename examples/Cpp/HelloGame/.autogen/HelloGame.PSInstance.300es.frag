#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform Scene
{
    layout(row_major) highp mat4 vpMatrix;
    layout(row_major) highp mat4 vpShadowMatrix;
    highp vec3 lightDir;
    highp float shininess;
    highp vec3 viewPos;
    highp float shadowSizeInv;
    highp vec3 warpCenter;
    highp float warpIntensity;
    highp vec3 bendDir;
    highp float ambientItensity;
    highp vec3 groundTint;
    highp float groundScale;
    highp vec3 lightColor;
    highp float warpScaleInv;
};

uniform highp sampler2DShadow s_shadowMapshadowMapSampler;

in highp vec3 v_WORLDPOS;
in highp vec3 v_NORMAL;
in highp vec4 v_COLOR;
layout(location = 0) out highp vec4 SV_Target;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    highp vec3 _61 = -lightDir;
    highp vec3 _62 = normalize(v_NORMAL);
    highp float _63 = dot(_62, _61);
    highp float _74 = dot(_62, normalize(normalize(viewPos - v_WORLDPOS) + _61));
    highp vec2 _85 = step(fract(gl_FragCoord.xy * 0.5), vec2(0.25));
    highp vec4 _93 = vec4(v_WORLDPOS, 1.0) * spvWorkaroundRowMajor(vpShadowMatrix);
    highp vec3 _99 = ((_93 / vec4(_93.w)).xyz * vec3(0.5, -0.5, 0.5)) + vec3(0.5);
    highp vec2 _102 = _99.xy;
    highp float _107 = _99.z;
    highp vec3 _134 = (lightColor * ((v_COLOR.xyz * mix(0.20000000298023223876953125, 1.0, isnan(_63) ? 0.0 : (isnan(0.0) ? _63 : max(0.0, _63)))) + vec3(pow(isnan(_74) ? 0.0 : (isnan(0.0) ? _74 : max(0.0, _74)), shininess)))) * mix(ambientItensity, 1.0, (((texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(-1.5, 0.5)) * shadowSizeInv), _107)) + texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(0.5)) * shadowSizeInv), _107))) + texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(-1.5)) * shadowSizeInv), _107))) + texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(0.5, -1.5)) * shadowSizeInv), _107))) * 0.25);
    SV_Target = vec4(_134, v_COLOR.w);
}

