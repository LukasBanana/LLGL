/*
 * Meshlet.hlsl
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#define NUM_TRIANGLES   6
#define NUM_VERTICES    (NUM_TRIANGLES + 2)
#define NUM_COLORS      6
#define M_PI            3.141592654

float aspectRatio;
uint numMeshlets;

struct VertexOut
{
    float4 position : SV_Position;
    float3 color    : COLOR;
};

float2 GetPointOnCircle(float angle, float radius)
{
    float s, c;
    sincos(angle * M_PI * 2.0, s, c);
    return float2(s * radius * aspectRatio, c * radius);
}

/*
Generates vertices and indices for a small umbrella geometry:
   6 ----- 1
 /   \   /   \
5 ---- 0 ---- 2
 \   /   \   /
   4 ----- 3
*/
[outputtopology("triangle")]
[numthreads(NUM_VERTICES, 1, 1)]
void MSMain(
    uint groupThreadId : SV_GroupThreadID,
    uint groupId : SV_GroupID,
    out indices uint3 triangles[NUM_TRIANGLES],
    out vertices VertexOut vertices[NUM_VERTICES])
{
    SetMeshOutputCounts(NUM_VERTICES, NUM_TRIANGLES);

    const float3 colors[NUM_COLORS] =
    {
        float3(1, 0, 0),
        float3(0, 1, 0),
        float3(0, 0, 1),
        float3(1, 1, 0),
        float3(0, 1, 1),
        float3(1, 0, 1),
    };

    const float radius = 0.5;
    float angle = (float)(groupThreadId + groupId * NUM_TRIANGLES) / (float)(NUM_TRIANGLES * numMeshlets);
    float2 coord = (groupThreadId == 0 ? float2(0, 0) : GetPointOnCircle(angle, radius));

    vertices[groupThreadId].position = float4(coord, 0, 1);
    vertices[groupThreadId].color    = colors[groupThreadId % NUM_COLORS];

    if (groupThreadId < NUM_TRIANGLES)
    {
        triangles[groupThreadId] = uint3(
            0,
            groupThreadId + 1,
            groupThreadId + 2
        );
    }
}

float4 PSMain(VertexOut inp) : SV_Target
{
    return float4(inp.color, 1);
}
