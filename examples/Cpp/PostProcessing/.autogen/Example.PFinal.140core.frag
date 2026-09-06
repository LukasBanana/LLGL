#version 140

layout(std140) uniform SceneSettings
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 diffuse;
    vec4 glossiness;
    vec3 lightDir;
    float intensity;
};

uniform sampler2D s_colorMapcolorMapSampler;
uniform sampler2D s_glossMapglossMapSampler;

in vec2 v_TEXCOORD;
out vec4 SV_Target;

void main()
{
    SV_Target = texture(s_colorMapcolorMapSampler, v_TEXCOORD) + (texture(s_glossMapglossMapSampler, v_TEXCOORD) * intensity);
}

