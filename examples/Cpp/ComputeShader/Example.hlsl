/*
 * HLSL compute shader
 */

#define MIN_DIST    ( 0.5 )
#define MAX_DIST    ( 0.5 )
#define MIN_RADIUS  ( 0.1 )
#define MAX_RADIUS  ( 0.1 )
#define M_PI        ( 3.141592654 )

cbuffer SceneState : register(b2)
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

RWBuffer<float4> sceneObjects : register(u3);

struct DrawIndirectArguments
{
    uint numVertices;
    uint numInstances;
    uint firstVertex;
    uint firstInstance;
};

RWBuffer<uint4> drawArgs : register(u4);

void WriteDrawArgs(DrawIndirectArguments args, uint idx)
{
    drawArgs[idx] = uint4(
        args.numVertices,
        args.numInstances,
        args.firstVertex,
        args.firstInstance
    );
}

void WriteDrawArgsTri(uint idx, uint numInstances, uint firstInstance)
{
    DrawIndirectArguments args;
    args.numVertices    = 3;
    args.numInstances   = numInstances;
    args.firstVertex    = 0;
    args.firstInstance  = firstInstance;
    WriteDrawArgs(args, idx);
}

void WriteDrawArgsQuad(uint idx, uint numInstances, uint firstInstance)
{
    DrawIndirectArguments args;
    args.numVertices    = 4;
    args.numInstances   = numInstances;
    args.firstVertex    = 3;
    args.firstInstance  = firstInstance;
    WriteDrawArgs(args, idx);
}

void WriteSceneObject(uint idx)
{
    // Compute scene object parameters
    float a = frac(time*0.1 + float(idx)/float(numSceneObjects))*M_PI*2.0;
    float t = cos(a*5.0)*0.5 + 0.5;
    float d = lerp(MIN_DIST, MAX_DIST, t);
    float r = lerp(MIN_RADIUS, MAX_RADIUS, t);
    float s = sin(a*2.0);
    float c = cos(a*2.0);

    // Compute rotation and position
    idx *= 2;
    sceneObjects[idx    ] = float4(c*r, s*r, -s*r, c*r);
    sceneObjects[idx + 1] = float4(sin(a)*d, cos(a)*d, 0.0, 0.0);
}

[numthreads(1, 1, 1)]
void CS(uint3 threadID : SV_DispatchThreadID)
{
    // Write draw arguments to indirect argument buffer
    if (threadID.x == 0)
    {
        float t = frac(time*0.1);
        if (t < 0.5)
        {
            uint n = (uint)lerp(0.0, (float)numSceneObjects, t * 2.0);
            WriteDrawArgsTri(0, n, 0);
            WriteDrawArgsQuad(1, numSceneObjects - n, n);
        }
        else
        {
            uint n = (uint)lerp(0.0, (float)numSceneObjects, (t - 0.5) * 2.0);
            WriteDrawArgsQuad(1, n, 0);
            WriteDrawArgsTri(0, numSceneObjects - n, n);
        }
    }

    // Write scene transformations
    WriteSceneObject(threadID.x);
}


/*
 * HLSL vertex shader
 */

struct VIn
{
    float2   coord    : COORD;
    float4   color    : COLOR;
    float2x2 rotation : ROTATION;
    float2   position : POSITION;
};

struct VOut
{
    float4 position : SV_Position;
    float4 color    : COLOR;
};

VOut VS(in VIn inp)
{
    VOut outp;
    outp.position   = float4(mul(inp.rotation, inp.coord) + inp.position, 0, 1);
    outp.color      = inp.color;
    return outp;
}


/*
 * HLSL pixel shader
 */

float4 PS(in VOut inp) : SV_Target0
{
    return inp.color;
}

