// WGSL texturing shader

struct Scene
{
    wvpMatrix : mat4x4f,
    wMatrix   : mat4x4f,
    lightVec  : vec3f,
};

@group(0) @binding(1) var<uniform> scene : Scene;

// VERTEX SHADER

struct InputVS
{
    @location(0) position : vec3f, // POSITION
    @location(1) normal   : vec3f, // NORMAL
    @location(2) texCoord : vec2f, // TEXCOORD
};

struct VertexOutput
{
    @builtin(position) position : vec4f, // SV_Position
    @location(0)       normal   : vec3f, // NORMAL
    @location(1)       texCoord : vec2f, // TEXCOORD
};

@vertex
fn VS(inp : InputVS) -> VertexOutput
{
    var outp : VertexOutput;
    outp.position = scene.wvpMatrix * vec4f(inp.position, 1.0);
    outp.normal = normalize((scene.wMatrix * vec4f(inp.normal, 0.0)).xyz);
    outp.texCoord = inp.texCoord;
    return outp;
}


// PIXEL SHADER

@group(0) @binding(2) var colorMap     : texture_2d<f32>;
@group(0) @binding(3) var samplerState : sampler;

@fragment
fn PS(inp : VertexOutput) -> @location(0) vec4f
{
    var color = textureSample(colorMap, samplerState, inp.texCoord);

	// Apply lambert factor for simple shading
    let NdotL = dot(scene.lightVec, normalize(inp.normal));
    color = vec4f(color.rgb * mix(0.2, 1.0, NdotL), color.a);
    
    return color;
}
