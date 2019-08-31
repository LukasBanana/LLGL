// PBR Metal Shader

#include <metal_stdlib>
#include <simd/simd.h>

#define M_PI 3.141592654

using namespace metal;

struct Settings
{
    float4x4    cMatrix;
    float4x4    vpMatrix;
    float4x4    wMatrix;
    float2      aspectRatio;
    float2      _pad0;
    float4      lightDir;
    uint        skyboxLayer;
    uint        materialLayer;
    uint2       _pad1;
};

// SKYBOX SHADER

struct VertexSkyOut
{
    float4 position [[position]];
    float4 viewRay;
};

/*
This function generates the coordinates for a fullscreen triangle with its vertex IDs:

(-1,+3)
   *
   | \
   |   \
   |     \
   |       \
(-1,+1)    (+1,+1)
   *----------*\
   |          |  \
   |  screen  |    \
   |          |      \
   *----------*--------*
(-1,-1)    (+1,-1)  (+3,-1)
*/
float4 GetFullscreenTriangleVertex(uint id)
{
    return float4(
        (id == 2 ? 3.0 : -1.0),
        (id == 0 ? 3.0 : -1.0),
        1.0,
        1.0
    );
}

vertex VertexSkyOut VSky(
    uint                id       [[vertex_id]],
    constant Settings&  settings [[buffer(1)]])
{
    VertexSkyOut outp;

    // Generate coorindate for fullscreen triangle
    outp.position = GetFullscreenTriangleVertex(id);

    // Generate view ray by vertex coordinate
    outp.viewRay = float4(outp.position.xy * settings.aspectRatio, 1, 0);

    return outp;
}

fragment float4 PSky(
    VertexSkyOut                inp      [[stage_in]],
    constant Settings&          settings [[buffer(1)]],
    sampler                     smpl     [[sampler(2)]],
    texturecube_array<float>    skybox   [[texture(3)]])
{
    float3 texCoord = normalize(settings.cMatrix * inp.viewRay).xyz;
    return skybox.sample(smpl, texCoord, settings.skyboxLayer);
}

// MESH SHADER

struct VertexIn
{
    float3 position  [[attribute(0)]];
    float3 normal    [[attribute(1)]];
    float3 tangent   [[attribute(2)]];
    float3 bitangent [[attribute(3)]];
    float2 texCoord  [[attribute(4)]];
};

struct VertexOut
{
    float4 position [[position]];
    float3 tangent;
    float3 bitangent;
    float3 normal;
    float2 texCoord;
    float4 worldPos;
};

vertex VertexOut VMesh(
    VertexIn           inp      [[stage_in]],
    constant Settings& settings [[buffer(1)]])
{
    VertexOut outp;

    outp.worldPos   = settings.wMatrix * float4(inp.position, 1);
    outp.position   = settings.vpMatrix * outp.worldPos;
    outp.tangent    = normalize(settings.wMatrix * float4(inp.tangent, 0)).xyz;
    outp.bitangent  = normalize(settings.wMatrix * float4(inp.bitangent, 0)).xyz;
    outp.normal     = normalize(settings.wMatrix * float4(inp.normal, 0)).xyz;
    outp.texCoord   = inp.texCoord;

    return outp;
}

float3 SpecularReflection(float3 f0, float3 f90, float VdotH)
{
    return mix(f0, f90, pow(saturate(1.0 - VdotH), 5.0));
}

float GeometricOcclusion(float NdotL, float NdotV, float a)
{
    float attnL = 2.0 * NdotL / (NdotL + sqrt(a * a + (1.0 - a * a) * (NdotL * NdotL)));
    float attnV = 2.0 * NdotV / (NdotV + sqrt(a * a + (1.0 - a * a) * (NdotV * NdotV)));
    return attnL * attnV;
}

float MicrofacetDistribution(float a, float NdotH)
{
    float a2 = a*a;
    float f = (NdotH * a2 - NdotH) * NdotH + 1.0;
    return a2 / (M_PI * f * f);
}

