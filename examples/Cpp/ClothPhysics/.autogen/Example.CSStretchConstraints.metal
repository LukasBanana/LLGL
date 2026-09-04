#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Returns 2D texture coords corresponding to 1D texel buffer coords
static inline __attribute__((always_inline))
uint2 spvTexelBufferCoord(uint tc)
{
    return uint2(tc % 4096, tc / 4096);
}

template <typename ImageT>
void spvImageFence(ImageT img) { img.fence(); }

struct type_SceneState
{
    float4x4 wvpMatrix;
    float4x4 wMatrix;
    float4 gravity;
    uint2 gridSize;
    uint2 _pad0;
    float damping;
    float dTime;
    float dStiffness;
    float _pad1;
    float4 lightVec;
};

struct ParticleView
{
    float4 currPos;
    float4 nextPos;
    float4 origPos;
    float4 normal;
    float invMass;
};

static inline __attribute__((always_inline))
uint GridPosToIndex(thread const uint2& gridPos, constant type_SceneState& SceneState)
{
    return (gridPos.y * SceneState.gridSize.x) + gridPos.x;
}

static inline __attribute__((always_inline))
float4 UVToOrigPos(thread const float2& uv)
{
    return float4((uv.x * 2.0) - 1.0, 0.0, uv.y * (-2.0), 1.0);
}

static inline __attribute__((always_inline))
void AccumulateStretchConstraints(thread const ParticleView& par, thread const int2& neighborGridPos, thread float3& dCorrection, constant type_SceneState& SceneState, texture2d<float> parBase, texture2d<float> parCurrPos)
{
    bool temp_var_logical = true;
    if (!(neighborGridPos.x < 0))
    {
        temp_var_logical = uint(neighborGridPos.x) >= SceneState.gridSize.x;
    }
    bool temp_var_logical_1 = true;
    if (!temp_var_logical)
    {
        temp_var_logical_1 = neighborGridPos.y < 0;
    }
    bool temp_var_logical_2 = true;
    if (!temp_var_logical_1)
    {
        temp_var_logical_2 = uint(neighborGridPos.y) >= SceneState.gridSize.y;
    }
    if (temp_var_logical_2)
    {
        return;
    }
    uint2 param_var_gridPos = uint2(neighborGridPos);
    uint idx = GridPosToIndex(param_var_gridPos, SceneState);
    float4 otherCurrPos = parCurrPos.read(spvTexelBufferCoord(idx));
    float2 param_var_uv = parBase.read(spvTexelBufferCoord(idx)).xy;
    float4 otherOrigPos = UVToOrigPos(param_var_uv);
    float otherInvMass = parBase.read(spvTexelBufferCoord(idx)).z;
    float3 dPos = (par.nextPos - otherCurrPos).xyz;
    float currDist = length(dPos);
    float edgeDist = distance(par.origPos, otherOrigPos);
    dPos = fast::normalize(dPos) * ((currDist - edgeDist) / (par.invMass + otherInvMass));
    dCorrection += (dPos * (-par.invMass));
}

static inline __attribute__((always_inline))
float3 ReadParticlePos(thread const uint2& gridPos, constant type_SceneState& SceneState, texture2d<float> parCurrPos)
{
    uint2 param_var_gridPos = gridPos;
    return parCurrPos.read(spvTexelBufferCoord(GridPosToIndex(param_var_gridPos, SceneState))).xyz;
}

static inline __attribute__((always_inline))
void AccumulateSurfaceNormal(thread const float4& pos, thread const int2& gridPos0, thread const int2& gridPos1, thread float4& normal, constant type_SceneState& SceneState, texture2d<float> parCurrPos)
{
    bool temp_var_logical = true;
    if (!(gridPos0.x < 0))
    {
        temp_var_logical = uint(gridPos0.x) >= SceneState.gridSize.x;
    }
    bool temp_var_logical_1 = true;
    if (!temp_var_logical)
    {
        temp_var_logical_1 = gridPos0.y < 0;
    }
    bool temp_var_logical_2 = true;
    if (!temp_var_logical_1)
    {
        temp_var_logical_2 = uint(gridPos0.y) >= SceneState.gridSize.y;
    }
    bool temp_var_logical_3 = true;
    if (!temp_var_logical_2)
    {
        temp_var_logical_3 = gridPos1.x < 0;
    }
    bool temp_var_logical_4 = true;
    if (!temp_var_logical_3)
    {
        temp_var_logical_4 = uint(gridPos1.x) >= SceneState.gridSize.x;
    }
    bool temp_var_logical_5 = true;
    if (!temp_var_logical_4)
    {
        temp_var_logical_5 = gridPos1.y < 0;
    }
    bool temp_var_logical_6 = true;
    if (!temp_var_logical_5)
    {
        temp_var_logical_6 = uint(gridPos1.y) >= SceneState.gridSize.y;
    }
    if (temp_var_logical_6)
    {
        return;
    }
    uint2 param_var_gridPos = uint2(gridPos0);
    float3 v0 = ReadParticlePos(param_var_gridPos, SceneState, parCurrPos) - pos.xyz;
    uint2 param_var_gridPos_1 = uint2(gridPos1);
    float3 v1 = ReadParticlePos(param_var_gridPos_1, SceneState, parCurrPos) - pos.xyz;
    float3 _471 = normal.xyz + cross(v0, v1);
    normal = float4(_471.x, _471.y, _471.z, normal.w);
}

