/*
 * Metal compute shader
 */

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#define MIN_DIST    ( 0.5 )
#define MAX_DIST    ( 0.5 )
#define MIN_RADIUS  ( 0.1 )
#define MAX_RADIUS  ( 0.1 )
#define M_PI        ( 3.141592654 )

struct SceneState
{
    float   time;
    uint    numSceneObjects;
};

struct SceneObject
{
    float2x2    rotation;
    float2      position;
    float2      _pad0;
};

struct DrawIndirectArguments
{
    uint numVertices;
    uint numInstances;
    uint firstVertex;
    uint firstInstance;
};

void WriteDrawArgsTri(device DrawIndirectArguments& args, uint numInstances, uint firstInstance)
{
    args.numVertices    = 3;
    args.numInstances   = numInstances;
    args.firstVertex    = 0;
    args.firstInstance  = firstInstance;
}

void WriteDrawArgsQuad(device DrawIndirectArguments& args, uint numInstances, uint firstInstance)
{
    args.numVertices    = 4;
    args.numInstances   = numInstances;
    args.firstVertex    = 3;
    args.firstInstance  = firstInstance;
}

void WriteSceneObject(device SceneObject& obj, uint idx, constant SceneState& sceneState)
{
    // Compute scene object parameters
    float a = fract(sceneState.time*0.1 + float(idx)/float(sceneState.numSceneObjects))*M_PI*2.0;
    float t = cos(a*5.0)*0.5 + 0.5;
    float d = mix(MIN_DIST, MAX_DIST, t);
    float r = mix(MIN_RADIUS, MAX_RADIUS, t);
    float s = sin(a*2.0);
    float c = cos(a*2.0);

    // Compute rotation and position
    obj.rotation[0][0] = c*r;
    obj.rotation[0][1] = s*r;
    obj.rotation[1][0] = -s*r;
    obj.rotation[1][1] = c*r;
    obj.position[0] = sin(a)*d;
    obj.position[1] = cos(a)*d;
}

kernel void CS(
    constant SceneState&          sceneState   [[buffer(2)]],
    device SceneObject*           sceneObjects [[buffer(3)]],
    device DrawIndirectArguments* drawArgs     [[buffer(4)]],
    uint                          threadID     [[thread_position_in_grid]])
{
    // Write draw arguments to indirect argument buffer
    if (threadID == 0)
    {
        float t = fract(sceneState.time*0.1);
        if (t < 0.5)
        {
            uint n = (uint)mix(0.0, (float)sceneState.numSceneObjects, t * 2.0);
            WriteDrawArgsTri(drawArgs[0], n, 0);
            WriteDrawArgsQuad(drawArgs[1], sceneState.numSceneObjects - n, n);
        }
        else
        {
            uint n = (uint)mix(0.0, (float)sceneState.numSceneObjects, (t - 0.5) * 2.0);
            WriteDrawArgsQuad(drawArgs[1], n, 0);
            WriteDrawArgsTri(drawArgs[0], sceneState.numSceneObjects - n, n);
        }
    }

    // Write scene transformations
    WriteSceneObject(sceneObjects[threadID], threadID, sceneState);
}


/*
 * Metal vertex shader
 */

struct VIn
{
    float2 coord     [[attribute(0)]];
    float4 color     [[attribute(1)]];
    float2 rotation0 [[attribute(2)]];
    float2 rotation1 [[attribute(3)]];
    float2 position  [[attribute(4)]];
};

struct VOut
{
    float4 position [[position]];
    float4 color;
};

vertex VOut VS(VIn inp [[stage_in]])
{
    VOut outp;
    float2x2 rotation = float2x2(inp.rotation0, inp.rotation1);
    outp.position   = float4(rotation * inp.coord + inp.position, 0, 1);
    outp.color      = inp.color;
    return outp;
}


/*
 * Metal pixel shader
 */

fragment float4 PS(VOut inp [[stage_in]])
{
    return inp.color;
}

