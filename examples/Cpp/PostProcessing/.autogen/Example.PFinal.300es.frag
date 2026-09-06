#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform SceneSettings
{
    layout(row_major) highp mat4 wvpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec4 diffuse;
    highp vec4 glossiness;
    highp vec3 lightDir;
    highp float intensity;
};

uniform highp sampler2D s_colorMapcolorMapSampler;
uniform highp sampler2D s_glossMapglossMapSampler;

in highp vec2 v_TEXCOORD;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    SV_Target = texture(s_colorMapcolorMapSampler, v_TEXCOORD) + (texture(s_glossMapglossMapSampler, v_TEXCOORD) * intensity);
}

