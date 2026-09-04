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

layout(binding = 2) uniform sampler2D s_colorMapcolorMapSampler;
layout(binding = 4) uniform sampler2DShadow s_shadowMapshadowMapSampler;

layout(location = 0) in vec2 v_TEXCOORD;
layout(location = 1) in vec3 v_WORLDPOS;
layout(location = 0) out vec4 SV_Target;

mat4 spvWorkaroundRowMajor(mat4 wrap) { return wrap; }

void main()
{
    vec4 _70 = texture(s_colorMapcolorMapSampler, v_TEXCOORD * groundScale) * vec4(groundTint, 1.0);
    vec2 _76 = step(fract(gl_FragCoord.xy * 0.5), vec2(0.25));
    vec4 _84 = vec4(v_WORLDPOS, 1.0) * spvWorkaroundRowMajor(vpShadowMatrix);
    vec3 _90 = ((_84 / vec4(_84.w)).xyz * vec3(0.5, -0.5, 0.5)) + vec3(0.5);
    vec2 _93 = _90.xy;
    float _98 = _90.z;
    SV_Target = vec4((_70.xyz * lightColor) * mix(ambientItensity, 1.0, (((texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(-1.5, 0.5)) * shadowSizeInv), _98)) + texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(0.5)) * shadowSizeInv), _98))) + texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(-1.5)) * shadowSizeInv), _98))) + texture(s_shadowMapshadowMapSampler, vec3(_93 + ((_76 + vec2(0.5, -1.5)) * shadowSizeInv), _98))) * 0.25), _70.w);
}

