// GLSL scene fragment shader

#version 140

// Specifies whether to enable Percentage-Closer-Filtering (PCF)
//#define ENABLE_PCF

#define PFC_OFFSET 0.005

layout(std140) uniform Settings
{
    mat4 wMatrix;
    mat4 vpMatrix;
    mat4 vpShadowMatrix;
    vec4 lightDir;
    vec4 diffuse;
};

uniform sampler2DShadow shadowMap;

in vec4 vWorldPos;
in vec4 vNormal;

out vec4 fragColor;

void main()
{
    // Project world position into shadow-map space
    vec4 shadowPos = vpShadowMatrix * vWorldPos;
    shadowPos /= shadowPos.w;
    shadowPos = shadowPos * 0.5 + 0.5;
    
    // Sample shadow map
    #ifdef ENABLE_PCF
    vec3[4] offsets = vec3[4](
        vec3(-PFC_OFFSET, -PFC_OFFSET, 0),
        vec3(-PFC_OFFSET, +PFC_OFFSET, 0),
        vec3(+PFC_OFFSET, +PFC_OFFSET, 0),
        vec3(+PFC_OFFSET, -PFC_OFFSET, 0)
    );
    vec4 shadowVec = vec4(
        texture(shadowMap, shadowPos.xyz + offsets[0]),
        texture(shadowMap, shadowPos.xyz + offsets[1]),
        texture(shadowMap, shadowPos.xyz + offsets[2]),
        texture(shadowMap, shadowPos.xyz + offsets[3])
    );
    float shadow = dot(shadowVec, vec4(0.25));
    #else
    float shadow = texture(shadowMap, shadowPos.xyz);
    #endif
    
    // Compute lighting
    vec3 normal = normalize(vNormal.xyz);
    float NdotL = max(0.2, dot(normal, -lightDir.xyz));
    
    // Set final output color
    shadow = mix(0.2, 1.0, shadow);
    fragColor = vec4(diffuse.rgb * (NdotL * shadow), 1.0);
}
