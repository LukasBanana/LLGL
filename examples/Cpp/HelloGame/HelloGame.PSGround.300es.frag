#version 300 es

// ESSL shader for HelloGame example
// This file is part of the LLGL project

precision mediump float;
precision mediump sampler2DShadow;

layout(std140) uniform Scene
{
    mat4    vpMatrix;
    mat4    vpShadowMatrix;
    vec3    lightDir;
    float   shininess;
    vec3    viewPos;
    float   shadowSizeInv;
    vec3    warpCenter;
    float   warpIntensity;
    vec3    bendDir;
    float   ambientItensity;
    vec3    groundTint;
    float   groundScale;
    vec3    lightColor;
    float   warpScaleInv;
};


// PIXEL SHADER GROUND

in vec2 vTexCoord;
in vec3 vWorldPos;

layout(location = 0) out vec4 outColor;

uniform sampler2D colorMap;
uniform sampler2DShadow shadowMap;

float SampleShadowMapOffset(vec3 worldPos, vec2 offset)
{
    // Project world position into shadow-map space
    vec4 shadowPos = vpShadowMatrix * vec4(worldPos, 1);
    shadowPos /= shadowPos.w;
    shadowPos.xy = shadowPos.xy * vec2(0.5, -0.5) + 0.5 + offset * shadowSizeInv;
    
    shadowPos.z = shadowPos.z * 0.5 + 0.5; // OpenGLES NDC space adjustment

    // Sample shadow map
    return texture(shadowMap, shadowPos.xyz);
}

float SampleShadowMapPCF(vec2 screenPos, vec3 worldPos)
{
    // Perform percentage closer filtering (PCF) as described here:
    // https://developer.nvidia.com/gpugems/gpugems/part-ii-lighting-and-shadows/chapter-11-shadow-map-antialiasing
    vec2 offset = vec2(greaterThan(fract(screenPos * 0.5), vec2(0.25)));
    if (offset.y > 1.1)
        offset.y = 0.0;

    float shadowSamples =
    (
        SampleShadowMapOffset(worldPos, offset + vec2(-1.5, +0.5)) +
        SampleShadowMapOffset(worldPos, offset + vec2(+0.5, +0.5)) +
        SampleShadowMapOffset(worldPos, offset + vec2(-1.5, -1.5)) +
        SampleShadowMapOffset(worldPos, offset + vec2(+0.5, -1.5))
    );

    return shadowSamples * 0.25;
}

void main()
{
    // Sample color map
    vec4 albedo = texture(colorMap, vTexCoord * groundScale) * vec4(groundTint, 1);

    // Apply shadow mapping
    float shadow = mix(ambientItensity, 1.0, SampleShadowMapPCF(gl_FragCoord.xy, vWorldPos));

    // Set final output color
    outColor = vec4(albedo.rgb * lightColor * shadow, albedo.a);
}

