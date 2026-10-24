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
    highp vec3 _30 = normalize(vec3(0.0, 1.0, 0.0));
    highp vec3 _33 = -lightDir;
    highp float _34 = dot(_30, _33);
    highp float _44 = dot(normalize(v_NORMAL), _33);
    SV_Target = mix(vec4(diffuse.xyz * (isnan(_44) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _44 : max(0.20000000298023223876953125, _44))), diffuse.w), vec4(vec3(1.0) * (isnan(_34) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _34 : max(0.20000000298023223876953125, _34))), 1.0), vec4(reflectionFading));
}

