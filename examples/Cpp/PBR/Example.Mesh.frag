// PBR GLSL Mesh Fragment Shader

#version 410 core

#define M_PI 3.141592654

uniform Settings
{
    mat4    cMatrix;
    mat4    vpMatrix;
    mat4    wMatrix;
    vec2    aspectRatio;
    float   mipCount;
    float   _pad0;
    vec4    lightDir;
    uint    skyboxLayer;
    uint    materialLayer;
    uvec2   _pad1;
};

uniform samplerCubeArray skyBox;
uniform sampler2DArray colorMaps;
uniform sampler2DArray normalMaps;
uniform sampler2DArray roughnessMaps;
uniform sampler2DArray metallicMaps;

in VMeshOut
{
    vec3 tangent;
    vec3 bitangent;
    vec3 normal;
    vec2 texCoord;
    vec4 worldPos;
}
inp;

out vec4 outColor;

// Schlick's approximation of the fresnel term
vec3 SchlickFresnel(vec3 f0, float cosT)
{
    return f0 + (1.0 - f0) * pow(1.0 - cosT, 5.0);
}

float GeometricOcclusion(float a, float NdotV)
{
    float a2 = a*2;
    return 2.0 * NdotV / (NdotV + sqrt(a2 + (1.0 - a2) * (NdotV * NdotV)));
}

// GGX normal distribution
float NormalDistribution(float a, float NdotH)
{
    float a2 = a*a;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (M_PI * d * d);
}

vec3 SampleEnvironment(float roughness, vec3 reflection)
{
    float lod = roughness * mipCount;
    return texture(skyBox, vec4(reflection, float(skyboxLayer)), lod).rgb;
}

vec3 BRDF(vec3 albedo, vec3 normal, vec3 viewVec, vec3 lightVec, float roughness, float metallic)
{
    // Compute color at normal incidence
    vec3 f0 = vec3(1.0 - 0.04);
    f0 = abs((1.0 - f0) / (1.0 + f0));
    f0 = f0 * f0;
    f0 = mix(f0, albedo, metallic);

    // Compute half vector and all scalar products
    vec3 halfVec = normalize(viewVec + lightVec);
    float NdotL = clamp(dot(normal, lightVec), 0.0, 1.0);
    float NdotV = clamp(dot(normal, viewVec), 0.001, 1.0);
    float NdotH = clamp(dot(normal, halfVec), 0.001, 1.0);
    float LdotH = clamp(dot(lightVec, halfVec), 0.0, 1.0);
    float VdotH = clamp(dot(viewVec, halfVec), 0.0, 1.0);

    // Compute Cook-Torance microfacet BRDF
    float alpha = roughness * roughness;
    vec3 F = SchlickFresnel(f0, VdotH);
    float G = GeometricOcclusion(alpha, NdotV);
    float D = NormalDistribution(alpha, NdotH);

    // Compute specular term and accumulate light
    vec3 specular = F * G * D / (4.0 * NdotV);

    vec3 reflection = -normalize(reflect(viewVec, normal));
    vec3 lighting = SampleEnvironment(roughness, reflection);

    return (albedo * (NdotL + lighting * 0.2) + specular * metallic);
}

void main()
{
    vec3 texCoord = vec3(inp.texCoord, float(materialLayer));

    // Sample textures
    vec4 albedo = texture(colorMaps, texCoord);
    vec3 normal = texture(normalMaps, texCoord).rgb;
    float roughness = texture(roughnessMaps, texCoord).r;
    float metallic = texture(metallicMaps, texCoord).r;

    // Compute final normal
    mat3 tangentSpace = mat3(
        normalize(inp.bitangent),
        normalize(inp.tangent),
        normalize(inp.normal)
    );

    normal = normalize(tangentSpace * (normal * 2.0 - 1.0));

    // Get view and light directions
    vec3 viewPos = (cMatrix * vec4(0, 0, 0, 1)).xyz;
    vec3 viewVec = normalize(viewPos - inp.worldPos.xyz);

    // Compute microfacet BRDF
    vec3 color = BRDF(albedo.rgb, normal, viewVec, lightDir.xyz, roughness, metallic);

    outColor = vec4(color, albedo.a);
}

