#version 140

layout(std140) uniform SceneView
{
    layout(row_major) mat4 wvpMatrix;
    layout(row_major) mat4 wMatrix;
    vec4 lightVec;
    vec4 baseColor;
};

uniform sampler2D s_colorMapcolorMapSampler;
uniform sampler2D s_paperDetailMappaperDetailMapSampler;

in vec3 v_NORMAL;
in vec2 v_TEXCOORD;
out vec4 SV_Target;

void main()
{
    vec4 _43 = texture(s_colorMapcolorMapSampler, v_TEXCOORD);
    vec4 _72 = (baseColor + vec4(texture(s_paperDetailMappaperDetailMapSampler, v_TEXCOORD).xyz - vec3(0.5), 0.0)) * vec4(mix(baseColor.xyz, _43.xyz, vec3(_43.w)), 1.0);
    SV_Target = vec4(_72.xyz * mix(0.20000000298023223876953125, 1.0, dot(lightVec.xyz, normalize(v_NORMAL))), _72.w);
}

