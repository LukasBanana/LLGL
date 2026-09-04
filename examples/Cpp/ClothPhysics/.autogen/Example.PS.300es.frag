#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform SceneState
{
    layout(row_major) highp mat4 wvpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec4 gravity;
    uvec2 gridSize;
    uvec2 _pad0;
    highp float damping;
    highp float dTime;
    highp float dStiffness;
    highp float _pad1;
    highp vec4 lightVec;
};

uniform highp sampler2D s_colorMaplinearSampler;

in highp vec4 v_NORMAL;
in highp vec2 v_TEXCOORD;
layout(location = 0) out highp vec4 SV_Target0;

void main()
{
    highp float _54 = dot(normalize(v_NORMAL.xyz) * mix(1.0, -1.0, float(gl_FrontFacing)), -lightVec.xyz);
    highp vec4 _60 = texture(s_colorMaplinearSampler, v_TEXCOORD);
    SV_Target0 = vec4(mix(_60.xyz, vec3(v_TEXCOORD, 1.0), vec3(0.5)).xyz * mix(0.20000000298023223876953125, 1.0, isnan(_54) ? 0.0 : (isnan(0.0) ? _54 : max(0.0, _54))), _60.w);
}