static inline __attribute__((always_inline))
void ApplyStretchConstraints(thread ParticleView& par, thread const int2& gridPos, constant type_SceneState& SceneState, texture2d<float> parBase, texture2d<float> parCurrPos)
{
    if (par.invMass == 0.0)
    {
        return;
    }
    float3 dPos = float3(0.0);
    ParticleView param_var_par = par;
    int2 param_var_neighborGridPos = gridPos + int2(0, -1);
    AccumulateStretchConstraints(param_var_par, param_var_neighborGridPos, dPos, SceneState, parBase, parCurrPos);
    ParticleView param_var_par_1 = par;
    int2 param_var_neighborGridPos_1 = gridPos + int2(0, 1);
    AccumulateStretchConstraints(param_var_par_1, param_var_neighborGridPos_1, dPos, SceneState, parBase, parCurrPos);
    ParticleView param_var_par_2 = par;
    int2 param_var_neighborGridPos_2 = gridPos + int2(-1, 0);
    AccumulateStretchConstraints(param_var_par_2, param_var_neighborGridPos_2, dPos, SceneState, parBase, parCurrPos);
    ParticleView param_var_par_3 = par;
    int2 param_var_neighborGridPos_3 = gridPos + int2(1, 0);
    AccumulateStretchConstraints(param_var_par_3, param_var_neighborGridPos_3, dPos, SceneState, parBase, parCurrPos);
    ParticleView param_var_par_4 = par;
    int2 param_var_neighborGridPos_4 = gridPos + int2(-1);
    AccumulateStretchConstraints(param_var_par_4, param_var_neighborGridPos_4, dPos, SceneState, parBase, parCurrPos);
    ParticleView param_var_par_5 = par;
    int2 param_var_neighborGridPos_5 = gridPos + int2(1, -1);
    AccumulateStretchConstraints(param_var_par_5, param_var_neighborGridPos_5, dPos, SceneState, parBase, parCurrPos);
    ParticleView param_var_par_6 = par;
    int2 param_var_neighborGridPos_6 = gridPos + int2(-1, 1);
    AccumulateStretchConstraints(param_var_par_6, param_var_neighborGridPos_6, dPos, SceneState, parBase, parCurrPos);
    ParticleView param_var_par_7 = par;
    int2 param_var_neighborGridPos_7 = gridPos + int2(1);
    AccumulateStretchConstraints(param_var_par_7, param_var_neighborGridPos_7, dPos, SceneState, parBase, parCurrPos);
    dPos /= float3(8.0);
    float4 normal = float4(0.0);
    float4 param_var_pos = par.currPos;
    int2 param_var_gridPos0 = gridPos + int2(0, 1);
    int2 param_var_gridPos1 = gridPos + int2(1, 0);
    AccumulateSurfaceNormal(param_var_pos, param_var_gridPos0, param_var_gridPos1, normal, SceneState, parCurrPos);
    float4 param_var_pos_1 = par.currPos;
    int2 param_var_gridPos0_1 = gridPos + int2(1, 0);
    int2 param_var_gridPos1_1 = gridPos + int2(0, -1);
    AccumulateSurfaceNormal(param_var_pos_1, param_var_gridPos0_1, param_var_gridPos1_1, normal, SceneState, parCurrPos);
    float4 param_var_pos_2 = par.currPos;
    int2 param_var_gridPos0_2 = gridPos + int2(0, -1);
    int2 param_var_gridPos1_2 = gridPos + int2(-1, 0);
    AccumulateSurfaceNormal(param_var_pos_2, param_var_gridPos0_2, param_var_gridPos1_2, normal, SceneState, parCurrPos);
    float4 param_var_pos_3 = par.currPos;
    int2 param_var_gridPos0_3 = gridPos + int2(-1, 0);
    int2 param_var_gridPos1_3 = gridPos + int2(0, 1);
    AccumulateSurfaceNormal(param_var_pos_3, param_var_gridPos0_3, param_var_gridPos1_3, normal, SceneState, parCurrPos);
    par.normal = normal / float4(4.0);
    float3 _265 = par.nextPos.xyz + (dPos * SceneState.dStiffness);
    par.nextPos = float4(_265.x, _265.y, _265.z, par.nextPos.w);
}

static inline __attribute__((always_inline))
void src_CSStretchConstraints(thread const uint2& threadID, constant type_SceneState& SceneState, texture2d<float> parBase, texture2d<float> parCurrPos, texture2d<float, access::write> parNextPos, texture2d<float, access::read_write> parNormal)
{
    uint2 param_var_gridPos = threadID;
    uint idx = GridPosToIndex(param_var_gridPos, SceneState);
    ParticleView par;
    par.currPos = parCurrPos.read(spvTexelBufferCoord(idx));
    par.nextPos = par.currPos;
    float2 param_var_uv = parBase.read(spvTexelBufferCoord(idx)).xy;
    par.origPos = UVToOrigPos(param_var_uv);
    spvImageFence(parNormal);
    par.normal = parNormal.read(spvTexelBufferCoord(idx));
    par.invMass = parBase.read(spvTexelBufferCoord(idx)).z;
    int2 param_var_gridPos_1 = int2(threadID);
    ApplyStretchConstraints(par, param_var_gridPos_1, SceneState, parBase, parCurrPos);
    parNextPos.write(par.nextPos, spvTexelBufferCoord(idx));
    parNormal.write(par.normal, spvTexelBufferCoord(idx));
}

kernel void CSStretchConstraints(constant type_SceneState& SceneState [[buffer(0)]], texture2d<float> parBase [[texture(1)]], texture2d<float> parCurrPos [[texture(2)]], texture2d<float, access::write> parNextPos [[texture(3)]], texture2d<float, access::read_write> parNormal [[texture(6)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]])
{
    uint2 param_var_threadID = gl_GlobalInvocationID.xy;
    src_CSStretchConstraints(param_var_threadID, SceneState, parBase, parCurrPos, parNextPos, parNormal);
}

