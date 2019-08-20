// HLSL model shader

cbuffer Settings : register(b1)
{
    float4x4    wMatrix;
    float4x4    wMatrixInv;
    float4x4    vpMatrix;
    float4x4    vpMatrixInv;
    float3      lightDir;
    float       shininess;
    float3      viewPos;
    float       threshold;  // should be in open range (0, 0.5).
    float3      albedo;
    float       reflectance;
};


// VERTEX SHADER SCENE

struct VSceneIn
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
};

struct VSceneOut
{
	float4 position : SV_Position;
    float4 ndc      : NDC;
    float4 worldPos : WORLDPOS;
    float4 modelPos : MODELPOS;
	float4 normal   : NORMAL;
};

void VScene(VSceneIn inp, out VSceneOut outp)
{
    outp.modelPos   = float4(inp.position, 1);
    outp.worldPos   = mul(wMatrix, outp.modelPos);
	outp.position	= mul(vpMatrix, outp.worldPos);
	outp.ndc        = outp.position / outp.position.w;
	outp.normal		= mul(wMatrix, float4(inp.normal, 0));
}


// PIXEL SHADER SCENE

Texture3D<float>    noiseTexture        : register(t2);
Texture2D<float>    depthRangeTexture   : register(t3);
SamplerState        linearSampler       : register(s4);

float SampleNoise(float3 v)
{
    return noiseTexture.Sample(linearSampler, v);
}

float SampleVolume(float3 pos)
{
    float noise = SampleNoise(pos*0.5);
    return smoothstep(0.5 - threshold, 0.5 + threshold, noise);
}

float SampleVolumeInBoundary(float3 pos)
{
    return SampleVolume(mul(wMatrixInv, float4(pos, 1)).xyz);
}

float DiffuseShading(float3 normal, float3 lightVec)
{
    float NdotL = dot(normal, lightVec);
    return lerp(0.2, 1.0, max(0.0, NdotL));
}

float SpecularShading(float3 normal, float3 lightVec, float3 viewVec)
{
    float3 halfVec = normalize(viewVec + lightVec);
    float NdotH = dot(normal, halfVec);
    return pow(max(0.0, NdotH), shininess);
}

float Glitter(float3 normal, float3 lightVec, float3 viewVec, float3 noisePos)
{
    float   RdotL       = dot(reflect(-viewVec, normal), lightVec);
    float   specBase    = lerp(0.2, 1.0, saturate(RdotL));
    float3  fp          = frac(noisePos * 150.0 + SampleNoise(noisePos * 12.0) * 9.0 + viewVec * 0.1);

    fp *= (1.0 - fp);
    float   glitter     = saturate(1.0 - 7.0 * (fp.x + fp.y + fp.z));

    return glitter * pow(specBase, 1.5);
}

void UnprojectDepth(inout float4 v)
{
    v = mul(vpMatrixInv, v);
    v /= v.w;
}

float4 PScene(VSceneOut inp) : SV_Target
{
    // Get input vectors
    float3  normal      = normalize(inp.normal.xyz);
    float3  lightVec    = -lightDir.xyz;
    float3  viewVec     = normalize(viewPos.xyz - inp.worldPos.xyz);

    // Compute blinn-phong shading
    float   diffuse     = DiffuseShading(normal, lightVec);
    float   specular    = SpecularShading(normal, lightVec, viewVec) * reflectance;

    #if 1
    // Sample noise texture and apply glitter
    specular += Glitter(normal, lightVec, viewVec, inp.worldPos.xyz);
    #endif

    // Project depth back into scene
    float2  screenPos   = inp.ndc.xy;
    float2  texCoord    = screenPos * float2(0.5, -0.5) + 0.5;
    float   maxDepth    = depthRangeTexture.Sample(linearSampler, texCoord);
    float4  maxDepthPos = float4(screenPos.x, screenPos.y, maxDepth, 1.0);
    UnprojectDepth(maxDepthPos);

    // Integrate volume density
    int     numIterations   = 64;
    float   stride          = 1.0 / (float)numIterations;
    float   trace           = 0.0;
    float   density         = 0.0;
    float   depthRange      = distance(inp.worldPos.xyz, maxDepthPos.xyz);
    float   dt              = depthRange * stride * 5.0;

    for (int i = 0; i < numIterations; ++i)
    {
        float3 tracePos = lerp(inp.worldPos.xyz, maxDepthPos.xyz, trace);
        density += SampleVolumeInBoundary(tracePos) * dt;
        trace += stride;
    }

    // Apply density attenuation, i.e. rescale range [0, +inf) to [0, 1).
    density = 1.0 - exp(-density*0.5);

    // Put everything together
    return float4(albedo.rgb * diffuse * lerp(0.35, 1.5, density) + (float3)specular, 1.0);
}



