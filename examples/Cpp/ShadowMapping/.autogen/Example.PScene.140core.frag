#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 vpShadowMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

uniform sampler2DShadow s_shadowMapshadowMapSampler;

in vec4 v_WORLDPOS;
in vec4 v_NORMAL;
out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _43 = v_WORLDPOS * spvWorkaroundRowMajor(vpShadowMatrix);
    vec3 _49 = ((_43 / vec4(_43.w)).xyz * vec3(0.5, -0.5, 0.5)) + vec3(0.5);
    float _62 = dot(normalize(v_NORMAL.xyz), -lightDir.xyz);
    SV_Target = vec4(diffuse.xyz * ((isnan(_62) ? 0.20000000298023223876953125 : (isnan(0.20000000298023223876953125) ? _62 : max(0.20000000298023223876953125, _62))) * mix(0.20000000298023223876953125, 1.0, texture(s_shadowMapshadowMapSampler, vec3(_49.xy, _49.z)))), 1.0);
}

