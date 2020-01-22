# ChangeLog v0.03

## Table of Contents

- [`BufferDescriptor` interface](#bufferdescriptor-interface)
- [`TextureDescriptor` interface](#texturedescriptor-interface)
- [MIP-map generation](#mip-map-generation)
- [Index buffer format](#index-buffer-format)
- [Storage buffer binding](#storage-buffer-binding)
- [Event listener interface](#event-listener-interface)
- [`ShaderUniform` interface](#shaderuniform-interface)
- [Shader reflection](#shader-reflection)
- [Renderer configuration](#renderer-configuration)
- [Default values](#default-values)
- [Renamed identifiers](#renamed-identifiers)
- [`PipelineLayoutDesc` syntax](#pipelinelayoutdesc-syntax)
- [`Format` information](#format-information)
- [Direct resource binding](#direct-resource-binding)
- [Resource heap binding](#resource-heap-binding)
- [Vertex attribute description](#vertex-attribute-description)
- [Storage buffer description](#storage-buffer-description)
- [Multi-sampling descriptor](#multi-sampling-descriptor)
- [`ReadTexture` interface](#readtexture-interface)
- [Stream-output interface](#stream-output-interface)
- [Pipeline state interface](#pipeline-state-interface)
- [Removed features](#removed-features)


## `BufferDescriptor` interface

A buffer no longer has a unique type. Instead the binding flags specify for which purposes the buffer will be used. This enables the buffer to be used for multiple purposes, e.g. as read/write storage buffer in a compute shader and later as an indirect argument buffer for drawing commands. Overall the `flags` member has been replaced by three new flags: `bindFlags`, `cpuAccessFlags`, and `miscFlags`. The new flags enumerations are shared between `BufferDescriptor` and `TextureDescriptor`: `BindFlags`, `CPUAccessFlags`, and `MiscFlags`.

Before:
```cpp
// Interface:
BufferType BufferDescriptor::type;
long       BufferDescriptor::flags;
uint64_t   BufferDescriptor::size;
/* ... */

// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.type  = LLGL::ConstantBuffer;
myBufferDesc.flags = LLGL::BufferFlags::DynamicUsage | LLGL::BufferFlags::MapWriteAccess;
myBufferDesc.size  = /* ... */;
```

After:
```cpp
// Interface:
uint64_t BufferDescriptor::size;
long     BufferDescriptor::bindFlags;
long     BufferDescriptor::cpuAccessFlags;
long     BufferDescriptor::miscFlags;
/* ... */

// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.size           = /* ... */;
myBufferDesc.bindFlags      = LLGL::BindFlags::ConstantBuffer;
myBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
myBufferDesc.miscFlags      = LLGL::MiscFlags::DynamicUsage;
```


## `TextureDescriptor` interface

Textures still have a unique type, but also use the new flags enumerations `BindFlags`, `CPUAccessFlags`, and `MiscFlags`.

Before:
```cpp
// Interface:
TextureType TextureDescriptor::type;
long        TextureDescriptor::flags;
Format      TextureDescriptor::format;
/* ... */

// Usage:
LLGL::TextureDescriptor myTexDesc;
myTexDesc.type  = LLGL::TextureType::Texture2DMS;
myTexDesc.flags = LLGL::TextureFlags::ColorAttachmentUsage |
                  LLGL::TextureFlags::SampleUsage          |
                  LLGL::TextureFlags::StorageUsage         |
                  LLGL::TextureFlags::FixedSamples;
/* ... */
```

After:
```cpp
// Interface:
TextureType TextureDescriptor::type;
long        TextureDescriptor::bindFlags;
long        TextureDescriptor::cpuAccessFlags;
long        TextureDescriptor::miscFlags;
Format      TextureDescriptor::format;
/* ... */

// Usage:
LLGL::TextureDescriptor myTexDesc;
myTexDesc.type              = LLGL::TextureType::Texture2DMS;
myTexDesc.bindFlags         = LLGL::BindFlags::ColorAttachment |
                              LLGL::BindFlags::Sampled         |
                              LLGL::BindFlags::Storage;
myTexDesc.cpuAccessFlags    = 0;
myTexDesc.miscFlags         = LLGL::MiscFlags::FixedSamples;
```


## MIP-map generation

The `GenerateMips` function has been moved from `RenderSystem` interface to `CommandBuffer` interface.
That said, the MIP-map generation at texture creation time has been simplified, by adding the `MiscFlags::GenerateMips` flag to the texture descriptor.
In order to generate MIP-maps *automatically*, either at texture creation time or during command buffer encoding,
the texture must be created with the binding flags `BindFlags::ColorAttachment` and `BindFlags::Sampled`.
Alternatively, the MIP-maps can be written *manually* using the `RenderSystem::WriteTexture` function.
For compressed texture formats, automatic MIP-map generation is not supported, because they cannot be used as render targets.

Before:
```cpp
// At texture creation:
LLGL::TextureDescriptor myTexDesc;
/* ... */
LLGL::Texture* myTexture = myRenderer->CreateTexture(myTexDesc);
myRenderer->GenerateMips(*myTexDesc);

// During rendering:
myCmdBuffer->Begin();
/* 1st part ... */
myCmdBuffer->End();
myCmdQueue->Submit(*myCmdBuffer);

myRenderer->GenerateMips(*myTexture);

myCmdBuffer->Begin();
/* 2nd part ... */
myCmdBuffer->End();
myCmdQueue->Submit(*myCmdBuffer);
```

After:
```cpp
// At texture creation:
LLGL::TextureDescriptor myTexDesc;
myTexDesc.bindFlags = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment;
myTexDesc.miscFlags = LLGL::MiscFlags::GenerateMips;
/* ... */
LLGL::Texture* myTexture = myRenderer->CreateTexture(myTexDesc);

// During rendering:
myCmdBuffer->Begin();
/* 1st part ... */
myCmdBuffer->GenerateMips(*myTexture);
/* 2nd part ... */
myCmdBuffer->End();
myCmdQueue->Submit(*myCmdBuffer);
```


## Index buffer format

The class `IndexFormat` has been removed and replaced by the `Format` enum. The only valid index formats are `Format::R16UInt` (16-bit) and `Format::R32UInt` (32-bit). An 8-bit index format is no longer supported (OpenGL was the only renderer that supports it).
Moreover, the index format in the buffer descriptor is optional. If the format is `Format::Undefined`, only the secondary `SetIndexBuffer` function of the `CommandBuffer` interface can be used with that index buffer.

Before:
```cpp
// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.type               = LLGL::BufferType::Index;
myBufferDesc.indexBuffer.format = LLGL::IndexFormat(LLGL::DataType::UInt16);
/* ... */
myCmdBuffer->SetIndexBuffer(*myIndexBuffer);
```

After:
```cpp
// Usage (Option A):
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.bindFlags   = LLGL::BindFlags::IndexBuffer;
myBufferDesc.format      = LLGL::Format::R16UInt;
/* ... */
myCmdBuffer->SetIndexBuffer(*myIndexBuffer);

// Usage (Option B):
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.bindFlags   = LLGL::BindFlags::IndexBuffer;
/* ... */
myCmdBuffer->SetIndexBuffer(*myIndexBuffer, LLGL::Format::R16UInt);
```


## Storage buffer binding

*TODO*


## Event listener interface

The following functions have been renamed:
- `Window::EventListener::OnLoseFocus` => `...::OnLostFocus`
- `Window::PostLoseFocus` => `...::PostLostFocus`

Moreover, the interface of the `Window::EventListener::OnQuit` function has changed:

Before:
```cpp
bool OnQuit(Window& sender);
```

After:
```cpp
void OnQuit(Window& sender, bool& veto);
```


## `ShaderUniform` interface

The `ShaderUniform` interface has been replaced by `SetUniform` and `SetUniforms` functions in the `CommandBuffer` interface.
Originally, the `ShaderUniform` interface was only intended to set binding points for the OpenGL backend when the GLSL version does not support [explicit binding points](https://www.khronos.org/opengl/wiki/Layout_Qualifier_(GLSL)#Binding_points).
To avoid setting these binding points in each render pass over and over again, a `name` property has been added to the `BindingDescriptor` structure.
This member is only required for OpenGL before version 4.2, or when the GLSL version does not support explicit binding points.
The resource name in conjunction with the pipeline layout will tell the OpenGL backend how to initialize the respective uniforms.
All other uniforms (matrices and other parameters for instance) must be set with the new `SetUniform`/`SetUniforms` functions each time a graphics pipeline is bound.
Also the syntax of the utility function `PipelineLayoutDesc` has been extended to support this optional property.

Before:
```cpp
// Interface:
class           ShaderUniform;
ShaderUniform*  ShaderProgram::LockShaderUniform();
void            ShaderProgram::UnlockShaderUniform();
void            ShaderProgram::BindConstantBuffer(const std::string& name,
                                                  std::uint32_t      bindingIndex);
void            ShaderProgram::BindStorageBuffer(const std::string& name,
                                                 std::uint32_t      bindingIndex);

// Usage:
float myProjectionMatrix[16] = /* ... */;
myShaderProgram->BindConstantBuffer("mySceneParams", 1);
if (LLGL::ShaderUniform* myUniformHandler = myShaderProgram->LockShaderUniform()) {
    myUniformHandler->SetUniform1i("myColorMap", 2);
    myUniformHandler->SetUniform4x4fv("myProjection", &myProjectionMatrix[0]);
    myShaderProgram->UnlockShaderUniform();
}
```

After:
```cpp
// Inteface:
std::string BindingDescriptor::name;
void CommandBuffer::SetUniform(UniformLocation location,
                               const void*     data,
                               std::uint32_t   dataSize);
void CommandBuffer::SetUniforms(UniformLocation location,
                                std::uint32_t   count,
                                const void*     data,
                                std::uint32_t   dataSize);

// Usage:
LLGL::PipelineLayoutDescriptor myLayoutDesc = LLGL::PipelineLayoutDesc(
    "cbuffer(mySceneParams@1):frag,"
    "texture(myColorMap@2):frag,"
);
LLGL::PipelineLayout* myLayout = myRenderer->CreatePipelineLayout(myLayoutDesc);
LLGL::UniformLocation myProjectionUniform = myShaderProgram->FindUniformLocation("myProjection");

/* ... */

myCmdBuffer->SetPipelineState(*myGfxPipeline);
float myProjectionMatrix[16] = /* ... */;
myCmdBuffer->SetUniform(myProjectionUniform, &myProjectionMatrix[0], sizeof(myProjectionMatrix));
```


## Shader reflection

All nested structures in `ShaderReflectionDescriptor` were moved into `LLGL` namespace and renamed as follows:
```cpp
LLGL::ShaderReflectionDescriptor                -> LLGL::ShaderReflection
LLGL::ShaderReflectionDescriptor::ResourceView  -> LLGL::ShaderResource
LLGL::ShaderReflectionDescriptor::Uniform       -> LLGL::ShaderUniform
```
All binding specific attributes in `ShaderResource` have been replaced by the `BindingDescriptor` structure.
Moreover, the shader reflection does no longer throw exceptions and only returns false on failure.

Before:
```cpp
// Interface:
std::string             LLGL::ShaderReflectionDescriptor::ResourceView::name;
LLGL::ResourceType      LLGL::ShaderReflectionDescriptor::ResourceView::type;
long                    LLGL::ShaderReflectionDescriptor::ResourceView::bindFlags;
long                    LLGL::ShaderReflectionDescriptor::ResourceView::stageFlags;
std::uint32_t           LLGL::ShaderReflectionDescriptor::ResourceView::slot;
std::uint32_t           LLGL::ShaderReflectionDescriptor::ResourceView::arraySize;
std::uint32_t           LLGL::ShaderReflectionDescriptor::ResourceView::constantBufferSize;
LLGL::StorageBufferType LLGL::ShaderReflectionDescriptor::ResourceView::storageBufferType;

LLGL::ShaderReflectionDescriptor ShaderProgram::QueryReflectionDesc() const;

// Usage:
try {
    LLGL::ShaderReflectionDescriptor reflection = myShaderProgram->QueryReflectionDesc();
    /* Evaluate ... */
} catch (const std::exception& e) {
    /* Error ... */
}
```

After:
```cpp
// Interface:
LLGL::BindingDescriptor LLGL::ShaderResource::binding;
std::uint32_t           LLGL::ShaderResource::constantBufferSize;
LLGL::StorageBufferType LLGL::ShaderResource::storageBufferType;

bool ShaderProgram::Reflect(LLGL::ShaderReflection& reflection) const;

// Usage:
LLGL::ShaderReflection reflection;
if (myShaderProgram->Reflect(reflection)) {
    /* Evaluate ... */
} else {
    /* Error ... */
}
```


## Renderer configuration

Renderer configuration between Vulkan and OpenGL has been unified.
The OpenGL context profile is no longer configured with each `RenderContext` but the entire `RenderSystem` instead.
Same for the debug callback.

Before:
```cpp
// Usage:
LLGL::RenderSystemDescriptor myRendererDesc;
myRendererDesc.moduleName                   = "OpenGL";
LLGL::RenderSystem* myRenderer = LLGL::RenderSystem::Load(myRendererDesc);

LLGL::RenderContextDescriptor myContextDesc;
myContextDesc.videoMode.resolution          = { 800, 600 };
myContextDesc.debugCallback                 = myDebugCallbackProc;
myContextDesc.profileOpenGL.contextProfile  = LLGL::OpenGLContextProfile::CoreProfile;
LLGL::RenderContext* myContext = myRenderer->CreateRenderContext(myContextDesc);
```

After:
```cpp
LLGL::RendererConfigurationOpenGL myRendererConfig;
myRendererConfig.contextProfile     = LLGL::OpenGLContextProfile::CoreProfile;

LLGL::RenderSystemDescriptor myRendererDesc;
myRendererDesc.moduleName           = "OpenGL";
myRendererDesc.debugCallback        = myDebugCallbackProc;
myRendererDesc.rendererConfig       = &myRendererConfig;
myRendererDesc.rendererConfigSize   = sizeof(myRendererConfig);
LLGL::RenderSystem* myRenderer = LLGL::RenderSystem::Load(myRendererDesc);

LLGL::RenderContextDescriptor myContextDesc;
myContextDesc.videoMode.resolution  = { 800, 600 };
LLGL::RenderContext* myContext = myRenderer->CreateRenderContext(myContextDesc);
```


## Default values

Before:
```cpp
LLGL::TextureDescriptor::type = LLGL::TextureType::Texture1D;
```

After:
```cpp
LLGL::TextureDescriptor::type = LLGL::TextureType::Texture2D;
```

## Renamed identifiers

Before/After:
```cpp
BindFlags::RWStorageBuffer             --> BindFlags::Storage
BindFlags::SampleBuffer                --> BindFlags::Sampled
CommandBuffer::SetRWStorageBuffer      --> CommandBuffer::SetResource
CommandBuffer::SetSampleBuffer         --> CommandBuffer::SetResource
Display::QueryList                     --> Display::InstantiateList
Display::QueryPrimary                  --> Display::InstantiatePrimary
Display::QuerySupportedDisplayModes    --> Display::GetSupportedDisplayModes
Format::BC1RGBA                        --> Format::BC1UNorm
Format::BC2RGBA                        --> Format::BC2UNorm
Format::BC3RGBA                        --> Format::BC3UNorm
FrameProfile::rwStorageBufferBindings  --> FrameProfile::storageBufferBindings
FrameProfile::sampleBufferBindings     --> FrameProfile::sampledBufferBindings
ImageDataSize                          --> GetMemoryFootprint
Image::QueryDstDesc                    --> Display::GetDstDesc
Image::QuerySrcDesc                    --> Display::GetSrcDesc
RenderingFeatures::hasCommandBufferExt --> RenderingFeatures::hasDirectResourceBinding
RenderContext::QueryColorFormat        --> RenderContext::GetColorFormat
RenderContext::QueryDepthStencilFormat --> RenderContext::GetDepthStencilFormat
Resource::QueryResourceType            --> Resource::GetResourceType
ShaderProgram::QueryInfoLog            --> ShaderProgram::GetReport
ShaderProgram::QueryUniformLocation    --> ShaderProgram::FindUniformLocation
TextureBufferSize                      --> GetMemoryFootprint
Texture::QueryDesc                     --> Texture::GetDesc
Texture::QueryMipExtent                --> Texture::GetMipExtent
StorageBufferType::Buffer              --> StorageBufferType::TypedBuffer
StorageBufferType::RWBuffer            --> StorageBufferType::RWTypedBuffer
```


## `PipelineLayoutDesc` syntax

Besides additions to the syntax of the `PipelineLayoutDesc` utility function, the following breaking change has been made:
The identifier `"sbuffer"` was renamed to `"buffer"`.


## `Format` information

Various functions to get meta data from a `Format` enumeration entry have been combined into a single function named `GetFormatAttribs`.

Before:
```cpp
// Interface:
std::uint32_t FormatBitSize(const Format format);
bool SplitFormat(const Format   format,
                 DataType&      dataType,
                 std::uint32_t& components);
bool FindSuitableImageFormat(const Format format,
                             ImageFormat& imageFormat,
                             DataType&    dataType);

// Usage:
myTextureByteSize = myTexturePixelCount * LLGL::FormatBitSize(myFormat) / 8;
LLGL::SplitFormat(myFormat, myDataType, myFormatComponents);
LLGL::FindSuitableImageFormat(myFormat, myImageFormat, myDataType);
```

After:
```cpp
// Interface:
const LLGL::FormatAttributes& GetFormatAttribs(const LLGL::Format format);

// Usage:
const auto& myFormatAttribs = LLGL::GetFormatAttribs(myFormat);
myTextureByteSize  = myTexturePixelCount * myFormatAttribs.bitSize / myFormatAttribs.blockWidth / myFormatAttribs.blockHeight / 8;
myFormatComponents = myFormatAttribs.components;
myImageFormat      = myFormatAttribs.format;
myDataType         = myFormatAttribs.dataType;
```


## Direct resource binding

The `CommandBufferExt` interface has been removed and all functions for direct resource binding have been replaced by `CommandBuffer::SetResource`.
The `ResetResourceSlots` function has been moved into `CommandBuffer` interface.

Before:
```cpp
// Interface:
void CommandBufferExt::SetConstantBuffer(LLGL::Buffer& buffer, std::uint32_t slot, long stageFlags = LLGL::StageFlags::AllStages);
void CommandBufferExt::SetSampledBuffer(LLGL::Buffer& buffer, std::uint32_t slot, long stageFlags = LLGL::StageFlags::AllStages);
void CommandBufferExt::SetStorageBuffer(LLGL::Buffer& buffer, std::uint32_t slot, long stageFlags = LLGL::StageFlags::AllStages);
void CommandBufferExt::SetTexture(LLGL::Texture& texture, std::uint32_t slot, long stageFlags = LLGL::StageFlags::AllStages);
void CommandBufferExt::SetSampler(LLGL::Sampler& sampler, std::uint32_t slot, long stageFlags = LLGL::StageFlags::AllStages);

// Usage:
myCmdBufferExt->SetConstantBuffer(*myConstBuffer, 0, LLGL::StageFlags::VertexStage);
myCmdBufferExt->SetTexture(*myColorMap, 1, LLGL::StageFlags::FragmentStage);
```

After:
```cpp
// Interface:
void CommandBuffer::SetResource(LLGL::Resource& resource,
                                std::uint32_t   slot,
                                long            bindFlags,
                                long            stageFlags = LLGL::StageFlags::AllStages);

// Usage:
myCmdBuffer->SetResource(*myConstBuffer, 0, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage);
myCmdBuffer->SetResource(*myColorMap, 1, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage);
```

## Resource heap binding

The `SetGraphicsResourceHeap` and `SetComputeResourceHeap` functions have been merged into a single function named `SetResourceHeap`.
The new parameter is optional and can be used when a resource heap contains resources for both the graphics and compute pipeline to avoid unnecessary bindings.
Moreover, the second parameter `firstSet` can finally be used for all backends. It specifies which set of descriptors is to be bound.
From now on, the number of descriptors must be a multiple of pipeline layout bindings instead of being equal to the number of bindings.
This enables the resource heaps to be used as actual heaps with an arbitrary number of resources.

Before:
```cpp
// Interface:
void CommandBuffer::SetGraphicsResourceHeap(LLGL::ResourceHeap& resourceHeap, std::uint32_t firstSet = 0);
void CommandBuffer::SetComputeResourceHeap(LLGL::ResourceHeap& resourceHeap, std::uint32_t firstSet = 0);

// Usage:
myCmdBuffer->SetGraphicsResourceHeap(*myCommonResourceHeap);
myCmdBuffer->SetGraphicsResourceHeap(*myGfxResourceHeap);
```

After:
```cpp
// Interface:
void CommandBuffer::SetResourceHeap(
    LLGL::ResourceHeap&             resourceHeap,
    std::uint32_t                   firstSet        = 0,
    const LLGL::PipelineBindPoint   bindPoint       = LLGL::PipelineBindPoint::Undefined
);

// Usage:
myCmdBuffer->SetResourceHeap(*myCommonResourceHeap, LLGL::PipelineBindPoint::Graphics);
myCmdBuffer->SetResourceHeap(*myGfxResourceHeap);
```


## Vertex attribute description

The `VertexFormat` structure is no longer required and only remains as a utility. All vertex attributes are passed as a single list.
Moreover, the input layout description has been moved from `ShaderProgramDescriptor` to `ShaderDescriptor`.

Before:
```cpp
// Interface:
std::vector<LLGL::VertexFormat>          LLGL::ShaderProgramDescriptor::vertexFormats;
LLGL::StreamOutputFormat                 LLGL::ShaderDescriptor::StreamOutput::format;
LLGL::ShaderDescriptor::StreamOutput     LLGL::ShaderDescriptor::streamOutput;
std::vector<LLGL::VertexAttribute>       LLGL::ShaderReflectionDescriptor::vertexAttributes;
std::vector<LLGL::StreamOutputAttribute> LLGL::ShaderReflectionDescriptor::streamOutputAttributes;
LLGL::VertexFormat                       LLGL::BufferDescriptor::VertexBuffer::format;
LLGL::BufferDescriptor::VertexBuffer     LLGL::BufferDescriptor::vertexBuffer;
LLGL::IndexFormat                        LLGL::BufferDescriptor::IndexBuffer::format;
LLGL::BufferDescriptor::IndexBuffer      LLGL::BufferDescriptor::indexBuffer;

// Usage:
struct MyVertex {
    float position[3];       // attribute 0
    float normal[3];         // attribute 1
    float texCoord[2];       // attribute 2
};
struct MyInstance {
    float   transform[4][4]; // attribute 3...6
    uint8_t color[4];        // attribute 7
};

LLGL::VertexFormat myVertexFmt;
myVertexFmt.AppendAttribute({ "position", LLGL::Format::RGB32Float });
myVertexFmt.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
myVertexFmt.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });
myVertexFmt.inputSlot = 0;

LLGL::VertexFormat myInstanceFmt;
myInstanceFmt.AppendAttribute({ "transform", 0, LLGL::Format::RGBA32Float, 1 });
myInstanceFmt.AppendAttribute({ "transform", 1, LLGL::Format::RGBA32Float, 1 });
myInstanceFmt.AppendAttribute({ "transform", 2, LLGL::Format::RGBA32Float, 1 });
myInstanceFmt.AppendAttribute({ "transform", 3, LLGL::Format::RGBA32Float, 1 });
myInstanceFmt.AppendAttribute({ "color",        LLGL::Format::RGBA8UNorm,  1 });
myInstanceFmt.inputSlot = 1;

LLGL::BufferDescriptor myVertexBufferDesc;
myVertexBufferDesc.vertexBuffer.format = myVertexFmt;
/* ... */

LLGL::BufferDescriptor myInstanceBufferDesc;
myInstanceBufferDesc.vertexBuffer.format = myInstanceFmt;
/* ... */

// Pass all vertex attributes to the vertex shader descriptor
LLGL::ShaderProgramDescriptor myShaderProgramDesc;
myShaderProgramDesc.vertexFormats = { myVertexFmt, myInstanceFmt };
/* ... */
```

After:
```cpp
// Interface:
std::vector<LLGL::VertexAttribute> LLGL::VertexShaderAttributes::inputAttribs;
std::vector<LLGL::VertexAttribute> LLGL::VertexShaderAttributes::outputAttribs;
LLGL::VertexShaderAttributes       LLGL::ShaderDescriptor::vertex;
LLGL::VertexShaderAttributes       LLGL::ShaderReflection::vertex;
std::vector<LLGL::VertexAttribute> LLGL::BufferDescriptor::vertexAttribs;
LLGL::Format                       LLGL::BufferDescriptor::indexFormat;

// Usage:
struct MyVertex {
    float position[3];       // attribute 0
    float normal[3];         // attribute 1
    float texCoord[2];       // attribute 2
};
struct MyInstance {
    float   transform[4][4]; // attribute 3...6
    uint8_t color[4];        // attribute 7
};

LLGL::VertexFormat myVertexFmt;
myVertexFmt.AppendAttribute({ "position", LLGL::Format::RGB32Float, 0 });
myVertexFmt.AppendAttribute({ "normal",   LLGL::Format::RGB32Float, 1 });
myVertexFmt.AppendAttribute({ "texCoord", LLGL::Format::RG32Float,  2 });
myVertexFmt.SetSlot(0);

LLGL::VertexFormat myInstanceFmt;
myInstanceFmt.AppendAttribute({ "transform", 0, LLGL::Format::RGBA32Float, 3, 1 });
myInstanceFmt.AppendAttribute({ "transform", 1, LLGL::Format::RGBA32Float, 4, 1 });
myInstanceFmt.AppendAttribute({ "transform", 2, LLGL::Format::RGBA32Float, 5, 1 });
myInstanceFmt.AppendAttribute({ "transform", 3, LLGL::Format::RGBA32Float, 6, 1 });
myInstanceFmt.AppendAttribute({ "color",        LLGL::Format::RGBA8UNorm,  7, 1 });
myInstanceFmt.SetSlot(1);

LLGL::BufferDescriptor myVertexBufferDesc;
myVertexBufferDesc.vertexAttribs = myVertexFmt.attributes;
/* ... */

LLGL::BufferDescriptor myInstanceBufferDesc;
myInstanceBufferDesc.vertexAttribs = myInstanceFmt.attributes;
/* ... */

// Pass all vertex attributes to the vertex shader descriptor
LLGL::ShaderDescriptor myVertexShaderDesc;
myVertexShaderDesc.vertex.inputAttribs = myVertexFmt.attributes;
myVertexShaderDesc.vertex.inputAttribs.insert(
    myVertexShaderDesc.vertex.inputAttribs.end(),
    myInstanceFmt.attributes.begin(),
    myInstanceFmt.attributes.end()
);
```


## Storage buffer description

The interleaved structure `BufferDescriptor::StorageBuffer` has been removed and a storage buffer is no described with the general purpose attributes.

Before:
```cpp
// Interface:
LLGL::StorageBufferType LLGL::BufferDescriptor::StorageBuffer::storageType;
LLGL::Format            LLGL::BufferDescriptor::StorageBuffer::format;
std::uint32_t           LLGL::BufferDescriptor::StorageBuffer::stride;

// Usage:
LLGL::BufferDescriptor myRWTypedBufferDesc;
myRWTypedBufferDesc.type                        = LLGL::BufferType::Storage;
myRWTypedBufferDesc.size                        = /* ... */
myRWTypedBufferDesc.storageBuffer.storageType   = LLGL::StorageBufferType::RWBuffer;
myRWTypedBufferDesc.storageBuffer.format        = LLGL::Format::RGBA32Float;
myRWTypedBufferDesc.storageBuffer.stride        = sizeof(float4)*4;

LLGL::BufferDescriptor myStructBufferDesc;
myStructBufferDesc.type                         = LLGL::BufferType::Storage;
myStructBufferDesc.size                         = /* ... */
myStructBufferDesc.storageBuffer.storageType    = LLGL::StorageBufferType::StructuredBuffer;
myStructBufferDesc.storageBuffer.format         = LLGL::Format::Undefined;
myStructBufferDesc.storageBuffer.stride         = sizeof(MyStruct);
```

After:
```cpp
// Interface:
std::uint32_t LLGL::BufferDescritpor::stride;
LLGL::Format  LLGL::BufferDescritpor::format;

// Usage:
LLGL::BufferDescriptor myRWTypedBufferDesc;
myRWTypedBufferDesc.size        = /* ... */
myRWTypedBufferDesc.stride      = sizeof(float4)*4;
myRWTypedBufferDesc.format      = LLGL::Format::RGBA32Float;
myRWTypedBufferDesc.bindFlags   = LLGL::BindFlags::Storage;

LLGL::BufferDescriptor myStructBufferDesc;
myStructBufferDesc.size         = /* ... */
myStructBufferDesc.stride       = sizeof(MyStruct);
myStructBufferDesc.format       = LLGL::Format::Undefined;
myStructBufferDesc.bindFlags    = LLGL::BindFlags::Sampled;
```


## Multi-sampling descriptor

`MultiSamplingDescriptor` has been removed and only the number of samples is now specified.
Additionally, the `sampleMask` member has been moved to `BlendDescriptor`.

Before:
```cpp
// Interface:
bool                          LLGL::MultiSamplingDescriptor::enabled;
std::uint32_t                 LLGL::MultiSamplingDescriptor::samples;
std::uint32_t                 LLGL::MultiSamplingDescriptor::sampleMask;
LLGL::MultiSamplingDescriptor LLGL::RasterizerDescriptor::multiSampling;
LLGL::MultiSamplingDescriptor LLGL::RenderTargetDescriptor::multiSampling;
LLGL::MultiSamplingDescriptor LLGL::RenderContextDescriptor::multiSampling;
```

After:
```cpp
// Interface:
std::uint32_t LLGL::RasterizerDescriptor::samples;
std::uint32_t LLGL::RenderTargetDescriptor::samples;
std::uint32_t LLGL::RenderContextDescriptor::samples;
std::uint32_t LLGL::BlendDescriptor::sampleMask;
```


## `ReadTexture` interface

The `ReadTexture` function of the `RenderSystem` interface now supports subresource regions.
Note that the number of MIP-map levels (i.e. `numMipLevels` attribute) must always be 1 for texture regions.

Before:
```cpp
// Interface:
void LLGL::RenderSystem::ReadTexture(
    LLGL::Texture&                  texture,
    std::uint32_t                   mipLevel,
    const LLGL::DstImageDescriptor& imageDesc
);

// Usage:
myRenderer->ReadTexture(*myTex, myMipLevel, myImageDesc);
```

After:
```cpp
// Interface:
void LLGL::RenderSystem::ReadTexture(
    LLGL::Texture&                  texture,
    const LLGL::TextureRegion       textureRegion,
    const LLGL::DstImageDescriptor& imageDesc
);

// Usage:
LLGL::TextureSubresource mySubresource {
    myArrayLayer, // base array layer
    myMipLevel    // Base MIP-map level
};
LLGL::TextureRegion myRegion {
    mySubresource,
    LLGL::Offset3D{ 0, 0, 0 },
    myTex->GetMipExtent(myMipLevel)
};
myRenderer->ReadTexture(*myTex, myRegion, myImageDesc);
```

## Stream-output interface

Stream-output buffers are now passed dynamically to the `BeginStreamOutput` function. There is no longer a `BufferArray` for stream-outputs.

Before:
```cpp
// Interface:
enum class LLGL::PrimitiveType;
void LLGL::CommandBuffer::SetStreamOutputBuffer(Buffer& buffer);
void LLGL::CommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray);
void LLGL::CommandBuffer::BeginStreamOutput();
void LLGL::CommandBuffer::EndStreamOutput();

// Usage:
myCmdBuffer->SetStreamOutputBufferArray(*mySOTargetsBufferArray);
myCmdBuffer->BeginStreamOutput(LLGL::PrimitiveType::Triangles);
/* ... */
myCmdBuffer->EndStreamOutput();
```

After:
```cpp
// Interface:
void LLGL::CommandBuffer::BeginStreamOutput(std::uint32_t numBuffers, Buffer* const * buffers);
void LLGL::CommandBuffer::EndStreamOutput();

// Usage:
myCmdBuffer->BeginStreamOutput(2, mySOTargets);
/* ... */
myCmdBuffer->EndStreamOutput();
```

## Pipeline state interface

Graphics and compute pipelines have been merged into one interface. `GraphicsPipeline` and `ComputePipeline` interfaces have been replaced by `PipelineState`.

Before:
```cpp
// Interface:
LLGL::GraphicsPipeline* LLGL::RenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc);
LLGL::ComputePipeline*  LLGL::RenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc);
void                    LLGL::CommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline);
void                    LLGL::CommandBuffer::SetComputePipeline(ComputePipeline& computePipeline);

// Usage:
LLGL::GraphicsPipeline* myGfxPipeline = myRenderer->CreateGraphicsPipeline(/* ... */);
/* ... */
myCmdBuffer->SetGraphicsPipeline(*myGfxPipeline);
```

After:
```cpp
// Interface:
LLGL::PipelineState*    LLGL::RenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& desc);
LLGL::PipelineState*    LLGL::RenderSystem::CreatePipelineState(const ComputePipelineDescriptor& desc);
void                    LLGL::CommandBuffer::SetPipelineState(PipelineState& pipelineState);

// Usage:
LLGL::PipelineState* myGfxPipeline = myRenderer->CreatePipelineState(/* ... */);
/* ... */
myCmdBuffer->SetPipelineState(*myGfxPipeline);
```


## Removed features

The following features/functions have been removed:
- The `Shader::Disassemble` function and `ShaderDisassembleFlags` enumeration have been removed. LLGL does not provide shader cross compilation or disassembling.
- The compressed RGB format `Format::BC1RGB`. Use `Format::BC1UNorm` instead (for RGBA).
- `StreamOutputFormat` has been replaced by `VertexFormat`
- `StreamOutputAttribute` has been replaced by `VertexAttribute`
- `PrimitiveType` enumeration has been replaced by `PrimitiveTopology`



