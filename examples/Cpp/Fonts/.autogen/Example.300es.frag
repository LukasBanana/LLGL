#version 300 es
precision mediump float;
precision highp int;

uniform highp sampler2D s_glyphTexturelinearSampler;

in highp vec2 v_TEXCOORD;
in highp vec4 v_COLOR;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    SV_Target = vec4(v_COLOR.xyz, v_COLOR.w * texture(s_glyphTexturelinearSampler, v_TEXCOORD).w);
}

