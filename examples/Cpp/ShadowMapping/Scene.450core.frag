// GLSL scene fragment shader

#version 450 core

layout(std140, binding = 1) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    mat4 vpShadowMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

layout(binding = 2) uniform texture2D shadowMap;
layout(binding = 3) uniform samplerShadow shadowMapSampler;

layout(location = 0) in vec4 vWorldPos;
layout(location = 1) in vec4 vNormal;

layout(location = 0) out vec4 fragColor;

void main()
{
    // Project world position into shadow-map space
    vec4 shadowPos = vpShadowMatrix * vWorldPos;
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * vec2(0.5, -0.5) + 0.5;
    
    // Sample shadow map
    float shadow = texture(sampler2DShadow(shadowMap, shadowMapSampler), shadowPos.xyz);
    
    // Compute lighting
    vec3 normal = normalize(vNormal.xyz);
    float NdotL = max(0.2, dot(normal, -lightDir.xyz));
    
    // Set final output color
    shadow = mix(0.2, 1.0, shadow);
    fragColor = vec4(diffuse.rgb * (NdotL * shadow), 1.0);
}
