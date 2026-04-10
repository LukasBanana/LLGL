// WGSL (for WebGPU)

struct InputVS
{
    @location(0) position: vec2f,
    @location(1) color: vec4f,
};

struct OutputVS
{
    @builtin(position) position: vec4f,
    @location(0) color: vec4f,
};

@vertex
fn VS(inp: InputVS) -> OutputVS
{
    var outp: OutputVS;
    outp.position = vec4f(inp.position, 0.0, 1.0);
    outp.color = inp.color;
    return outp;
}

@fragment
fn PS(inp: OutputVS) -> @location(0) vec4f
{
    return inp.color;
}