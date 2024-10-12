// HLSL tessellation shader


// Uniform buffer object (also named "Constant Buffer")
cbuffer Scene : register(b1)
{
    float4x4    vpMatrix;
    float4x4    vMatrix;
    float4x4    wMatrix;
    float3      lightVec;
    float       texScale;
    float       tessLevelInner;
    float       tessLevelOuter;
    float       maxHeightFactor;
    float       shininessPower;
};

struct InputVS
{
    float3 position     : POSITION;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 bitangent    : BITANGENT;
    float2 texCoord     : TEXCOORD;
};

struct OutputVS
{
    float3 worldPos     : WORLDPOS;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 bitangent    : BITANGENT;
    float2 texCoord     : TEXCOORD;
};

struct OutputHS
{
    float edges[4]      : SV_TessFactor;
    float inner[2]      : SV_InsideTessFactor;
};

struct OutputDS
{
    float4 screenPos    : SV_Position;
    float3 viewPos      : VIEWPOS;
    float3 normal       : NORMAL;
    float3 tangent      : TANGENT;
    float3 bitangent    : BITANGENT;
    float2 texCoord     : TEXCOORD;
};


// VERTEX SHADER

OutputVS VS(InputVS inp)
{
    OutputVS outp;
    outp.worldPos   = mul(wMatrix, float4(inp.position, 1)).xyz;
    outp.normal     = normalize(mul(wMatrix, float4(inp.normal, 0)).xyz);
    outp.tangent    = normalize(mul(wMatrix, float4(inp.tangent, 0)).xyz);
    outp.bitangent  = normalize(mul(wMatrix, float4(inp.bitangent, 0)).xyz);
    outp.texCoord   = inp.texCoord;
    return outp;
}


// HULL SHADER

OutputHS PatchConstantFuncHS(InputPatch<OutputVS, 4> inp)
{
    OutputHS outp;
    
    outp.edges[0] = tessLevelOuter;
    outp.edges[1] = tessLevelOuter;
    outp.edges[2] = tessLevelOuter;
    outp.edges[3] = tessLevelOuter;
    
    outp.inner[0] = tessLevelInner;
    outp.inner[1] = tessLevelInner;
    
    return outp;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFuncHS")]
[maxtessfactor(64.0)]
OutputVS HS(InputPatch<OutputVS, 4> inp, uint id : SV_OutputControlPointID)
{
    OutputVS outp;
    outp.worldPos   = inp[id].worldPos;
    outp.normal     = inp[id].normal;
    outp.tangent    = inp[id].tangent;
    outp.bitangent  = inp[id].bitangent;
    outp.texCoord   = inp[id].texCoord;
    return outp;
}


// DOMAIN SHADER

float InterpolateHeightAlongEdges(float2 coord)
{
    float2 t = 1.0 - abs(coord*2.0 - 1.0);
    return smoothstep(0.0, 0.2, min(t.x, t.y));
}

#define INTERPOLATE_PATCH(COMP)                             \
    lerp(                                                   \
        lerp(patch[0].COMP, patch[1].COMP, tessCoord.x),    \
        lerp(patch[2].COMP, patch[3].COMP, tessCoord.x),    \
        tessCoord.y                                         \
    )

SamplerState linearSampler : register(s2);
Texture2D<float> heightMap : register(t6);

[domain("quad")]
OutputDS DS(OutputHS inp, float2 tessCoord : SV_DomainLocation, const OutputPatch<OutputVS, 4> patch)
{
    OutputDS outp;
    
    // Interpolate world position
    float3 interpolatedWorldPos = INTERPOLATE_PATCH(worldPos);
    float2 interpolatedTexCoord = INTERPOLATE_PATCH(texCoord);
    float2 scaledTexCoord = (interpolatedTexCoord - 0.5)*texScale + 0.5;

    // Sample height map and create bump by moving along the patch normal vector
    float bumpHeight = heightMap.SampleLevel(linearSampler, scaledTexCoord, 0.0) * maxHeightFactor * InterpolateHeightAlongEdges(interpolatedTexCoord);

    interpolatedWorldPos += patch[0].normal * bumpHeight;

    // Transform vertex by the view-projection matrix
    outp.screenPos  = mul(vpMatrix, float4(interpolatedWorldPos, 1));
    outp.viewPos    = mul(vMatrix, float4(interpolatedWorldPos, 1)).xyz;
    outp.normal     = patch[0].normal;
    outp.tangent    = patch[0].tangent;
    outp.bitangent  = patch[0].bitangent;
    outp.texCoord   = scaledTexCoord;

    return outp;
}


// PIXEL SHADER

Texture2D colorMap : register(t3);
Texture2D<float3> normalMap : register(t4);
Texture2D<float> specularMap : register(t5);

float4 PhongShading(float4 albedo, float3 normal, float shininess, float3 viewVec)
{
    // Diffuse lighting
    float  lambertian   = saturate(dot(normal, lightVec));
    float3 diffuse      = albedo.rgb * lambertian;

    // Specular lighting
    float3 reflectVec   = normalize(reflect(lightVec, normal));
    float3 halfVec      = normalize(lightVec - viewVec);
    float specAngle     = saturate(dot(normal, halfVec));
    float3 specular     = (float3)(shininess * pow(specAngle, shininessPower));

    return float4(diffuse + specular, albedo.a);
}

float4 PS(OutputDS inp) : SV_Target
{
    // Sample color, normal, and specular maps
    float4 albedo = colorMap.Sample(linearSampler, inp.texCoord);
    float3 normal = normalize(normalMap.Sample(linearSampler, inp.texCoord) * 2.0 - 1.0);
    float roughness = specularMap.Sample(linearSampler, inp.texCoord);
    float shininess = 1.0 - roughness;

    // Transform normal from world-space into tangent-space
    float3x3 tangentSpace = float3x3(
        normalize(inp.tangent),
        normalize(inp.bitangent),
        normalize(inp.normal)
    );
    normal = mul(tangentSpace, normal);

    // Compute simple phong shading
    return PhongShading(albedo, normal, shininess, normalize(inp.viewPos));
}



