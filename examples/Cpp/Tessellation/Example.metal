// Metal tessellation shader

#include <metal_stdlib>

using namespace metal;


// Structure for constant buffer
struct Settings
{
	float4x4 wvpMatrix;
	float tessLevelInner;
	float tessLevelOuter;
	float twist;
	float _pad0;
};


// HULL SHADER

MTLQuadTessellationFactorsHalf PatchConstantFuncHS(constant Settings& settings)
{
	MTLQuadTessellationFactorsHalf outp;
	
	outp.edgeTessellationFactor[0] = settings.tessLevelOuter;
	outp.edgeTessellationFactor[1] = settings.tessLevelOuter;
	outp.edgeTessellationFactor[2] = settings.tessLevelOuter;
	outp.edgeTessellationFactor[3] = settings.tessLevelOuter;
	
	outp.insideTessellationFactor[0] = settings.tessLevelInner;
	outp.insideTessellationFactor[1] = settings.tessLevelInner;
	
	return outp;
}

kernel void HS(
    constant Settings&                      settings    [[buffer(1)]],
    device MTLQuadTessellationFactorsHalf*  patchTess   [[buffer(30)]],
    uint                                    id          [[thread_position_in_grid]])
{
    patchTess[id] = PatchConstantFuncHS(settings);
}


// DOMAIN SHADER

struct VertexIn
{
    float3 position [[attribute(0)]];
};

struct PatchIn
{
    patch_control_point<VertexIn> inp;
};

struct DomainOut
{
   float4 position [[position]];
   float3 color;
};

[[patch(quad, 4)]]
vertex DomainOut DS(
    PatchIn             patch       [[stage_in]],
    constant Settings&  settings    [[buffer(1)]],
    float2              tessCoord   [[position_in_patch]])
{
	DomainOut outp;
	
	// Interpolate position
	float u = tessCoord.x;
	float v = tessCoord.y;
	
	float3 a = mix(patch.inp[0].position, patch.inp[1].position, u);
	float3 b = mix(patch.inp[2].position, patch.inp[3].position, u);
	
	float3 position = mix(a, b, v);
	
	// Set final vertex color
	outp.color = (1.0 - position) * 0.5;
	
	// Apply twist rotation matrix (rotate around Y axis)
	float twistFactor = (position.y + 1.0) * 0.5;
	
	float s = sin(settings.twist * twistFactor);
	float c = cos(settings.twist * twistFactor);
	
	float3x3 rotation = float3x3(
		c,  0, -s,
		0,  1,  0,
		s,  0,  c
	);
	
	position = position * rotation;
	
	// Transform vertex by the world-view-projection matrix chain
	outp.position = settings.wvpMatrix * float4(position, 1);
	
	return outp;
}


// PIXEL SHADER

fragment float4 PS(DomainOut inp [[stage_in]])
{
	return float4(inp.color, 1);
}



