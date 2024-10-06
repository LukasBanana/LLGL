/*
 * StreamOutput.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

cbuffer SOScene : register(b1)
{
    float4x4    vsMatrix;
    float4x4    gsMatrices[3];
    float4      lightVec;
    float       normalizeFactorVS;
    float       normalizeFactorDS;
    float       tessLevelOuter;
    float       tessLevelInner;
}

// VERTEX SHADER

struct VertexIn
{
    float4 position : POSITION;
    float3 normal   : NORMAL;
    float3 color    : COLOR;
};

struct VertexOut
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float3 color    : COLOR;
};

void VSMain(VertexIn inp, out VertexOut outp)
{
    float4 normalizedPos = float4(normalize(inp.position.xyz), 1.0);
    outp.position = mul(vsMatrix, lerp(inp.position, normalizedPos, normalizeFactorVS));
    outp.normal   = normalize(lerp(inp.normal, normalizedPos.xyz, normalizeFactorVS));
    outp.color    = inp.color;
}

// HULL SHADER

struct HullOut
{
	float edges[3] : SV_TessFactor;
	float inner[1] : SV_InsideTessFactor;
};

HullOut HSPatchConstant(InputPatch<VertexOut, 3> inp)
{
	HullOut outp;
	
	outp.edges[0] = tessLevelOuter;
	outp.edges[1] = tessLevelOuter;
	outp.edges[2] = tessLevelOuter;
	
	outp.inner[0] = tessLevelInner;
	
	return outp;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSPatchConstant")]
[maxtessfactor(16.0)]
VertexIn HSMain(InputPatch<VertexOut, 3> inp, uint id : SV_OutputControlPointID)
{
	VertexIn outp;
	outp.position   = inp[id].position;
    outp.normal     = inp[id].normal;
    outp.color      = inp[id].color;
	return outp;
}

// DOMAIN SHADER

[domain("tri")]
VertexOut DSMain(HullOut inp, float3 tessCoord : SV_DomainLocation, const OutputPatch<VertexIn, 3> patch)
{
	VertexOut outp;
	
	// Interpolate position
	float4 position = patch[0].position * tessCoord.x + patch[1].position * tessCoord.y + patch[2].position * tessCoord.z;
    float4 normalizedPos = float4(normalize(position.xyz), 1.0);
    outp.position = lerp(position, normalizedPos, normalizeFactorDS);
	
	// Interpolate normal
	float3 normal = patch[0].normal * tessCoord.x + patch[1].normal * tessCoord.y + patch[2].normal * tessCoord.z;
    outp.normal = lerp(normal, normalizedPos.xyz, normalizeFactorDS);

    // Interpolate color
	outp.color = patch[0].color * tessCoord.x + patch[1].color * tessCoord.y + patch[2].color * tessCoord.z;
	
	return outp;
}

// GEOMETRY SHADER

[maxvertexcount(9)]
void GSMain(triangle VertexOut inp[3], inout TriangleStream<VertexOut> streamOutput)
{
    const float3 instanceColors[3] =
    {
        float3(0.8, 0.2, 0.1), // red
        float3(0.2, 0.9, 0.2), // green
        float3(0.2, 0.3, 0.9), // blue
    };

    for (int instanceIndex = 0; instanceIndex < 3; ++instanceIndex)
    {
        for (int vertexIndex = 0; vertexIndex < 3; ++vertexIndex)
        {
            VertexOut outp;
            outp.position   = mul(gsMatrices[instanceIndex], inp[vertexIndex].position);
            outp.normal     = inp[vertexIndex].normal;
            outp.color      = inp[vertexIndex].color * instanceColors[instanceIndex];
            streamOutput.Append(outp);
        }
        streamOutput.RestartStrip();
    }
}

// PIXEL SHADER

float4 PSMain(VertexOut inp) : SV_Target
{
    float3 normal = normalize(inp.normal);
    float NdotL = saturate(dot(lightVec.xyz, normal));
    float shading = lerp(0.2, 1.0, NdotL);
    return float4(inp.color * shading, 1.0);
}
