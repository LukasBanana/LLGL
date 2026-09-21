#version 140

layout(std140) uniform Settings
{
    layout(row_major) mat4 wMatrix;
    layout(row_major) mat4 vpMatrix;
    vec3 lightDir;
    float shininess;
    vec4 viewPos;
    vec4 albedo;
};

uniform sampler2D s_colorMaplinearSampler;

in vec4 v_WORLDPOS;
in vec4 v_NORMAL;
in vec2 v_TEXCOORD;
out vec4 SV_Target;

void main()
{
    vec3 _45 = -lightDir;
    vec3 _47 = normalize(v_NORMAL.xyz);
    float _48 = dot(_47, _45);
    float _63 = dot(_47, normalize(normalize(viewPos.xyz - v_WORLDPOS.xyz) + _45));
    SV_Target = mix(vec4(1.0), texture(s_colorMaplinearSampler, v_TEXCOORD), vec4(albedo.w)) * vec4((albedo.xyz * mix(0.20000000298023223876953125, 1.0, isnan(_48) ? 0.0 : (isnan(0.0) ? _48 : max(0.0, _48)))) + vec3(pow(isnan(_63) ? 0.0 : (isnan(0.0) ? _63 : max(0.0, _63)), shininess)), 1.0);
}

