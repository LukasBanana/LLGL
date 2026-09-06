#version 300 es
precision mediump float;
precision highp int;

layout(std140) uniform Settings
{
    layout(row_major) highp mat4 cMatrix;
    layout(row_major) highp mat4 vpMatrix;
    layout(row_major) highp mat4 wMatrix;
    highp vec2 aspectRatio;
    highp float mipCount;
    highp float _pad0;
    highp vec4 lightDir;
    uint skyboxLayer;
    uint materialLayer;
    uvec2 _pad1;
};

uniform highp sampler2DArray s_colorMapssmpl;
uniform highp sampler2DArray s_normalMapssmpl;
uniform highp sampler2DArray s_roughnessMapssmpl;
uniform highp sampler2DArray s_metallicMapssmpl;
uniform highp samplerCubeArray s_skyBoxsmpl;

in highp vec3 v_TANGENT;
in highp vec3 v_BITANGENT;
in highp vec3 v_NORMAL;
in highp vec2 v_TEXCOORD;
in highp vec4 v_WORLDPOS;
layout(location = 0) out highp vec4 SV_Target;

highp mat4 spvWorkaroundRowMajor(highp mat4 wrap) { return wrap; }
mediump mat4 spvWorkaroundRowMajorMP(mediump mat4 wrap) { return wrap; }

void main()
{
    highp vec3 _73 = vec3(v_TEXCOORD, float(materialLayer));
    highp vec4 _77 = texture(s_colorMapssmpl, _73);
    highp vec4 _86 = texture(s_roughnessMapssmpl, _73);
    highp float _87 = _86.x;
    highp vec4 _90 = texture(s_metallicMapssmpl, _73);
    highp float _91 = _90.x;
    highp vec3 _96 = mat3(normalize(v_TANGENT), normalize(v_BITANGENT), normalize(v_NORMAL)) * ((texture(s_normalMapssmpl, _73).xyz * 2.0) - vec3(1.0));
    highp vec3 _103 = normalize((vec4(0.0, 0.0, 0.0, 1.0) * spvWorkaroundRowMajor(cMatrix)).xyz - v_WORLDPOS.xyz);
    highp vec3 _104 = _77.xyz;
    highp vec3 _108 = abs(vec3(0.02040817402303218841552734375));
    highp vec3 _111 = mix(_108 * _108, _104, vec3(_91));
    highp vec3 _113 = normalize(_103 + lightDir.xyz);
    highp float _117 = clamp(dot(_96, _103), 0.001000000047497451305389404296875, 1.0);
    highp float _119 = clamp(dot(_96, _113), 0.001000000047497451305389404296875, 1.0);
    highp float _122 = _87 * _87;
    highp float _128 = _122 * 2.0;
    highp float _137 = _122 * _122;
    highp float _141 = ((_119 * _119) * (_137 - 1.0)) + 1.0;
    SV_Target = vec4((_104 * (vec3(clamp(dot(_96, lightDir.xyz), 0.0, 1.0)) + (textureLod(s_skyBoxsmpl, vec4(-normalize(reflect(_103, _96)), float(skyboxLayer)), _87 * mipCount).xyz * 0.20000000298023223876953125))) + (((((_111 + ((vec3(1.0) - _111) * pow(1.0 - clamp(dot(_103, _113), 0.0, 1.0), 5.0))) * ((2.0 * _117) / (_117 + sqrt(_128 + ((1.0 - _128) * (_117 * _117)))))) * (_137 / ((3.1415927410125732421875 * _141) * _141))) / vec3(4.0 * _117)) * _91), _77.w);
}

