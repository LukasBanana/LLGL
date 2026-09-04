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

uniform highp sampler2D s_colorMapcolorMapSampler;
uniform highp sampler2DShadow s_shadowMapshadowMapSampler;

in highp vec2 v_TEXCOORD;
in highp vec3 v_WORLDPOS;
layout(location = 0) out highp vec4 SV_Target;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    highp vec4 _70 = texture(s_colorMapcolorMapSampler, v_TEXCOORD * groundScale) * vec4(groundTint, 1.0);
    highp vec2 _76 = step(fract(gl_FragCoord.xy * 0.5), vec2(0.25));
    highp vec4 _84 = vec4(v_WORLDPOS, 1.0) * spvWorkaroundRowMajor(vpShadowMatrix);
    highp vec3 _90 = ((_84 / vec4(_84.w)).xyz * vec3(0.5, -0.5, 0.5)) + vec3(0.5);
    highp vec2 _93 = _90.xy;
    highp float _98 = _90.z;
    SV_Target = vec4((_70.xyz * lightColor) * mix(ambientItensity, 1.0, (((texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(-1.5, 0.5)) * shadowSizeInv), _98)) + texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(0.5)) * shadowSizeInv), _98))) + texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(-1.5)) * shadowSizeInv), _98))) + texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(0.5, -1.5)) * shadowSizeInv), _98))) * 0.25), _70.w);
}

