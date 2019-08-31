// PBR HLSL Shader

#define M_PI 3.141592654

cbuffer Settings : register(b1)
{
    float4x4    cMatrix;
    float4x4    vpMatrix;
    float4x4    wMatrix;
    float2      aspectRatio;
    float       mipCount;
    float       _pad0;
    float4      lightDir;
    uint        skyboxLayer;
    uint        materialLayer;
    uint2       _pad1;
};

// SKYBOX SHADER

struct VSkyOut
{
    float4 position : SV_Position;
    float4 viewRay  : VIEWRAY;
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

void VSky(uint id : SV_VertexID, out VSkyOut outp)
{
    // Generate coorindate for fullscreen triangle
    outp.position = GetFullscreenTriangleVertex(id);

    // Generate view ray by vertex coordinate
    outp.viewRay = float4(outp.position.xy * aspectRatio, 1, 0);
}

SamplerState smpl : register(s2);
TextureCubeArray skyBox : register(t3);

float4 PSky(VSkyOut inp) : SV_Target
{
    float3 texCoord = normalize(mul(cMatrix, inp.viewRay)).xyz;
    return skyBox.Sample(smpl, float4(texCoord, (float)skyboxLayer));
}

// MESH SHADER

struct VMeshIn
{
    float3 position  : POSITION;
    float3 normal    : NORMAL;
    float3 tangent   : TANGENT;
    float3 bitangent : BITANGENT;
    float2 texCoord  : TEXCOORD;
};

struct VMeshOut
{
    float4 position     : SV_Position;
    float3 tangent      : TANGENT;
    float3 bitangent    : BITANGENT;
    float3 normal       : NORMAL;
    float2 texCoord     : TEXCOORD;
    float4 worldPos     : WORLDPOS;
};

void VMesh(VMeshIn inp, out VMeshOut outp)
{
    outp.worldPos   = mul(wMatrix, float4(inp.position, 1));
    outp.position   = mul(vpMatrix, outp.worldPos);
    outp.tangent    = normalize(mul(wMatrix, float4(inp.tangent, 0))).xyz;
    outp.bitangent  = normalize(mul(wMatrix, float4(inp.bitangent, 0))).xyz;
    outp.normal     = normalize(mul(wMatrix, float4(inp.normal, 0))).xyz;
    outp.texCoord   = inp.texCoord;
}

Texture2DArray<float4>  colorMaps       : register(t4);
Texture2DArray<float3>  normalMaps      : register(t5);
Texture2DArray<float>   roughnessMaps   : register(t6);
Texture2DArray<float>   metallicMaps    : register(t7);

// Schlick's approximation of the fresnel term
float3 SchlickFresnel(float3 f0, float cosT)
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

float3 BRDF(float3 albedo, float3 normal, float3 viewVec, float3 lightVec, float roughness, float metallic)
{
    // Compute color at normal incidence
    float3 f0 = (float3)(1.0 - 0.04);
    f0 = abs((1.0 - f0) / (1.0 + f0));
    f0 = f0 * f0;
    f0 = lerp(f0, albedo, metallic);

    // Compute half vector and all scalar products
    float3 halfVec = normalize(viewVec + lightVec);
    float NdotL = clamp(dot(normal, lightVec), 0.0, 1.0);
    float NdotV = clamp(dot(normal, viewVec), 0.001, 1.0);
    float NdotH = clamp(dot(normal, halfVec), 0.001, 1.0);
    float LdotH = saturate(dot(lightVec, halfVec));
    float VdotH = saturate(dot(viewVec, halfVec));

    // Compute Cook-Torance microfacet BRDF
    float alpha = roughness * roughness;
    float3 F = SchlickFresnel(f0, VdotH);
    float G = GeometricOcclusion(alpha, NdotV);
    float D = NormalDistribution(alpha, NdotH);

    // Compute specular term and accumulate light
    float3 specular = F * G * D / (4.0 * NdotV);

    return (albedo * NdotL + specular);
}

float3 SampleEnvironment(float roughness, float3 reflection)
{
    float lod = roughness * mipCount;
    return skyBox.Sample(smpl, float4(reflection, (float)skyboxLayer), lod).rgb;
}

float4 PMesh(VMeshOut inp) : SV_Target
{
    float3 texCoord = float3(inp.texCoord, (float)materialLayer);

    // Sample textures
    float4 albedo = colorMaps.Sample(smpl, texCoord);
    float3 normal = normalMaps.Sample(smpl, texCoord);
    float roughness = 0.0;//0.6;//roughnessMaps.Sample(smpl, texCoord);
    float metallic = metallicMaps.Sample(smpl, texCoord);

    // Compute final normal
    #if 1
    normal = normalize(inp.normal);
    #else
    float3x3 tangentSpace = float3x3(
        normalize(inp.bitangent),
        normalize(inp.tangent),
        normalize(inp.normal)
    );

    normal = normalize(mul(tangentSpace, (normal * 2.0 - 1.0)));
    #endif

    // Get view and light directions
    float3 viewPos = mul(cMatrix, float4(0, 0, 0, 1)).xyz;
    float3 viewVec = normalize(viewPos - inp.worldPos.xyz);

    // Sample incoming light from environment map
    float3 reflection = -normalize(reflect(viewVec, normal));
    float3 lighting = SampleEnvironment(roughness, reflection);

    // Compute microfacet BRDF
    float3 color = BRDF(albedo.rgb, normal, viewVec, lightDir.xyz, roughness, metallic);

    #if 1
    //color += lighting * 0.2;
    color = lighting;
    #endif

    return float4(color, albedo.a);
}

