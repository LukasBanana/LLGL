ChangeLog v0.02
===============

Vertex layout in `ShaderProgram` interface:
--------------------------

Before:
```cpp
// Interface:
void ShaderProgram::BuildInputLayout(const VertexFormat& vertexFormat);

// Usage:
LLGL::VertexFormat myVertexFormat;
/* ... */
myShaderProgram->BuildInputLayout(myVertexFormat);
```

After:
```cpp
// Interface:
void ShaderProgram::BuildInputLayout(std::uint32_t numVertexFormats, const VertexFormat* vertexFormats);

// Usage:
LLGL::VertexFormat myVertexFormat;
/* ... */
myShaderProgram->BuildInputLayout(1, &myVertexFormat);
```


`LogicOp` usage:
----------------

Before:
```cpp
// Interface:
LogicOp GraphicsAPIDependentStateDescriptor::StateOpenGLDescriptor::logicOp;

// Usage:
LLGL::GraphicsAPIDependentStateDescriptor myGfxDependentState;
myGfxDependentState.stateOpenGL.logicOp = /* ... */;
myRenderer->SetGraphicsAPIDependentState(myGfxDependentState);
```

After:
```cpp
// Interface:
LogicOp BlendDescriptor::logicOp;

// Usage:
LLGL::GraphicsPipelineDescriptor myGfxPipelineDesc;
/* ... */
myGfxPipelineDesc.blend.logicOp = /* ... */;
myRenderer->CreateGraphicsPipeline(myGfxPipelineDesc);
```


Rasterization line width (OpenGL and Vulkan only):
--------------------------------------------------

Before:
```cpp
// Interface:
float GraphicsAPIDependentStateDescriptor::lineWidth;

// Usage:
LLGL::GraphicsAPIDependentStateDescriptor myGfxDependentState;
myGfxDependentState.stateOpenGL.lineWidth = /* ... */;
myRenderer->SetGraphicsAPIDependentState(myGfxDependentState);
```

After:
```cpp
// Interface:
float RasterizerDescriptor::lineWidth;

// Usage:
LLGL::GraphicsPipelineDescriptor myGfxPipelineDesc;
/* ... */
myGfxPipelineDesc.rasterizer.lineWidth = /* ... */;
myRenderer->CreateGraphicsPipeline(myGfxPipelineDesc);
```


OpenGL profile selection:
-------------------------

Before:
```cpp
// Interface:
bool          ProfileOpenGLDescriptor::extProfile;
bool          ProfileOpenGLDescriptor::coreProfile;
bool          ProfileOpenGLDescriptor::debugDump;
OpenGLVersion ProfileOpenGLDescriptor::version;

// Usage:
LLGL::RenderContextDescriptor myContextDesc;
/* ... */
myContextDesc.profileOpenGL.extProfile  = true;
myContextDesc.profileOpenGL.coreProfile = true;
myContextDesc.profileOpenGL.debugDump   = true;
myContextDesc.profileOpenGL.version     = LLGL::OpenGLVersion::OpenGL_4_6;
myRenderer->CreateRenderContext(myContextDesc);
```

After:
```cpp
// Interface:
OpenGLContextProfile ProfileOpenGLDescriptor::contextProfile;
int                  ProfileOpenGLDescriptor::majorVersion;
int                  ProfileOpenGLDescriptor::minorVersion;

// Usage:
LLGL::RenderContextDescriptor myContextDesc;
/* ... */
myContextDesc.profileOpenGL.contextProflie = LLGL::OpenGLContextProfile::CoreProfile;
myContextDesc.profileOpenGL.majorVersion   = 4;
myContextDesc.profileOpenGL.minorVersion   = 6;
myRenderer->CreateRenderContext(myContextDesc);
```


Query texture descriptor:
-------------------------

Before:
```cpp
// Interface:
TextureDescriptor RenderSystem::QueryTextureDescriptor(const Texture& texture);

// Usage:
auto myTextureDesc = myRenderer->QueryTextureDescriptor(myTexture);
```

After:
```cpp
// Interface:
TextureDescriptor Texture::QueryDesc() const;

// Usage:
auto myTextureDesc = myTexture->QueryDesc();
```


Render target attachments:
--------------------------

***Under Construction***

Before:
```cpp
// Interface:
unsigned int  RenderTargetAttachmentDescriptor::mipLevel;
unsigned int  RenderTargetAttachmentDescriptor::layer;
AxidDirection RenderTargetAttachmentDescriptor::cubeFace;

MultiSamplingDescriptor RenderTargetDescriptor::multiSampling;
bool                    RenderTargetDescriptor::customMultiSampling;

void RenderTarget::AttachDepthBuffer(const Gs::Vector2ui& size);
void RenderTarget::AttachStencilBuffer(const Gs::Vector2ui& size);
void RenderTarget::AttachDepthStencilBuffer(const Gs::Vector2ui& size);
void RenderTarget::AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc);
void RenderTarget::DetachAll();

// Usage:
// Create render target
LLGL::RenderTargetDescriptor myRenderTargetDesc;
/* myRenderTargetDesc ... */
auto myRenderTarget = myRenderer->CreateRenderTarget(myRenderTargetDesc);

// Attach depth buffer
/* myRenderTargetSize ... */
myRenderTarget->AttachDepthBuffer(myRenderTargetSize);

// Attach color texture
LLGL::RenderTargetAttachmentDescriptor myAttachmentDesc;
/* myAttachmentDesc ... */
myRenderTarget->AttachTexture(myColorTexture, myAttachmentDesc);
```

