#version 420

layout(binding = 1, std140) uniform Settings
{
    layout(row_major) mat4 cMatrix;
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 wMatrix;
    vec2 aspectRatio;
    float mipCount;
    float _pad0;
    vec4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uvec2 _pad1;
};

layout(binding = 4) uniform sampler2DArray s_colorMapssmpl;
layout(binding = 5) uniform sampler2DArray s_normalMapssmpl;
layout(binding = 6) uniform sampler2DArray s_roughnessMapssmpl;
layout(binding = 7) uniform sampler2DArray s_metallicMapssmpl;
layout(binding = 3) uniform samplerCubeArray s_skyBoxsmpl;

layout(location = 0) in vec3 v_TANGENT;
layout(location = 1) in vec3 v_BITANGENT;
layout(location = 2) in vec3 v_NORMAL;
layout(location = 3) in vec2 v_TEXCOORD;
layout(location = 4) in vec4 v_WORLDPOS;
layout(location = 0) out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec3 _73 = vec3(v_TEXCOORD, float(materialLayer));
    vec4 _77 = texture(s_colorMapssmpl, _73);
    vec4 _86 = texture(s_roughnessMapssmpl, _73);
    float _87 = _86.x;
    vec4 _90 = texture(s_metallicMapssmpl, _73);
    float _91 = _90.x;
    vec3 _96 = mat3(normalize(v_TANGENT), normalize(v_BITANGENT), normalize(v_NORMAL)) * ((texture(s_normalMapssmpl, _73).xyz * 2.0) - vec3(1.0));
    vec3 _103 = normalize((vec4(0.0, 0.0, 0.0, 1.0) * spvWorkaroundRowMajor(cMatrix)).xyz - v_WORLDPOS.xyz);
    vec3 _104 = _77.xyz;
    vec3 _108 = abs(vec3(0.02040817402303218841552734375));
    vec3 _111 = mix(_108 * _108, _104, vec3(_91));
    vec3 _113 = normalize(_103 + lightDir.xyz);
    float _117 = clamp(dot(_96, _103), 0.001000000047497451305389404296875, 1.0);
    float _119 = clamp(dot(_96, _113), 0.001000000047497451305389404296875, 1.0);
    float _122 = _87 * _87;
    float _128 = _122 * 2.0;
    float _137 = _122 * _122;
    float _141 = ((_119 * _119) * (_137 - 1.0)) + 1.0;
    SV_Target = vec4((_104 * (vec3(clamp(dot(_96, lightDir.xyz), 0.0, 1.0)) + (textureLod(s_skyBoxsmpl, vec4(-normalize(reflect(_103, _96)), float(skyboxLayer)), _87 * mipCount).xyz * 0.20000000298023223876953125))) + (((((_111 + ((vec3(1.0) - _111) * pow(1.0 - clamp(dot(_103, _113), 0.0, 1.0), 5.0))) * ((2.0 * _117) / (_117 + sqrt(_128 + ((1.0 - _128) * (_117 * _117)))))) * (_137 / ((3.1415927410125732421875 * _141) * _141))) / vec3(4.0 * _117)) * _91), _77.w);
}

