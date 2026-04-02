# About the dev-d3d9 branch

This is an experimental branch to explore the possibility of supporting older devices as well as fixed-function D3D9 to allow LLGL applications to benefit from external tools such as NVIDIA Remix.

## Status

This is in its early stages with many features missing. Please do not expect this to be in a working state; some interfaces such as `ResourceHeap` are not implemented and only uniforms are supported but no emulation for constant buffers yet. There is also no test automation for this backend.

Considering that this backend will likely become a maintainence burden, there is no guarantee this backend will ever be merged into the main branch nor that it will be kept up to date.

**Use at your own discretion!**

### Supported Examples

Here is a list of examples currently working with D3D9:
- HelloTriangle
- Texturing
- Instancing

## Concepts

For tools like NVIDIA Remix to work, the D3D9 backend needs to support the fixed-function pipeline. The goal is to allow creating PSOs mostly as usual except that shaders are simply left out. LLGL required so far that at least a vertex shader is always provided. Omitting it will tell the D3D9 backend to create a fixed-function PSO internally. Most fixed-function configuration will be set via `SetResource()` and `SetUniforms()` functions. Here is an example:
```cpp
myD3D9FixedFunctionPSOLayout = renderer->CreatePipelineLayout(
    LLGL::Parse(
        // Resource identifiers are only relevant for programmable PSOs
        "texture(colorMap@0):frag,"
        "texture(normalMap@1):frag,"

        // Fixed-function attributes are specified as uniforms.
        // The names of such uniforms have special meanings and can be mapped,
        // e.g. default name for the position of D3DLIGHT9[0] is "light0_position".
        // These names should be configurable globally,
        // so that they can refer to the same meaning as their programmable PSO counterpart.
        "float3(light0_position),"
        "bool(light0_enabled),"

        // Special attributes need pre-defined constant values from <LLGL/Constants.h>
        "int(fogMode),"
    )
);
struct MyUniforms {
    struct Light {
        float position[3];
        bool  enabled;
    } lights[1];
    int32_t fogMode;
} uniforms;

...

cmdBuffer->SetPipelineState(*myD3D9FixedFunctionPSO);

// Set texture just like you would with shaders
cmdBuffer->SetResource(0, *myColorTexture);
cmdBuffer->SetResource(1, *myNormalTexture);

uniforms.lights[0].position[0] = ...
uniforms.lights[0].enabled = true;
uniforms.fogMode = LLGL_FIXED_FUNCTION_FOG_MODE_EXP2;
cmdBuffer->SetUniforms(0, &uniforms, sizeof(uniforms));
```
