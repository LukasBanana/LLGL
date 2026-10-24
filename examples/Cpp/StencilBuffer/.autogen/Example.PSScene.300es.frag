#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform SceneView
{
    layout(row_major) highp mat4 wMatrix;
    layout(row_major) highp mat4 vpMatrix;
    highp vec3 lightDir;
    highp float reflectionFading;
    highp vec4 diffuse;
};

in highp vec3 v_NORMAL;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp float _30 = dot(normalize(v_NORMAL), -lightDir);
    SV_Target = vec4(diffuse.xyz * (isnan(_30) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _30 : max(0.20000000298023223876953125, _30))), diffuse.w);
}

