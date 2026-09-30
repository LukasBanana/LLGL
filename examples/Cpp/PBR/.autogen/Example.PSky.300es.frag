#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform SceneView
{
    layout(row_major) highp mat4 cMatrix;
    layout(row_major) highp mat4 vpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec2 aspectRatio;
    highp float mipCount;
    highp float _pad0;
    highp vec4 lightDir;
};

uniform highp samplerCube s_skyBoxsmpl;

in highp vec4 v_VIEWRAY;
layout(location = 0) out highp vec4 SV_Target;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    SV_Target = texture(s_skyBoxsmpl, normalize(vec4(v_VIEWRAY.xy, gl_FrontFacing ? (-1.0) : 1.0, 0.0) * spvWorkaroundRowMajor(cMatrix)).xyz);
}

