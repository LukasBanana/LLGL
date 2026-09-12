#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform SceneView
{
    layout(row_major) highp mat4 wvpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec4 lightVec;
    highp vec4 baseColor;
};

uniform highp sampler2D s_colorMapcolorMapSampler;
uniform highp sampler2D s_paperDetailMappaperDetailMapSampler;

in highp vec3 v_NORMAL;
in highp vec2 v_TEXCOORD;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp vec4 _43 = texture(s_colorMapcolorMapSampler, v_TEXCOORD);
    highp vec4 _72 = (baseColor + vec4(texture(s_paperDetailMappaperDetailMapSampler, v_TEXCOORD).xyz - vec3(0.5), 0.0)) * vec4(mix(baseColor.xyz, _43.xyz, vec3(_43.w)), 1.0);
    SV_Target = vec4(_72.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec.xyz, normalize(v_NORMAL))), _72.w);
}

