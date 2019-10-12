/*
 * Metal cloth physics shader
 */

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SceneState
{
    float4x4    wvpMatrix;
    float4x4    wMatrix;
    float4      gravity;
    uint2       gridSize;
    uint2       _pad0;
    float       damping;
    float       dTime;
    float       dStiffness;
    float       _pad1;
    float4      lightVec;
};

// Sub view of a particle
struct ParticleView
{
    float4 currPos;
    float4 nextPos;
    float4 origPos;
    float4 normal;
    float invMass;
};

// Returns the particle index for the specified grid
uint GridPosToIndex(uint2 gridPos, uint gridWidth)
{
    return (gridPos.y * gridWidth + gridPos.x);
}

// Converts the specified grid UV coordinates to the original vertex coordinates.
// Only distance between those coordinates are important.
float4 UVToOrigPos(float2 uv)
{
    return float4(uv.x * 2.0 - 1.0, 0.0, uv.y * -2.0, 1.0);
}

void AccumulateStretchConstraints(
    thread const ParticleView&  par,
    device float4*              parCurrPos,
    device float4*              parBase,
    int2                        neighborGridPos,
    uint2                       gridSize,
    thread float3&              dCorrection)
{
    if (neighborGridPos.x < 0 || (uint)neighborGridPos.x >= gridSize.x ||
        neighborGridPos.y < 0 || (uint)neighborGridPos.y >= gridSize.y)
    {
        return;
    }

    // Read neighbor particle
    uint idx = GridPosToIndex((uint2)neighborGridPos, gridSize.x);
    float4 otherCurrPos = parCurrPos[idx];
    float4 otherOrigPos = UVToOrigPos(parBase[idx].xy);
    float otherInvMass = parBase[idx].z;

    // Compute edge distance between particle and its neighbor
    float3 dPos = (par.nextPos - otherCurrPos).xyz;
    float currDist = length(dPos);
    float edgeDist = distance(par.origPos, otherOrigPos);

    // Compute stretch constraint
    dPos = normalize(dPos) * ((currDist - edgeDist) / (par.invMass + otherInvMass));

    // Adjust position
    dCorrection += (dPos * -par.invMass);
}

float3 ReadParticlePos(
    device float4*  parCurrPos,
    uint2           gridPos,
    uint            gridWidth)
{
    return parCurrPos[GridPosToIndex(gridPos, gridWidth)].xyz;
}

void AccumulateSurfaceNormal(
    float4          pos,
    device float4*  parCurrPos,
    int2            gridPos0,
    int2            gridPos1,
    uint2           gridSize,
    thread float4&  normal)
{
    if (gridPos0.x < 0 || (uint)gridPos0.x >= gridSize.x ||
        gridPos0.y < 0 || (uint)gridPos0.y >= gridSize.y ||
        gridPos1.x < 0 || (uint)gridPos1.x >= gridSize.x ||
        gridPos1.y < 0 || (uint)gridPos1.y >= gridSize.y)
    {
        return;
    }

    float3 v0 = ReadParticlePos(parCurrPos, (uint2)gridPos0, gridSize.x) - pos.xyz;
    float3 v1 = ReadParticlePos(parCurrPos, (uint2)gridPos1, gridSize.x) - pos.xyz;

    normal.xyz += cross(v0, v1);
}

void ApplyStretchConstraints(
    thread ParticleView&    par,
    device float4*          parCurrPos,
    device float4*          parBase,
    constant SceneState&    sceneState,
    int2                    gridPos)
{
    if (par.invMass == 0.0)
    {
        return;
    }

    // Apply stretch constraints
    float3 dPos = (float3)0;
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2( 0, -1), sceneState.gridSize, dPos);
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2( 0, +1), sceneState.gridSize, dPos);
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2(-1,  0), sceneState.gridSize, dPos);
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2(+1,  0), sceneState.gridSize, dPos);
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2(-1, -1), sceneState.gridSize, dPos);
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2(+1, -1), sceneState.gridSize, dPos);
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2(-1, +1), sceneState.gridSize, dPos);
    AccumulateStretchConstraints(par, parCurrPos, parBase, gridPos + int2(+1, +1), sceneState.gridSize, dPos);
    dPos /= 8.0;

    // Compute normal
    float4 normal = (float4)0;
    AccumulateSurfaceNormal(par.currPos, parCurrPos, gridPos + int2( 0, +1), gridPos + int2(+1,  0), sceneState.gridSize, normal);
    AccumulateSurfaceNormal(par.currPos, parCurrPos, gridPos + int2(+1,  0), gridPos + int2( 0, -1), sceneState.gridSize, normal);
    AccumulateSurfaceNormal(par.currPos, parCurrPos, gridPos + int2( 0, -1), gridPos + int2(-1,  0), sceneState.gridSize, normal);
    AccumulateSurfaceNormal(par.currPos, parCurrPos, gridPos + int2(-1,  0), gridPos + int2( 0, +1), sceneState.gridSize, normal);
    par.normal = normal / 4.0;

    // Adjust position by correction vector
    par.nextPos.xyz += dPos * sceneState.dStiffness;
}

