#version 140

layout(std140) uniform SceneView
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
};

uniform sampler2D s_colorMapsmpl;
uniform sampler2D s_normalMapsmpl;
uniform sampler2D s_roughnessMapsmpl;
uniform sampler2D s_metallicMapsmpl;
uniform samplerCube s_skyBoxsmpl;

in vec3 v_TANGENT;
in vec3 v_BITANGENT;
in vec3 v_NORMAL;
in vec2 v_TEXCOORD;
in vec4 v_WORLDPOS;
out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _66 = texture(s_colorMapsmpl, v_TEXCOORD);
    vec4 _75 = texture(s_roughnessMapsmpl, v_TEXCOORD);
    float _76 = _75.x;
    vec4 _79 = texture(s_metallicMapsmpl, v_TEXCOORD);
    float _80 = _79.x;
    vec3 _85 = mat3(normalize(v_TANGENT), normalize(v_BITANGENT), normalize(v_NORMAL)) * ((texture(s_normalMapsmpl, v_TEXCOORD).xyz * 2.0) - vec3(1.0));
    vec3 _92 = normalize((vec4(0.0, 0.0, 0.0, 1.0) * spvWorkaroundRowMajor(cMatrix)).xyz - v_WORLDPOS.xyz);
    vec3 _93 = _66.xyz;
    vec3 _97 = abs(vec3(0.02040817402303218841552734375));
    vec3 _100 = mix(_97 * _97, _93, vec3(_80));
    vec3 _102 = normalize(_92 + lightDir.xyz);
    float _106 = clamp(dot(_85, _92), 0.001000000047497451305389404296875, 1.0);
    float _108 = clamp(dot(_85, _102), 0.001000000047497451305389404296875, 1.0);
    float _111 = _76 * _76;
    float _117 = _111 * 2.0;
    float _126 = _111 * _111;
    float _130 = ((_108 * _108) * (_126 - 1.0)) + 1.0;
    SV_Target = vec4((_93 * (vec3(clamp(dot(_85, lightDir.xyz), 0.0, 1.0)) + (textureLod(s_skyBoxsmpl, -normalize(reflect(_92, _85)), _76 * mipCount).xyz * 0.20000000298023223876953125))) + (((((_100 + ((vec3(1.0) - _100) * pow(1.0 - clamp(dot(_92, _102), 0.0, 1.0), 5.0))) * ((2.0 * _106) / (_106 + sqrt(_117 + ((1.0 - _117) * (_106 * _106)))))) * (_126 / ((3.1415927410125732421875 * _130) * _130))) / vec3(4.0 * _106)) * _80), _66.w);
}

