#version 300 es
precision mediump float;
precision highp int;

in highp vec3 v_COLOR;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    SV_Target = vec4(v_COLOR, 1.0);
}