kernel void CSForces(
    uint2                   threadID    [[thread_position_in_grid]],
    constant SceneState&    sceneState  [[buffer(0)]],
    device float4*          parBase     [[buffer(1)]], // UV (.xy) and inverse mass (.z)
    device float4*          parCurrPos  [[buffer(2)]],
    device float4*          parVelocity [[buffer(5)]])
{
    uint idx = GridPosToIndex(threadID, sceneState.gridSize.x);

    // Accumulate force and multiply by inverse mass
    float invMass = parBase[idx].z;
    float4 force = sceneState.gravity;
    force *= invMass;

    // Apply velocity and damping
    parVelocity[idx] += force * sceneState.dTime * sceneState.damping;

    // Apply position based physics simulation
    parCurrPos[idx] += float4(parVelocity[idx].xyz, 0.0) * sceneState.dTime;
}

kernel void CSStretchConstraints(
    uint2                   threadID    [[thread_position_in_grid]],
    constant SceneState&    sceneState  [[buffer(0)]],
    device float4*          parBase     [[buffer(1)]], // UV (.xy) and inverse mass (.z)
    device float4*          parCurrPos  [[buffer(2)]],
    device float4*          parNextPos  [[buffer(3)]],
    device float4*          parNormal   [[buffer(6)]])
{
    uint idx = GridPosToIndex(threadID, sceneState.gridSize.x);

    // Read particle
    ParticleView par;
    par.currPos = parCurrPos[idx];
    par.nextPos = par.currPos;
    par.origPos = UVToOrigPos(parBase[idx].xy);
    par.normal  = parNormal[idx];
    par.invMass = parBase[idx].z;

    // Apply stretch constraints
    ApplyStretchConstraints(par, parCurrPos, parBase, sceneState, (int2)threadID);

    // Write next position back to swap-buffer
    parNextPos[idx] = par.nextPos;
    parNormal[idx] = par.normal;
}

kernel void CSRelaxation(
    uint2                   threadID    [[thread_position_in_grid]],
    constant SceneState&    sceneState  [[buffer(0)]],
    device float4*          parCurrPos  [[buffer(2)]],
    device float4*          parNextPos  [[buffer(3)]],
    device float4*          parPrevPos  [[buffer(4)]],
    device float4*          parVelocity [[buffer(5)]])
{
    uint idx = GridPosToIndex(threadID, sceneState.gridSize.x);

    // Adjust velocity and store current and previous position
    parCurrPos[idx] = parNextPos[idx];
    parVelocity[idx] = (parCurrPos[idx] - parPrevPos[idx]) / sceneState.dTime;
    parPrevPos[idx] = parCurrPos[idx];
}


/*
 * Metal vertex shader
 */

struct VIn
{
    float4 position [[attribute(0)]];
    float4 normal   [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VOut
{
    float4 position [[position]];
    float4 normal;
    float2 texCoord;
};

vertex VOut VS(
    VIn                     inp         [[stage_in]],
    constant SceneState&    sceneState  [[buffer(3)]])
{
    VOut outp;
    outp.position   = sceneState.wvpMatrix * inp.position;
    outp.normal     = sceneState.wMatrix * inp.normal;
    outp.texCoord   = inp.texCoord;
    return outp;
}


/*
 * Metal fragment shader
 */

fragment float4 PS(
    VOut                    inp             [[stage_in]],
    constant SceneState&    sceneState      [[buffer(3)]],
    texture2d<float>        colorMap        [[texture(4)]],
    sampler                 linearSampler   [[sampler(5)]],
    bool                    frontFace       [[front_facing]])
{
    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    normal *= select(1.0, -1.0, frontFace);

    float NdotL = mix(0.2, 1.0, max(0.0, dot(normal, -sceneState.lightVec.xyz)));

    // Sample color texture
    float4 color = colorMap.sample(linearSampler, inp.texCoord);

    color.rgb = mix(color.rgb, float3(inp.texCoord, 1.0), 0.5);

    return float4(color.rgb * NdotL, color.a);
}

