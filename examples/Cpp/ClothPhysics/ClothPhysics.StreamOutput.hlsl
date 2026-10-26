
#if __spirv__ && !IS_TARGET_300ES

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

struct ParticleView
{
    float4 currPos;
    float4 nextPos;
    float4 origPos;
    float4 normal;
    float invMass;
};

Buffer<float4> parBase     : register(t1);
Buffer<float4> parCurrPos  : register(t2);
Buffer<float4> parNextPos  : register(t3);
Buffer<float4> parPrevPos  : register(t4);
Buffer<float4> parVelocity : register(t5);
Buffer<float4> parNormal   : register(t6);

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

uint GridPosToIndex(uint2 gridPos)
{
    return (gridPos.y * gridSize.x + gridPos.x);
}

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

    uint idx = GridPosToIndex((uint2)neighborGridPos);

    float4 otherCurrPos = parCurrPos[idx];
    float4 otherOrigPos = UVToOrigPos(parBase[idx].xy);
    float otherInvMass = parBase[idx].z;

    float3 dPos = (par.nextPos - otherCurrPos).xyz;
    float currDist = length(dPos);
    float edgeDist = distance(par.origPos, otherOrigPos);

    dPos = normalize(dPos) * ((currDist - edgeDist) / (par.invMass + otherInvMass));

    dCorrection += (dPos * -par.invMass);
}

float3 ReadParticlePos(uint2 gridPos)
{
    return parCurrPos[GridPosToIndex(gridPos)].xyz;
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

    float4 normal = (float4)0;
    AccumulateSurfaceNormal(par.currPos, gridPos + int2( 0, +1), gridPos + int2(+1,  0), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2(+1,  0), gridPos + int2( 0, -1), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2( 0, -1), gridPos + int2(-1,  0), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2(-1,  0), gridPos + int2( 0, +1), normal);
    par.normal = normal / 4.0;

    par.nextPos.xyz += dPos * dStiffness;
}

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

struct VOut
{
    float4 position : SV_Position;
    float4 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

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

Texture2D colorMap : register(t4);
SamplerState linearSampler : register(s5);

float4 PS(in VOut inp, bool frontFace : SV_IsFrontFace) : SV_Target0
{
    float3 normal = normalize(inp.normal.xyz);
    normal *= lerp(1.0, -1.0, frontFace);

    float NdotL = lerp(0.2, 1.0, max(0.0, dot(normal, -lightVec.xyz)));

    float4 color = colorMap.Sample(linearSampler, inp.texCoord);

    color.rgb = lerp(color.rgb, float3(inp.texCoord, 1.0), 0.5);

    return float4(color.rgb * NdotL, color.a);
}