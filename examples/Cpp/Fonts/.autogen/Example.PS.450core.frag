#version 450

layout(binding = 0) uniform sampler2D s_glyphTexturelinearSampler;

layout(location = 0) in vec2 v_TEXCOORD;
layout(location = 1) in vec4 v_COLOR;
layout(location = 0) out vec4 SV_Target;

void main()
{
    SV_Target = vec4(v_COLOR.xyz, v_COLOR.w * texture(s_glyphTexturelinearSampler, v_TEXCOORD).w);
}

