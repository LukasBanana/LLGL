/*
 * HLSL cloth physics shader
 */

#define COMPUTE_MODE_RWBUFFERS      0
#define COMPUTE_MODE_RWTEXTURES     1
#define COMPUTE_MODE_STREAMOUTPUT   2

#ifndef COMPUTE_MODE
#define COMPUTE_MODE COMPUTE_MODE_RWBUFFERS
#endif

#if __spirv__ && !GL_ES

#   define VK_XFB_OFFSET 35
#   define VK_XFB_BUFFER 36
#   define VK_XFB_STRIDE 37
#   define STREAM_OUTPUT(INDEX)                     \
        [[vk::location(INDEX)]]                     \
        [[vk::ext_decorate(VK_XFB_BUFFER, INDEX)]]  \
        [[vk::ext_decorate(VK_XFB_OFFSET, 0)]]      \
        [[vk::ext_decorate(VK_XFB_STRIDE, 16)]]

#   define VK_CAPABILITY_TRANSFORM_FEEDBACK 53
#   define VK_EXECUTION_MODE_XFB            11
#   define EXECUTION_MODE_XFB                                       \
        [[vk::ext_capability(VK_CAPABILITY_TRANSFORM_FEEDBACK)]]    \
        vk::ext_execution_mode(VK_EXECUTION_MODE_XFB)

#else

#   define STREAM_OUTPUT(INDEX)
#   define EXECUTION_MODE_XFB

#endif

// Use bindig slot 3 for this cbuffer. The cross-compiled Metal shader uses the same slot
// and reserves the same binding space for its 3 vertex buffers (binding slots 0-2).
cbuffer SceneState : register(b7)
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

// Particle buffers
#if COMPUTE_MODE == COMPUTE_MODE_RWTEXTURES

Texture2D<float4>   parBase     : register(t1); // UV (.xy) and inverse mass (.z)
RWTexture2D<float4> parCurrPos  : register(u2);
RWTexture2D<float4> parNextPos  : register(u3);
RWTexture2D<float4> parPrevPos  : register(u4);
RWTexture2D<float4> parVelocity : register(u5);
RWTexture2D<float4> parNormal   : register(u6);

#define DECL_GRIDPOS_INDEX(IDX, POS) uint2 IDX = (uint2)(POS)

#else

#   if COMPUTE_MODE == COMPUTE_MODE_RWBUFFERS

Buffer<float4>      parBase     : register(t1); // UV (.xy) and inverse mass (.z)
RWBuffer<float4>    parCurrPos  : register(u2);
RWBuffer<float4>    parNextPos  : register(u3);
RWBuffer<float4>    parPrevPos  : register(u4);
RWBuffer<float4>    parVelocity : register(u5);
RWBuffer<float4>    parNormal   : register(u6);

#   else

Buffer<float4> parBase     : register(t1);
Buffer<float4> parCurrPos  : register(t2);
Buffer<float4> parNextPos  : register(t3);
Buffer<float4> parPrevPos  : register(t4);
Buffer<float4> parVelocity : register(t5);
Buffer<float4> parNormal   : register(t6);

#   endif

// Returns the particle index for the specified grid
uint GridPosToIndex(uint2 gridPos)
{
    return (gridPos.y * gridSize.x + gridPos.x);
}

#define DECL_GRIDPOS_INDEX(IDX, POS) uint IDX = GridPosToIndex((uint2)(POS))

#endif // /ENABLE_STORAGE_TEXTURES

// Converts the specified grid UV coordinates to the original vertex coordinates.
// Only distance between those coordinates are important.
float4 UVToOrigPos(float2 uv)
{
    return float4(uv.x * 2.0 - 1.0, 0.0, uv.y * -2.0, 1.0);
}

