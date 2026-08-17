# ChangeLog v0.05

Version 0.05 removed all features that were deprecatd in 0.04 and refactored a few outdated API choices that originated from OpenGL and Direct3D 11.

## Table of Contents

- [Swap-chain formats](#swap-chain-formats)
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
