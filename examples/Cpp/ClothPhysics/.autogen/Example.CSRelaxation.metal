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

kernel void CSRelaxation(constant type_SceneState& SceneState [[buffer(0)]], texture2d<float, access::read_write> parCurrPos [[texture(2)]], texture2d<float> parNextPos [[texture(3)]], texture2d<float, access::read_write> parPrevPos [[texture(4)]], texture2d<float, access::write> parVelocity [[texture(5)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]])
{
    uint _34 = (gl_GlobalInvocationID.y * SceneState.gridSize.x) + gl_GlobalInvocationID.x;
    parCurrPos.write(parNextPos.read(spvTexelBufferCoord(_34)), spvTexelBufferCoord(_34));
    spvImageFence(parCurrPos);
    spvImageFence(parPrevPos);
    parVelocity.write((parCurrPos.read(spvTexelBufferCoord(_34)) - parPrevPos.read(spvTexelBufferCoord(_34))) / float4(SceneState.dTime), spvTexelBufferCoord(_34));
    spvImageFence(parCurrPos);
    parPrevPos.write(parCurrPos.read(spvTexelBufferCoord(_34)), spvTexelBufferCoord(_34));
}