After:
```cpp
// Interface:
AttachmentType AttachmentDescriptor::type;
Texture*       AttachmentDescriptor::texture;
uint32_t       AttachmentDescriptor::width;
uint32_t       AttachmentDescriptor::height;
uint32_t       AttachmentDescriptor::mipLevel;
uint32_t       AttachmentDescriptor::layer;
AxisDirection  AttachmentDescriptor::cubeFace;

vector<AttachmentDescriptor> RenderTargetDescriptor::attachments;
MultiSamplingDescriptor      RenderTargetDescriptor::multiSampling;
bool                         RenderTargetDescriptor::customMultiSampling;

// Usage:
LLGL::RenderTargetDescriptor myRenderTargetDesc;

myRenderTargetDesc.attachments.resize(2);

// Specify depth and color attachments
/* myRenderTargetSize ... */
myRenderTargetDesc.attachments =
{
    LLGL::AttachmentDesc(LLGL::AttachmentType::Depth, myRenderTargetSize.x, myRenderTargetSize.y),
    LLGL::AttachmentDesc(LLGL::AttachmentType::Color, myColorTexture)
};

// Create render target
auto myRenderTarget = myRenderer->CreateRenderTarget(myRenderTargetDesc);
```


Texture initialization with `ImageDescriptor` interface:
--------------------------------------------------------

Before:
```cpp
// Interface:
ImageFormat ImageDescriptor::format;
DataType    ImageDescriptor::dataType;
const void* ImageDescriptor::buffer;
uint32_t    ImageDescriptor::compressedSize;

// Usage (for uncompressed images):
LLGL::ImageDescriptor myImageDesc;
myImageDesc.format   = LLGL::ImageFormat::RGBA;
myImageDesc.dataType = LLGL::DataType::UInt8;
myImageDesc.buffer   = myImageBuffer;

// Usage (for compressed images):
LLGL::ImageDescriptor myImageDesc;
myImageDesc.format         = LLGL::ImageFormat::CompressedRGBA;
myImageDesc.buffer         = myImageBuffer;
myImageDesc.compressedSize = myImageBufferSize;
```

After:
```cpp
// Interface:
ImageFormat ImageDescriptor::format;
DataType    ImageDescriptor::dataType;
const void* ImageDescriptor::data;
size_t      ImageDescriptor::dataSize;

// Usage (for both compressed and uncompressed images):
LLGL::ImageDescriptor myImageDesc;
myImageDesc.format   = /* ... */;
myImageDesc.dataType = /* ... */;
myImageDesc.data     = myImageBuffer;
myImageDesc.dataSize = myImageBufferSize;
```


Dynamic state access for shader resources:
------------------------------------------

Dynamic state access for constant buffers, storage buffers, textures, and samplers is only supported for the legacy rendering APIs auch as OpenGL and Direct3D 11.
For the new APIs such as Direct3D 12 and Vulkan the `ResourceViewHeap` interface can be used to bind all shader resources for a graphics pipeline at once.

Before:
```cpp
// Interface:
class CommandBuffer {
    void SetConstantBuffer(/* ... */);
    /* ... */
};

// Usage:
auto myCmdBuffer = myRenderer->CreateCommandBuffer();
/* ... */
myCmdBuffer->SetConstantBuffer(/* ... */);
myCmdBuffer->SetConstantBufferArray(/* ... */);
myCmdBuffer->SetStorageBuffer(/* ... */);
myCmdBuffer->SetStorageBufferArray(/* ... */);
myCmdBuffer->SetTexture(/* ... */);
myCmdBuffer->SetTextureArray(/* ... */);
myCmdBuffer->SetSampler(/* ... */);
myCmdBuffer->SetSamplerArray(/* ... */);
```

After:
```cpp
// Interface:
class CommandBufferExt : public CommandBuffer {
    void SetConstantBuffer(/* ... */);
    /* ... */
};

// Usage (only supported with OpenGL and Direct3D 11):
auto myCmdBufferExt = myRenderer->CreateCommandBufferExt();
/* ... */
myCmdBufferExt->SetConstantBuffer(/* ... */);
myCmdBufferExt->SetConstantBufferArray(/* ... */);
myCmdBufferExt->SetStorageBuffer(/* ... */);
myCmdBufferExt->SetStorageBufferArray(/* ... */);
myCmdBufferExt->SetTexture(/* ... */);
myCmdBufferExt->SetTextureArray(/* ... */);
myCmdBufferExt->SetSampler(/* ... */);
myCmdBufferExt->SetSamplerArray(/* ... */);
```







