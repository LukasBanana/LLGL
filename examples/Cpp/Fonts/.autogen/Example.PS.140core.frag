#version 140

uniform sampler2D s_glyphTexturelinearSampler;

in vec2 v_TEXCOORD;
in vec4 v_COLOR;
out vec4 SV_Target;

void main()
{
    SV_Target = vec4(v_COLOR.xyz, v_COLOR.w * texture(s_glyphTexturelinearSampler, v_TEXCOORD).w);
}

