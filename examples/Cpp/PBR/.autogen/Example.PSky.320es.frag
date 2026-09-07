#version 320 es
precision mediump float;
precision highp int;

layout(binding = 1, std140) uniform Settings
{
    layout(row_major) highp mat4 cMatrix;
    layout(row_major) highp mat4 vpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec2 aspectRatio;
    highp float mipCount;
    highp float _pad0;
    highp vec4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uvec2 _pad1;
};

layout(binding = 3) uniform highp samplerCubeArray s_skyBoxsmpl;

layout(location = 0) in highp vec4 v_VIEWRAY;
layout(location = 0) out highp vec4 SV_Target;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    SV_Target = texture(s_skyBoxsmpl, vec4(normalize(vec4(v_VIEWRAY.xy, gl_FrontFacing ? (-1.0) : 1.0, 0.0) * spvWorkaroundRowMajor(cMatrix)).xyz, float(skyboxLayer)));
}

