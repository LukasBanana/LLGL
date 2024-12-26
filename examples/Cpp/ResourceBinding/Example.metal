// Metal shader

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Scene
{
	float4x4 vpMatrix;
};

struct Model
{
	float3   lightVec;
	uint     instance;
};

struct VSInput
{
	float3 position [[attribute(0)]];
	float3 normal   [[attribute(1)]];
	float2 texCoord [[attribute(2)]];
};

struct VSOutput
{
	float4 position [[position]];
	float4 worldPos;
	float3 normal;
	float2 texCoord;
};


// VERTEX SHADER

struct Transform
{
	float4x4 wMatrix;
};

vertex VSOutput VSMain(
    VSInput                 inp         [[stage_in]],
    constant Scene&         scene       [[buffer(3)]],
    constant Model&         model       [[buffer(2)]],
    device const Transform* transforms  [[buffer(1)]])
{
    VSOutput outp;
	Transform transform = transforms[model.instance];
	outp.worldPos = transform.wMatrix * float4(inp.position, 1);
	outp.position = scene.vpMatrix * outp.worldPos;
	outp.normal   = (transform.wMatrix * float4(inp.normal, 0)).xyz;
	outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

fragment float4 PSMain(
    VSOutput            inp             [[stage_in]],
    constant Model&     model           [[buffer(2)]],
    texture2d<float>    colorMap        [[texture(4)]],
    sampler             colorMapSampler [[sampler(5)]])
{
	// Sample color map
	float4 color = colorMap.sample(colorMapSampler, inp.texCoord);

	// Compute lighting
	float3 normal = normalize(inp.normal);
	float NdotL = max(0.2, dot(normal, model.lightVec));
	
	return float4(color.rgb * NdotL, color.a);
};

