#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform Scene
{
    layout(row_major) highp mat4 wvpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec3 lightVec;
};

uniform highp sampler2D s_colorMapsamplerState;

in highp vec3 v_NORMAL;
in highp vec2 v_TEXCOORD;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp vec4 _37 = texture(s_colorMapsamplerState, v_TEXCOORD);
    highp vec3 _44 = _37.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec, normalize(v_NORMAL)));
    SV_Target = vec4(_44.x, _44.y, _44.z, _37.w);
}

