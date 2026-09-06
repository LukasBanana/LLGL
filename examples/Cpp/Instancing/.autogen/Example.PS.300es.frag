#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform Settings
{
    layout(row_major) highp mat4 vpMatrix;
    highp vec4 viewPos;
    highp vec3 fogColor;
    highp float fogDensity;
    highp vec2 animVec;
};

uniform highp sampler2DArray s_textexSampler;

in highp vec4 v_WORLDPOS;
in highp vec3 v_TEXCOORD;
in highp vec3 v_COLOR;
layout(location = 0) out highp vec4 SV_Target;

void main()
{
    highp vec4 _45 = texture(s_textexSampler, v_TEXCOORD);
    if ((_45.w - 0.5) < 0.0)
    {
        discard;
    }
    highp float _56 = distance(viewPos, v_WORLDPOS) * fogDensity;
    highp vec3 _66 = mix(_45.xyz * v_COLOR, fogColor, vec3(1.0 - (1.0 / exp(_56 * _56))));
    SV_Target = vec4(_66.x, _66.y, _66.z, _45.w);
}

