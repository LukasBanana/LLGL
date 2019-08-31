# ChangeLog v0.03

## Table of Contents

- [`BufferDescriptor` interface](#bufferdescriptor-interface)
- [`TextureDescriptor` interface](#texturedescriptor-interface)
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
                              LLGL::BindFlags::SampleBuffer    |
                              LLGL::BindFlags::RWStorageBuffer;
myTexDesc.cpuAccessFlags    = 0;
myTexDesc.miscFlags         = LLGL::MiscFlags::FixedSamples;
```


## Index buffer format

The class `IndexFormat` has been removed and replaced by the `Format` enum. The only valid index formats are `Format::R16UInt` (16-bit) and `Format::R32UInt` (32-bit). An 8-bit index format is no longer supported (OpenGL was the only renderer anyways that supported it).

Before:
```cpp
// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.type               = LLGL::BufferType::Index;
myBufferDesc.indexBuffer.format = LLGL::IndexFormat(LLGL::DataType::UInt16);
/* ... */
```

After:
```cpp
// Usage:
LLGL::BufferDescriptor myBufferDesc;
myBufferDesc.bindFlags          = LLGL::BindFlags::IndexBuffer;
myBufferDesc.indexBuffer.format = LLGL::Format::R16UInt;
/* ... */
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
All other uniforms (matrices and other parameters for instances) must be set with the new `SetUniform`/`SetUniforms` functions each time a graphics pipeline is bound.
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
LLGL::UniformLocation myProjectionUniform = myShaderProgram->QueryUniformLocation("myProjection");

/* ... */

myCmdBuffer->SetGraphicsPipeline(myGfxPipeline);
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
try
{
    LLGL::ShaderReflectionDescriptor reflection = myShaderProgram->QueryReflectionDesc();
    /* Evaluate ... */
}
catch (const std::exception& e)
{
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
if (myShaderProgram->Reflect(reflection))
{
    /* Evaluate ... */
}
else
{
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
BindFlags::SampleBuffer                --> BindFlags::Sampled
BindFlags::RWStorageBuffer             --> BindFlags::Storage
CommandBuffer::SetSampleBuffer         --> CommandBuffer::SetSampledBuffer
CommandBuffer::SetRWStorageBuffer      --> CommandBuffer::SetStorageBuffer
FrameProfile::sampleBufferBindings     --> FrameProfile::sampledBufferBindings
FrameProfile::rwStorageBufferBindings  --> FrameProfile::storageBufferBindings
RenderingFeatures::hasCommandBufferExt --> RenderingFeatures::hasDirectResourceBinding
Resource::QueryResourceType            --> Resource::GetResourceType
Display::QueryList                     --> Display::InstantiateList
Display::QueryPrimary                  --> Display::InstantiatePrimary
Display::QuerySupportedDisplayModes    --> Display::GetSupportedDisplayModes
Image::QuerySrcDesc                    --> Display::GetSrcDesc
Image::QueryDstDesc                    --> Display::GetDstDesc
RenderContext::QueryColorFormat        --> RenderContext::GetColorFormat
RenderContext::QueryDepthStencilFormat --> RenderContext::GetDepthStencilFormat
Texture::QueryDesc                     --> Texture::GetDesc
Texture::QueryMipExtent                --> Texture::GetMipExtent
ShaderProgram::QueryUniformLocation    --> ShaderProgram::FindUniformLocation
ShaderProgram::QueryInfoLog            --> ShaderProgram::GetReport
```


## `PipelineLayoutDesc` syntax

Besides additions to the syntax of the `PipelineLayoutDesc` utility function, the following breaking change has been made:
The identifier `"sbuffer"` was renamed to `"buffer"`.


## `Format` information

Various functions to get meta data from a `Format` enumeration entry have been combined into a single function named `GetFormatDesc`.

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
```
// Interface:
const LLGL::FormatDescriptor& GetFormatDesc(const LLGL::Format format);

// Usage:
const auto& myFormatDesc = LLGL::GetFormatDesc(myFormat);
myTextureByteSize  = myTexturePixelCount * myFormatDesc.bitSize / myFormatDesc.blockWidth / myFormatDesc.blockHeight / 8;
myFormatComponents = myFormatDesc.components;
myImageFormat      = myFormatDesc.format;
myDataType         = myFormatDesc.dataType;
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
myCmdBuffer->SetResource(*myConstBuffer, 0, LLGL::BindFlags::ConstantBuffer, LLGL::BindFlags::VertexStage);
myCmdBuffer->SetResource(*myColorMap, 1, LLGL::BindFlags::Sampled, LLGL::BindFlags::FragmentStage);
```


## Removed features

The following features/functions have been removed:
- The `Shader::Disassemble` function and `ShaderDisassembleFlags` enumeration have been removed. LLGL does not provide shader cross compilation or disassembling.




