// Metal volume rendering shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Settings
{
    float4x4    wMatrix;
    float4x4    wMatrixInv;
    float4x4    vpMatrix;
    float4x4    vpMatrixInv;
    float4      lightDirAndShininess;
    float4      viewPosAndThreshold; // threshold should be in open range (0, 0.5).
    float4      albedoAndReflectance;
};


// VERTEX SHADER SCENE

struct VSceneIn
{
	float3 position [[attribute(0)]];
	float3 normal   [[attribute(1)]];
};

struct VSceneOut
{
	float4 position [[position]];
    float4 ndc;
    float4 worldPos;
    float4 modelPos;
	float4 normal;
};

vertex VSceneOut VScene(
    VSceneIn            inp         [[stage_in]],
    constant Settings&  settings    [[buffer(1)]])
{
    VSceneOut outp;
    outp.modelPos   = float4(inp.position, 1);
    outp.worldPos   = settings.wMatrix * outp.modelPos;
	outp.position	= settings.vpMatrix * outp.worldPos;
	outp.ndc        = outp.position / outp.position.w;
	outp.normal		= settings.wMatrix * float4(inp.normal, 0);
    return outp;
}


// PIXEL SHADER SCENE

float SampleNoise(texture3d<float> tex, sampler smpl, float3 v)
{
    return tex.sample(smpl, v).r;
}

float SampleVolume(texture3d<float> tex, sampler smpl, float threshold, float3 pos)
{
    float noise = SampleNoise(tex, smpl, pos*0.5);
    return smoothstep(0.5 - threshold, 0.5 + threshold, noise);
}

float SampleVolumeInBoundary(texture3d<float> tex, sampler smpl, float threshold, float4x4 M, float3 pos)
{
    return SampleVolume(tex, smpl, threshold, (M * float4(pos, 1)).xyz);
}

float DiffuseShading(float3 normal, float3 lightVec)
{
    float NdotL = dot(normal, lightVec);
    return mix(0.2, 1.0, max(0.0, NdotL));
}

float SpecularShading(float3 normal, float3 lightVec, float3 viewVec, float shininess)
{
    float3 halfVec = normalize(viewVec + lightVec);
    float NdotH = dot(normal, halfVec);
    return pow(max(0.0, NdotH), shininess);
}

float Glitter(texture3d<float> tex, sampler smpl, float3 normal, float3 lightVec, float3 viewVec, float3 noisePos)
{
    float   RdotL       = dot(reflect(-viewVec, normal), lightVec);
    float   specBase    = mix(0.2, 1.0, saturate(RdotL));
    float3  fp          = fract(noisePos * 150.0 + SampleNoise(tex, smpl, noisePos * 12.0) * 9.0 + viewVec * 0.1);

    fp *= (1.0 - fp);
    float   glitter     = saturate(1.0 - 7.0 * (fp.x + fp.y + fp.z));

    return glitter * pow(specBase, 1.5);
}

void UnprojectDepth(float4x4 M, thread float4& v)
{
    v = M * v;
    v /= v.w;
}

fragment float4 PScene(
    VSceneOut           inp                 [[stage_in]],
    constant Settings&  settings            [[buffer(1)]],
    texture3d<float>    noiseTexture        [[texture(2)]],
    texture2d<float>    depthRangeTexture   [[texture(3)]],
    sampler             linearSampler       [[sampler(4)]])
{
    // Get input vectors
    float3  normal      = normalize(inp.normal.xyz);
    float3  lightVec    = -settings.lightDirAndShininess.xyz;
    float3  viewVec     = normalize(settings.viewPosAndThreshold.xyz - inp.worldPos.xyz);

    // Compute blinn-phong shading
    float   diffuse     = DiffuseShading(normal, lightVec);
    float   specular    = SpecularShading(normal, lightVec, viewVec, settings.lightDirAndShininess.a) * settings.albedoAndReflectance.a;

    // Sample noise texture and apply glitter
    specular += Glitter(noiseTexture, linearSampler, normal, lightVec, viewVec, inp.worldPos.xyz);

    // Project depth back into scene
    float2  screenPos   = inp.ndc.xy;
    float2  texCoord    = screenPos * float2(0.5, -0.5) + 0.5;
    float   maxDepth    = depthRangeTexture.sample(linearSampler, texCoord).r;
    float4  maxDepthPos = float4(screenPos.x, screenPos.y, maxDepth, 1.0);
    UnprojectDepth(settings.vpMatrixInv, maxDepthPos);

    // Sample volume density
    int     numIterations   = 64;
    float   stride          = 1.0 / (float)numIterations;
    float   trace           = 0.0;
    float   density         = 0.0;

    for (int i = 0; i < numIterations; ++i)
    {
        float3 tracePos = mix(inp.worldPos.xyz, maxDepthPos.xyz, trace);
        density += SampleVolumeInBoundary(noiseTexture, linearSampler, settings.viewPosAndThreshold.a, settings.wMatrixInv, tracePos) * stride;
        trace += stride;
    }

    #if 0
    // Apply density damping on the edges
    float depthRange = distance(inp.worldPos.xyz, maxDepthPos.xyz);
    if (depthRange > 5.0)
        return 1.0;
    //density *= smoothstep(0.1, 0.2, depthRange);
    #endif

    // Put everything together
    return float4(settings.albedoAndReflectance.rgb * diffuse * mix(0.35, 1.5, density) + (float3)specular, 1.0);
}