void AccumulateStretchConstraints(ParticleView par, int2 neighborGridPos, inout float3 dCorrection)
{
    if (neighborGridPos.x < 0 || (uint)neighborGridPos.x >= gridSize.x ||
        neighborGridPos.y < 0 || (uint)neighborGridPos.y >= gridSize.y)
    {
        return;
    }

    // Read neighbor particle
    DECL_GRIDPOS_INDEX(idx, neighborGridPos);

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

float3 ReadParticlePos(uint2 gridPos)
{
    DECL_GRIDPOS_INDEX(idx, gridPos);
    return parCurrPos[idx].xyz;
}

void AccumulateSurfaceNormal(float4 pos, int2 gridPos0, int2 gridPos1, inout float4 normal)
{
    if (gridPos0.x < 0 || (uint)gridPos0.x >= gridSize.x ||
        gridPos0.y < 0 || (uint)gridPos0.y >= gridSize.y ||
        gridPos1.x < 0 || (uint)gridPos1.x >= gridSize.x ||
        gridPos1.y < 0 || (uint)gridPos1.y >= gridSize.y)
    {
        return;
    }

    float3 v0 = ReadParticlePos((uint2)gridPos0) - pos.xyz;
    float3 v1 = ReadParticlePos((uint2)gridPos1) - pos.xyz;

    normal.xyz += cross(v0, v1);
}

void ApplyStretchConstraints(inout ParticleView par, int2 gridPos)
{
    if (par.invMass == 0.0)
    {
        return;
    }

    // Apply stretch constraints
    float3 dPos = (float3)0;
    AccumulateStretchConstraints(par, gridPos + int2( 0, -1), dPos);
    AccumulateStretchConstraints(par, gridPos + int2( 0, +1), dPos);
    AccumulateStretchConstraints(par, gridPos + int2(-1,  0), dPos);
    AccumulateStretchConstraints(par, gridPos + int2(+1,  0), dPos);
    AccumulateStretchConstraints(par, gridPos + int2(-1, -1), dPos);
    AccumulateStretchConstraints(par, gridPos + int2(+1, -1), dPos);
    AccumulateStretchConstraints(par, gridPos + int2(-1, +1), dPos);
    AccumulateStretchConstraints(par, gridPos + int2(+1, +1), dPos);
    dPos /= 8.0;

    // Compute normal
    float4 normal = (float4)0;
    AccumulateSurfaceNormal(par.currPos, gridPos + int2( 0, +1), gridPos + int2(+1,  0), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2(+1,  0), gridPos + int2( 0, -1), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2( 0, -1), gridPos + int2(-1,  0), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2(-1,  0), gridPos + int2( 0, +1), normal);
    par.normal = normal / 4.0;

    // Adjust position by correction vector
    par.nextPos.xyz += dPos * dStiffness;
}

#if COMPUTE_MODE == COMPUTE_MODE_STREAMOUTPUT

struct ForcesOut
{
    STREAM_OUTPUT(0) float4 currPos  : CURRPOS;
};

struct StretchConstraintsOut
{
    STREAM_OUTPUT(0) float4 nextPos : NEXTPOS;
    STREAM_OUTPUT(1) float4 normal  : NORMAL;
};

struct RelaxationOut
{
    STREAM_OUTPUT(0) float4 currPos  : CURRPOS;
    STREAM_OUTPUT(1) float4 velocity : VELOCITY;
};

void VSForces(uint id : SV_VertexID, out ForcesOut outp)
{
    EXECUTION_MODE_XFB;

    float invMass = parBase[id].z;
    float4 force = mul(gravity, wMatrix);
    force *= invMass;

    float4 velocity = parVelocity[id] + force * dTime * damping;
    outp.currPos = parCurrPos[id] + float4(velocity.xyz, 0.0) * dTime;
}

void VSStretchConstraints(uint id : SV_VertexID, out StretchConstraintsOut outp)
{
    EXECUTION_MODE_XFB;

    uint2 gridPos = uint2(id % gridSize.x, id / gridSize.x);

    ParticleView par;
    par.currPos = parCurrPos[id];
    par.nextPos = par.currPos;
    par.origPos = UVToOrigPos(parBase[id].xy);
    par.normal  = parNormal[id];
    par.invMass = parBase[id].z;

    ApplyStretchConstraints(par, (int2)gridPos);

    outp.nextPos = par.nextPos;
    outp.normal = par.normal;
}

void VSRelaxation(uint id : SV_VertexID, out RelaxationOut outp)
{
    EXECUTION_MODE_XFB;

    outp.currPos = parNextPos[id];
    outp.velocity = (outp.currPos - parPrevPos[id]) / dTime;
}

#else // COMPUTE_MODE

[numthreads(1, 1, 1)]
void CSForces(uint2 threadID : SV_DispatchThreadID)
{
    DECL_GRIDPOS_INDEX(idx, threadID);

    // Accumulate force and multiply by inverse mass
    float invMass = parBase[idx].z;
    float4 force = mul(gravity, wMatrix);
    force *= invMass;

    // Apply velocity and damping
    float4 velocity = parVelocity[idx] + force * dTime * damping;

    // Apply position based physics simulation
    parCurrPos[idx] += float4(velocity.xyz, 0.0) * dTime;
}

[numthreads(1, 1, 1)]
void CSStretchConstraints(uint2 threadID : SV_DispatchThreadID)
{
    DECL_GRIDPOS_INDEX(idx, threadID);

    // Read particle
    ParticleView par;
    par.currPos = parCurrPos[idx];
    par.nextPos = par.currPos;
    par.origPos = UVToOrigPos(parBase[idx].xy);
    par.normal  = parNormal[idx];
    par.invMass = parBase[idx].z;

    // Apply stretch constraints
    ApplyStretchConstraints(par, (int2)threadID);

    // Write next position back to swap-buffer
    parNextPos[idx] = par.nextPos;
    parNormal[idx] = par.normal;
}

[numthreads(1, 1, 1)]
void CSRelaxation(uint2 threadID : SV_DispatchThreadID)
{
    DECL_GRIDPOS_INDEX(idx, threadID);

    // Adjust velocity and store current and previous position
    parCurrPos[idx] = parNextPos[idx];
    parVelocity[idx] = (parCurrPos[idx] - parPrevPos[idx]) / dTime;
    parPrevPos[idx] = parCurrPos[idx];
}

#endif // /COMPUTE_MODE


/*
 * HLSL vertex shader
 */

struct VOut
{
    float4 position : SV_Position;
    float4 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

#if ENABLE_STORAGE_TEXTURES

Texture2D<float4> vertexBase    : register(t1); // UV (.xy) and inverse mass (.z)
Texture2D<float4> vertexPos     : register(t2);
Texture2D<float4> vertexNormal  : register(t6);

void VS(uint id : SV_VertexID, out VOut outp)
{
    uint2 idx = uint2(id % gridSize.x, id / gridSize.x);
    outp.position   = mul(wvpMatrix, vertexPos[idx]);
    outp.normal     = mul(wMatrix, vertexNormal[idx]);
    outp.texCoord   = vertexBase[idx].xy;
}

#else // ENABLE_STORAGE_TEXTURES

struct VIn
{
    float4 position : POS;
    float4 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

void VS(in VIn inp, out VOut outp)
{
    outp.position   = mul(wvpMatrix, inp.position);
    outp.normal     = mul(wMatrix, inp.normal);
    outp.texCoord   = inp.texCoord;
}

#endif // /ENABLE_STORAGE_TEXTURES


/*
 * HLSL pixel shader
 */

Texture2D colorMap : register(t4);
SamplerState linearSampler : register(s5);

float4 PS(in VOut inp, bool frontFace : SV_IsFrontFace) : SV_Target0
{
    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    normal *= lerp(1.0, -1.0, frontFace);

    float NdotL = lerp(0.2, 1.0, max(0.0, dot(normal, -lightVec.xyz)));

    // Sample color texture
    float4 color = colorMap.Sample(linearSampler, inp.texCoord);

    color.rgb = lerp(color.rgb, float3(inp.texCoord, 1.0), 0.5);

    return float4(color.rgb * NdotL, color.a);
}

