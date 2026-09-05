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

struct type_SceneState
{
    float time;
    uint numSceneObjects;
    float aspectRatio;
};

kernel void CS(constant type_SceneState& SceneState [[buffer(2)]], texture2d<float, access::write> sceneObjects [[texture(3)]], texture2d<uint, access::write> drawArgs [[texture(4)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]])
{
    if (gl_GlobalInvocationID.x == 0u)
    {
        float _48 = fract(SceneState.time * 0.100000001490116119384765625);
        if (_48 < 0.5)
        {
            uint _58 = uint(mix(0.0, float(SceneState.numSceneObjects), _48 * 2.0));
            drawArgs.write(uint4(3u, _58, 0u, 0u), spvTexelBufferCoord(0u));
            drawArgs.write(uint4(4u, SceneState.numSceneObjects - _58, 3u, _58), spvTexelBufferCoord(1u));
        }
        else
        {
            uint _70 = uint(mix(0.0, float(SceneState.numSceneObjects), (_48 - 0.5) * 2.0));
            drawArgs.write(uint4(4u, _70, 3u, 0u), spvTexelBufferCoord(1u));
            drawArgs.write(uint4(3u, SceneState.numSceneObjects - _70, 0u, _70), spvTexelBufferCoord(0u));
        }
    }
    float _85 = fract((SceneState.time * 0.100000001490116119384765625) + (float(gl_GlobalInvocationID.x) / float(SceneState.numSceneObjects)));
    float _86 = _85 * 6.283185482025146484375;
    float _90 = (cos(_85 * 31.415927886962890625) * 0.5) + 0.5;
    float _91 = mix(0.5, 0.5, _90);
    float _92 = mix(0.100000001490116119384765625, 0.100000001490116119384765625, _90);
    float _93 = _85 * 12.56637096405029296875;
    float _94 = sin(_93);
    uint _96 = gl_GlobalInvocationID.x * 2u;
    float _97 = cos(_93) * _92;
    sceneObjects.write(float4(_97, _94 * _92, (-_94) * _92, _97), spvTexelBufferCoord(_96));
    sceneObjects.write(float4(sin(_86) * _91, cos(_86) * _91, 0.0, 0.0), spvTexelBufferCoord((_96 + 1u)));
}

