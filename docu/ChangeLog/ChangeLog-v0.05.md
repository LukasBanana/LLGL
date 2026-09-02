# ChangeLog v0.05

Version 0.05 removed all features that were deprecatd in 0.04 and refactored a few outdated API choices that originated from OpenGL and Direct3D 11.

## Table of Contents

- [Swap-chain formats](#swap-chain-formats)
- [Vertex attributes](#vertex-attributes)
- [Vertex buffers](#vertex-buffers)
- [Vertex arrays](#vertex-arrays)
- [Relaxed uniform identifiers](#relaxed-uniform-identifiers)
- [Renamed identifiers](#renamed-identifiers)


## Swap-chain formats

`SwapChainDescriptor` used to have integer fields for `colorBits`, `depthBits`, and `stencilBits` that only acted as hints for the backend to pick a suitable color and depth-stencil format.
These are superseded by `colorFormat` and `depthStencilFormat` to allow choosing sRGB and non-sRGB color formats and a streamlined depth-stencil format selection.
They do, however, still only act as hints and the backend is as always free to pick a different one without reporting any errors.

**NOTE**: This is still a work-in-progress. The old fields are therefore not yet deprecated, but will be before the next release version.

Before:
```cpp
// Interface:
int LLGL::SwapChainDescriptor::colorBits;
int LLGL::SwapChainDescriptor::depthBits;
int LLGL::SwapChainDescriptor::stencilBits;
```

After:
```cpp
// Interface:
LLGL::Format LLGL::SwapChainDescriptor::colorFormat;
LLGL::Format LLGL::SwapChainDescriptor::depthStencilFormat;
```


## Vertex attributes

Vertex attributes used to be specified in two places, (1) in the vertex buffer and (2) in the vertex shader.
This has several drawbacks, (1) it's redundant, (2) it's inflexible to use a shader with different attribute formats as input layout (e.g. RGBA/BGRA/sRGB),
and (3) the vertex attributes have to be specified early as shaders are usually created once at launch while PSOs are more commonly created on the fly in many engine designs.

The new API moves these attributes to the graphics PSO and unifies them.

**NOTE**: This is still a work-in-progress. The old fields are therefore not yet deprecated, but will be before the next release version.

Before:
```cpp
// Usage (D3D example):
LLGL::ShaderDescriptor vertexShaderDesc  { LLGL::ShaderType::VertexShader,   myShaderSource, "VS", "vs_5_0" };
LLGL::ShaderDescriptor fragmentShaderDesc{ LLGL::ShaderType::FragmentShader, myShaderSource, "PS", "ps_5_0" };
vertexShaderDesc.vertex.inputAttribs = myVertexAttributes;

LLGL::BufferDescriptor vertexBufferDesc;
vertexBufferDesc.size          = sizeof(vertices);
vertexBufferDesc.bindFlags     = LLGL::BindFlags::VertexBuffer;
vertexBufferDesc.vertexAttribs = myVertexAttributes;
```

After:
```cpp
// Usage (D3D example)
LLGL::ShaderDescriptor vertexShaderDesc  { LLGL::ShaderType::VertexShader,   myShaderSource, "VS", "vs_5_0" };
LLGL::ShaderDescriptor fragmentShaderDesc{ LLGL::ShaderType::FragmentShader, myShaderSource, "PS", "ps_5_0" };

LLGL::BufferDescriptor vertexBufferDesc;
vertexBufferDesc.size          = sizeof(vertices);
vertexBufferDesc.bindFlags     = LLGL::BindFlags::VertexBuffer;

LLGL::GraphicsPipelineDescriptor graphicsPSODesc;
graphicsPSODesc.inputVertexAttribs = myVertexAttributes;
```


## Vertex buffers

As mentioned in section the [Vertex attributes](#vertex-attributes) section, vertex buffers no longer need their attributes specified.
This also sunsets the old secondary `SetVertexBuffer` function that modified the attributes as a workaround to use a vertex buffer for multiple formats.
It is superseded by a new secondary function that specfies stride and base offset (this wasn't previously supported).
The stride, however, cannot be changed as it was never supported by Vulkan and Metal; They tie the stride to the graphics PSO.
As a compromise to keep the API lightweight and backend agnostic, a vertex buffer must either have a default stride (via `BufferDescriptor::stride`) or
a custom stride via this new function, both of which *must* match the stride specified for the respective vertex attributes in the graphics PSO (via `GraphicsPipelineDescriptor::inputVertexAttribs`).

Before:
```cpp
// Interface
LLGL::CommandBuffer::SetVertexBuffer(LLGL::Buffer& buffer, std::uint32_t numVertexAttribs, const LLGL::VertexAttribute* vertexAttribs);
```

After:
```cpp
// Interface
LLGL::CommandBuffer::SetVertexBuffer(LLGL::Buffer& buffer, std::uint32_t stride, std::uint64_t offset = 0);
```


## Vertex arrays

To create vertex arrays, use the `ArrayView` initializer. This is analogous to `CreateResourceHeap()` and `WriteResourceHeap()`.

Before:
```cpp
// Interface
LLGL::BufferArray* LLGL::RenderSystem::CreateBufferArray(std::uint32_t numBuffers, LLGL::Buffer* const * bufferArray);

// Usage
LLGL::Buffer* myBuffers[] = { bufA, bufB };
LLGL::BufferArray myBufferArray = myRenderer->CreateBufferArray(sizeof(myBuffers)/sizeof(myBuffers[0]), myBuffers);
```

After:
```cpp
// Interface
LLGL::BufferArray* LLGL::RenderSystem::CreateBufferArray(ArrayView<BufferLocation> bufferLocations);

// Usage
LLGL::BufferArray myBufferArray = myRenderer->CreateBufferArray({ bufA, bufB });
```


## Relaxed uniform identifiers

This section does not describe a breaking change nor a deprecation. Instead, it outlines a relaxation of constraints
that are worth discussing in conjunction with the new scripts for automatic translation of all example shaders from a single source of HLSL.
The constraint that uniform descriptors were only allowed to contain a single identifier weren't even clearly stated previously.
This has been relaxed to also allow chains of identifiers, e.g. `"scene.projection"` can now describe a valid uniform in the pipeline layout.
This is meant to simplify the pipeline layout description with multiple shader backends in mind.
Take a look at the following HLSL shader for instance:
```hlsl
cbuffer Scene : register(b1) {
    float4x4 projection;
}
float4 VSMain(float4 position : POSITION) : SV_Position {
    return mul(projection, position);
}
```
For such a simple shader, `projection` could be updated via uniforms, which are lightweight data that can be updated fast (via `SetUniforms()`)
without involving buffer copy operations (via `UpdateBuffer()`).
Translating this by hand would allow to have the same uniform in GLSL, e.g. `uniform mat4 projection;`,
but a cross-compiler toolchain might translate it to something like this:
```glsl
#version 140
layout(std140) uniform Scene {
    mat4 projection;
};
in float4 POSITION;
void main() {
    gl_Position = projection * POSITION;
}
```
Now the input parameter is no longer a loose uniform but has become a UBO, which does not work with `SetUniforms()`.
This can be changed by marking the cbuffer in HLSL with the SPIR-V specific `[[vk::push_constant]]` attribute:
```hlsl
struct Scene_t {
    float4x4 projection;
};
#if __spirv__
[[vk::push_constant]] Scene_t scene;
#else
cbuffer Scene : register(b1) {
    Scene_t scene;
}
#endif
float4 VSMain(float4 position : POSITION) : SV_Position {
    return mul(scene.projection, position);
}
```
Since SPIR-V does not support more than one push constant block, we'd have to put our cbuffer fields into a struct.
For D3D output (DXIL or DXBC), we can share the same declaration to avoid duplication and use that struct as-is.
The attentive reader might already know where this leads us since we now have to address the uniforms by the identifier chain `scene.projection`.
Eventhough SPIR-V only supports a single push constant block, the HLSL input could technically still have other cbuffers
and structs that use the identifier `projection`, so distinguishing them is critical.
With the new relaxed rules, LLGL supports specifying this uniform field with `scene.projection`.
This has also been updated in the `Parse()`/`ParseContext` API:
```cpp
myRenderer->CreatePipelineLayout(
    LLGL::Parse("float4x4( scene.projection )")
);
```


## Renamed identifiers

The following identifiers have also been renamed and their old names have been deprecated as type aliases, i.e. they are still available but will be removed in the next version of LLGL:

##### Key::**BrowserFavorits** &rarr; **BrowserFavorites**
This was a typo.

##### ShaderType::**Amplification** &rarr; **Task**
Short lived identifier that originated from D3D terminology. The new identifier is using the shorter and more fitting name adopted from the Vulkan terminology, following the other shader stage identifiers.

##### StageFlags::**AmplificationStage** &rarr; **TaskStage**
Analogous to `ShaderType::Amplification`.

##### MeshPipelineDescriptor::**amplificationShader** &rarr; **taskShader**
Analogous to `ShaderType::Amplification`.

##### ProfileCommandQueueRecord::**commandBufferSubmittions** &rarr; **commandBufferSubmissions**
This was a typo.
