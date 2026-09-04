#version 450

layout(binding = 1, std140) uniform Scene
{
    layout(row_major) mat4 vpMatrix;
    layout(row_major) mat4 vpShadowMatrix;
    vec3 lightDir;
    float shininess;
    vec3 viewPos;
    float shadowSizeInv;
    vec3 warpCenter;
    float warpIntensity;
    vec3 bendDir;
    float ambientItensity;
    vec3 groundTint;
    float groundScale;
    vec3 lightColor;
    float warpScaleInv;
};

layout(binding = 4) uniform sampler2DShadow s_shadowMapshadowMapSampler;

layout(location = 0) in vec3 v_WORLDPOS;
layout(location = 1) in vec3 v_NORMAL;
layout(location = 3) in vec4 v_COLOR;
layout(location = 0) out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec3 _61 = -lightDir;
    vec3 _62 = normalize(v_NORMAL);
    float _63 = dot(_62, _61);
    float _74 = dot(_62, normalize(normalize(viewPos - v_WORLDPOS) + _61));
    vec2 _85 = step(fract(gl_FragCoord.xy * 0.5), vec2(0.25));
    vec4 _93 = vec4(v_WORLDPOS, 1.0) * spvWorkaroundRowMajor(vpShadowMatrix);
    vec3 _99 = ((_93 / vec4(_93.w)).xyz * vec3(0.5, -0.5, 0.5)) + vec3(0.5);
    vec2 _102 = _99.xy;
    float _107 = _99.z;
    vec3 _134 = (lightColor * ((v_COLOR.xyz * mix(0.20000000298023223876953125, 1.0, isnan(_63) ? 0.0 : (isnan(0.0) ? _63 : max(0.0, _63)))) + vec3(pow(isnan(_74) ? 0.0 : (isnan(0.0) ? _74 : max(0.0, _74)), shininess)))) * mix(ambientItensity, 1.0, (((texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(-1.5, 0.5)) * shadowSizeInv), _107)) + texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(0.5)) * shadowSizeInv), _107))) + texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(-1.5)) * shadowSizeInv), _107))) + texture(s_shadowMapshadowMapSampler, vec3(_102 + ((_85 + vec2(0.5, -1.5)) * shadowSizeInv), _107))) * 0.25);
    SV_Target = vec4(_134, v_COLOR.w);
}

