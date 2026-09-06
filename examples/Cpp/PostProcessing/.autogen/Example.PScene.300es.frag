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

in highp vec3 v_NORMAL;
layout(location = 0) out highp vec4 SV_Target0;
layout(location = 1) out highp vec4 SV_Target1;

void main()
{
    highp float _30 = dot(lightDir, normalize(v_NORMAL));
    highp float _31 = isnan(_30) ? 0.4000000059604644775390625 : (isnan(0.4000000059604644775390625) ? _30 : max(0.4000000059604644775390625, _30));
    SV_Target0 = diffuse * vec4(_31, _31, _31, 1.0);
    SV_Target1 = glossiness;
}