float3 SampleEnvironment(
    texturecube_array<float>    envMap,
    uint                        envMapLayer,
    sampler                     smpl,
    float                       roughness,
    float3                      reflection)
{
    float mipCount = 1.0; // resolution of 1024x1024
    float lod = roughness * mipCount;

    float3 diffuse = envMap.sample(smpl, reflection, envMapLayer, level(lod)).rgb;

    return diffuse;
}

float3 BRDF(
    texturecube_array<float>    envMap,
    uint                        envMapLayer,
    sampler                     smpl,
    float3                      albedo,
    float3                      normal,
    float3                      viewDir,
    float3                      lightDir,
    float                       roughness,
    float                       metallic)
{
    float3 f0 = float3(0.04);
    float alpha = roughness * roughness;

    float3 halfDir = normalize(viewDir + lightDir);
    float3 reflection = -normalize(reflect(viewDir, normal));

    float3 diffuseColor  = (albedo * (1.0 - f0) * (1.0 - metallic));
    float3 specularColor = mix(f0, albedo, metallic);

    float reflectance = max(max(specularColor.r, specularColor.g), specularColor.b);
    float reflectance90 = saturate(reflectance * 25.0);

    float NdotL = clamp(dot(normal, lightDir), 0.001, 1.0);
    float NdotV = clamp(abs(dot(normal, viewDir)), 0.001, 1.0);
    float NdotH = saturate(dot(normal, halfDir));
    float LdotH = saturate(dot(lightDir, halfDir));
    float VdotH = saturate(dot(viewDir, halfDir));

    float3 F = SpecularReflection(specularColor, (float3)reflectance90, VdotH);
    float  G = GeometricOcclusion(NdotL, NdotV, alpha);
    float  D = MicrofacetDistribution(alpha, NdotH);

    float3 diffuse  = diffuseColor * (1.0 - F);
    float3 specular = F * G * D / (4.0 * NdotL * NdotV);

    float3 color = (diffuse + specular) * NdotL;

    //color += SampleEnvironment(envMap, envMapLayer, smpl, roughness, reflection) * 0.2;

    return color;
}

fragment float4 PMesh(
    VertexOut                   inp           [[stage_in]],
    constant Settings&          settings      [[buffer(1)]],
    sampler                     smpl          [[sampler(2)]],
    texturecube_array<float>    skybox        [[texture(3)]],
    texture2d_array<float>      colorMaps     [[texture(4)]],
    texture2d_array<float>      normalMaps    [[texture(5)]],
    texture2d_array<float>      roughnessMaps [[texture(6)]],
    texture2d_array<float>      metallicMaps  [[texture(7)]])
{
    // Sample textures
    uint layer = settings.materialLayer;

    float4 albedo = colorMaps.sample(smpl, inp.texCoord, layer);
    float3 normal = normalMaps.sample(smpl, inp.texCoord, layer).rgb;
    float roughness = roughnessMaps.sample(smpl, inp.texCoord, layer).r;
    float metallic = metallicMaps.sample(smpl, inp.texCoord, layer).r;

    // Compute final normal
    float3x3 tangentSpace = float3x3(
        normalize(inp.tangent),
        normalize(inp.bitangent),
        normalize(inp.normal)
    );

    normal = normalize(tangentSpace * (normal * 2.0 - 1.0));

    // Get view and light directions
    float3 viewPos = (settings.cMatrix * float4(0, 0, 0, 1)).xyz;
    float3 viewDir = normalize(viewPos - inp.worldPos.xyz);

    // Compute fragment color
    float4 color = albedo;

    color.rgb = BRDF(
        skybox,
        settings.skyboxLayer,
        smpl,
        albedo.rgb,
        normal,
        viewDir,
        settings.lightDir.xyz,
        roughness,
        metallic
    );

    #if 0
    color.rgb = normal * 0.5 + 0.5;
    #endif

    return color;
}

