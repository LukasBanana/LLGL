// HLSL SM 3.0 (for Direct3D 9)

struct Scene
{
	float4x4	vpMatrix;
	float4 		viewPos;
	float4		fogColorAndDensity;
	float2		animVec;
};

uniform Scene scene;

struct InputVS
{
	// Per-vertex attributes
	float3		position	: POSITION;
	float2		texCoord	: TEXCOORD0;
	
	// Per-instance attributes
	float3		color		: COLOR;
	float4x4	wMatrix		: TEXCOORD1;
	float		arrayLayer	: TEXCOORD5; // Unused in SM3
};

struct OutputVS
{
	float4 position	: POSITION;
	float4 worldPos : TEXCOORD0;
	float2 texCoord	: TEXCOORD1;
	float3 color	: COLOR;
};


// VERTEX SHADER

OutputVS VS(InputVS inp)
{
	OutputVS outp;
	
	float2 offset = scene.animVec * inp.position.y;
	
	float4 coord = float4(
		inp.position.x + offset.x,
		inp.position.y,
		inp.position.z + offset.y,
		1.0
	);
	
	outp.worldPos = mul(inp.wMatrix, coord);
	outp.position = mul(scene.vpMatrix, outp.worldPos);
	
	outp.texCoord = inp.texCoord;
	outp.color    = inp.color;
	
	return outp;
}


// PIXEL SHADER

uniform sampler2D tex : register(s3);

float4 PS(OutputVS inp) : COLOR
{
	// Sample albed texture
	float4 color = tex2D(tex, inp.texCoord);
	
	// Apply alpha clipping
	clip(color.a - 0.5);
	
    // Compute fog density
    float viewDist = distance(scene.viewPos, inp.worldPos);
    float fog = viewDist*scene.fogColorAndDensity.a;
    fog = 1.0 - 1.0/exp(fog*fog);

    // Interpolate between albedo and fog color
    color.rgb = lerp(color.rgb * inp.color, scene.fogColorAndDensity.rgb, fog);
	
	return color;
};

