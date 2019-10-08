/*
 * HLSL compute shader
 */

#define SIZEOF_PARTICLE (7)

cbuffer SceneState : register(b1)
{
    float4x4    wvpMatrix;
    float4x4    wMatrix;
    float4      gravity;
    uint2       gridSize;
    uint        numIterations;
    uint        _pad0;
    float       damping;
    float       stiffness;
    float       dt;
    float       _pad1;
    float4      lightVec;
};

struct Particle
{
    float4 origPos;
    float4 prevPos;
    float4 nextPos;
    float4 velocity;
    float4 currPos;
    float4 normal;
    float4 uv_invMass;
};

RWBuffer<float4> particles : register(u2);

uint GridPosToIndex(uint2 gridPos)
{
    return (gridPos.y * gridSize.x + gridPos.x) * SIZEOF_PARTICLE;
}

void ReadParticle(uint2 gridPos, out Particle par)
{
    uint i = GridPosToIndex(gridPos);
    par.origPos     = particles[i + 0];
    par.prevPos     = particles[i + 1];
    par.nextPos     = particles[i + 2];
    par.velocity    = particles[i + 3];
    par.currPos     = particles[i + 4];
    par.normal      = particles[i + 5];
    par.uv_invMass  = particles[i + 6];
}

void ReadParticleSub(uint2 gridPos, out float3 currPos, out float3 origPos, out float invMass)
{
    uint i = GridPosToIndex(gridPos);
    origPos = particles[i + 0].xyz;
    currPos = particles[i + 4].xyz;
    invMass = particles[i + 6].z;
}

float3 ReadParticlePos(uint2 gridPos)
{
    uint i = GridPosToIndex(gridPos);
    return particles[i + 4].xyz;
}

void WriteParticle(uint2 gridPos, in Particle par)
{
    uint i = GridPosToIndex(gridPos);
    //particles[i + 0] = par.origPos;
    particles[i + 1] = par.prevPos;
    particles[i + 2] = par.nextPos;
    particles[i + 3] = par.velocity;
    particles[i + 4] = par.currPos;
    //particles[i + 5] = par.normal;
    //particles[i + 6] = par.uv_invMass;
}

void WriteParticleNextPosAndNormal(uint2 gridPos, float4 nextPos, float4 normal)
{
    uint i = GridPosToIndex(gridPos);
    particles[i + 2] = nextPos;
    particles[i + 5] = normal;
}

void AccumulateStretchConstraints(Particle par, int2 neighborGridPos, inout float3 dCorrection)
{
    if (neighborGridPos.x < 0 || (uint)neighborGridPos.x >= gridSize.x ||
        neighborGridPos.y < 0 || (uint)neighborGridPos.y >= gridSize.y)
    {
        return;
    }

    // Read neighbor particle
    float3 neighborPos;
    float3 neighborOrigPos;
    float neighborInvMass;
    ReadParticleSub((uint2)neighborGridPos, neighborPos, neighborOrigPos, neighborInvMass);

    // Compute edge distance between particle and its neighbor
    float3 dPos = par.nextPos.xyz - neighborPos;
    float currDist = length(dPos);
    float edgeDist = distance(par.origPos.xyz, neighborOrigPos);

    // Compute stretch constraint
    dPos = normalize(dPos) * ((currDist - edgeDist) / (par.uv_invMass.z + neighborInvMass));

    // Adjust position
    dCorrection += (dPos * -par.uv_invMass.z);
}

void AccumulateSurfaceNormal(float4 pos, int2 gridPos0, int2 gridPos1, inout float3 normal)
{
    if (gridPos0.x < 0 || (uint)gridPos0.x >= gridSize.x ||
        gridPos0.y < 0 || (uint)gridPos0.y >= gridSize.y ||
        gridPos1.x < 0 || (uint)gridPos1.x >= gridSize.x ||
        gridPos1.y < 0 || (uint)gridPos1.y >= gridSize.y)
    {
        return;
    }

    float3 v0 = ReadParticlePos(gridPos0) - pos.xyz;
    float3 v1 = ReadParticlePos(gridPos1) - pos.xyz;

    normal += cross(v0, v1);
}

void ApplyStretchConstraints(inout Particle par, int2 gridPos)
{
    if (par.uv_invMass.z == 0.0)
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
    float3 normal = (float3)0;
    AccumulateSurfaceNormal(par.currPos, gridPos + int2( 0, +1), gridPos + int2(+1,  0), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2(+1,  0), gridPos + int2( 0, -1), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2( 0, -1), gridPos + int2(-1,  0), normal);
    AccumulateSurfaceNormal(par.currPos, gridPos + int2(-1,  0), gridPos + int2( 0, +1), normal);
    par.normal.xyz = normal / 4.0;

    // Adjust position by correction vector
    par.nextPos.xyz += dPos * stiffness;
}

[numthreads(1, 1, 1)]
void CSForces(uint2 threadID : SV_DispatchThreadID)
{
    // Read particle
    Particle par;
    ReadParticle(threadID, par);

    // Accumulate force and multiply by inverse mass
    float3 force = gravity.xyz;
    force *= par.uv_invMass.z;

    // Apply velocity and damping
    par.velocity.xyz += force * dt * damping;

    // Apply position based physics simulation
    par.currPos.xyz += par.velocity.xyz * dt;

    // Write particle back to swap-buffer
    WriteParticle(threadID, par);
}

[numthreads(1, 1, 1)]
void CSStretchConstraints(uint2 threadID : SV_DispatchThreadID)
{
    // Read particle
    Particle par;
    ReadParticle(threadID, par);
    par.nextPos = par.currPos;

    // Apply stretch constraints
    ApplyStretchConstraints(par, (int2)threadID);

    // Write next position back to swap-buffer
    WriteParticleNextPosAndNormal(threadID, par.nextPos, par.normal);
}

[numthreads(1, 1, 1)]
void CSRelaxation(uint2 threadID : SV_DispatchThreadID)
{
    // Read particle
    Particle par;
    ReadParticle(threadID, par);

    // Adjust velocity
    par.currPos = par.nextPos;
    par.velocity.xyz = (par.currPos.xyz - par.prevPos.xyz) / dt;
    par.prevPos = par.currPos;

    // Write particle back to swap-buffer
    WriteParticle(threadID, par);
}


/*
 * HLSL vertex shader
 */

struct VIn
{
    float4 position : POS;
    float4 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

struct VOut
{
    float4 position : SV_Position;
    float4 normal   : NORMAL;
    float2 texCoord : TEXCOORD;
};

VOut VS(in VIn inp)
{
    VOut outp;
    outp.position   = mul(wvpMatrix, inp.position);
    outp.normal     = mul(wMatrix, inp.normal);
    outp.texCoord   = inp.texCoord;
    return outp;
}


/*
 * HLSL pixel shader
 */

float4 PS(in VOut inp, bool frontFace : SV_IsFrontFace) : SV_Target0
{
    // Compute lighting
    float3 normal = normalize(inp.normal.xyz);
    normal *= lerp(1.0, -1.0, frontFace);

    #if 0
    return float4(normal*0.5+0.5,1.0);
    #endif

    float NdotL = lerp(0.2, 1.0, max(0.0, dot(normal, -lightVec.xyz)));

    // Sample color texture
    float4 diffuse = (float4)1;

    diffuse.rg = lerp(diffuse.rg, inp.texCoord, 0.5);

    float4 color = diffuse;

    return float4(color.rgb * NdotL, color.a);
}

