ChangeLog v0.02
===============

`Shader` interface:
-------------------

There is no longer the possibility to compile the shader source or load binary shaders from the `Shader` interface. Instead the compilation process is done by the `CreateShader` function.

Before:
```cpp
// Interface:
bool Shader::Compile(const std::string&, const ShaderDescriptor&);
bool Shader::LoadBinary(std::vector<char>&&, const ShaderDescriptor&);

// Usage:
LLGL::Shader* myShader = myRenderer->CreateShader(LLGL::ShaderType::Vertex);
myShader->Compile(myShaderSource, { "VMain", "vs_4_0" });
```

After:
```cpp
// Interface:
bool Shader::HasErrors() const;

// Usage:
LLGL::ShaderDescritpor myShaderDesc;
myShaderDesc.type       = LLGL::ShaderType::Vertex;
myShaderDesc.source     = myShaderSource.c_str();
myShaderDesc.sourceSize = myShaderSource.size();
myShaderDesc.entryPoint = "VMain";
myShaderDesc.profile    = "vs_4_0";
LLGL::Shader* myShader = myRenderer->CreateShader(myShaderDesc);

// Usage (loading from file):
LLGL::Shader* myShader = myRenderer->CreateShader({ LLGL::ShaderType::Vertex, "myShader.hlsl", "VMain", "vs_4_0" });
```


`ShaderProgram` interface:
--------------------------

Shaders can no longer be attached or detached after a shader program has been created. All shader linking is done at creation time.

Before:
```cpp
// Interface:
void ShaderProgram::AttachShader(Shader&);
void ShaderProgram::DetachAll();
bool ShaderProgram::LinkShaders();
void ShaderProgram::BuildInputLayout(const VertexFormat& vertexFormat);

// Usage:
myShaderProgram->BuildInputLayout(myVertexFormat);

myShaderProgram->AttachShader(*myVertexShader);
myShaderProgram->AttachShader(*myFragmentShader);

if (!myShaderProgram->LinkShaders()) {
    throw std::runtime_error(myShaderProgram->QueryInfoLog());
}
```

After:
```cpp
// Interface:
bool ShaderProgram::HasErrors();

// Usage:
LLGL::ShaderProgramDescriptor myProgramDesc;

myProgramDesc.vertexFormats  = { myVertexFormat };
myProgramDesc.vertexShader   = myVertexShader;
myProgramDesc.fragmentShader = myFragmentShader;

myShaderProgram = myRenderer->CreateShaderProgram(myProgramDesc);

if (myShaderProgram->HasErrors()) {
    throw std::runtime_error(myShaderProgram->QueryInfoLog());
}
```


`LogicOp` usage:
----------------

Logic pixel operation parameter has been moved from `GraphicsAPIDependentStateDescriptor` into `BlendDescriptor` of a graphics pipeline.

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

Line width rasterization parameter has been moved from `GraphicsAPIDependentStateDescriptor` into `RasterizerDescriptor` of a graphics pipeline.

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

The OpenGL profile can now be selected with dynamic version numbers instead of fixed enumeration entries. For the profile type, the boolean members `extProfile` and `coreProfile` have been replaced by the enumeration `OpenGLContextProfile`.

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

The function to query the `TextureDescriptor` from a `Texture` object has been moved from the `RenderSystem` interface into the `Texture` interface.

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

Render target attachments are now defined by the `RenderTargetDescriptor` structure when the render target is created. Instead of detaching all attachments (i.e. `DetachAll`), release the render target and create a new one.

Dynamic attachment functions have been removed such as `AttachTexture` and `DetachAll`.

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
Gs::Vector2ui myRenderTargetSize = { 512, 512 };
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
uint32_t       AttachmentDescriptor::mipLevel;
uint32_t       AttachmentDescriptor::layer;
AxisDirection  AttachmentDescriptor::cubeFace;

Extent2D                     RenderTargetDescriptor::resolution;
MultiSamplingDescriptor      RenderTargetDescriptor::multiSampling;
bool                         RenderTargetDescriptor::customMultiSampling;
vector<AttachmentDescriptor> RenderTargetDescriptor::attachments;

// Usage:
LLGL::RenderTargetDescriptor myRenderTargetDesc;

// Specify render-target resolution
myRenderTargetDesc.resolution = { 512, 512 };

// Specify depth and color attachments
myRenderTargetDesc.attachments = {
    LLGL::AttachmentDesc(LLGL::AttachmentType::Depth),
    LLGL::AttachmentDesc(LLGL::AttachmentType::Color, myColorTexture)
};

// Create render target
auto myRenderTarget = myRenderer->CreateRenderTarget(myRenderTargetDesc);
```


Texture initialization with `ImageDescriptor` interface:
--------------------------------------------------------

Member `buffer` has been renamed to `data` to be consistent with the nomenclature (like in the `RenderSystem::ReadTexture` function for instance. Member `compressedSize` has been renamed to `dataSize` to be used for robustness checks and not only for compressed image data.

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
ImageFormat SrcImageDescriptor::format;
DataType    SrcImageDescriptor::dataType;
const void* SrcImageDescriptor::data;
size_t      SrcImageDescriptor::dataSize;

// Usage (for both compressed and uncompressed images):
LLGL::SrcImageDescriptor myImageDesc;
myImageDesc.format   = /* ... */;
myImageDesc.dataType = /* ... */;
myImageDesc.data     = myImageBuffer;
myImageDesc.dataSize = myImageBufferSize;
```


Dynamic state access for shader resources:
------------------------------------------

Dynamic state access for constant buffers, storage buffers, textures, and samplers is only supported for the legacy rendering APIs auch as OpenGL and Direct3D 11. For the new APIs such as Direct3D 12 and Vulkan the `ResourceViewHeap` interface can be used to bind all shader resources for a graphics pipeline at once.

Before:
```cpp
// Interface:
class CommandBuffer {
    /* ... */
    void SetConstantBuffer(/* ... */);
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
    /* ... */
    void SetConstantBuffer(/* ... */);
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


Clear specific attachments of active render target:
---------------------------------------------------

Now multiple attachments (both color and depth-stencil) of the active render target *can* be cleared at once.

Before:
```cpp
// Interface:
void CommandBuffer::Clear(long flags);
void CommandBuffer::ClearTarget(uint32_t targetIndex, const ColorRGBAf& color);

// Usage:
myCmdBuffer->ClearTarget(0, LLGL::ColorRGBAf{ /* ... */ }); // Clear color attachment 0
myCmdBuffer->ClearTarget(1, LLGL::ColorRGBAf{ /* ... */ }); // Clear color attachment 1
myCmdBuffer->SetClearDepth(1.0f);
myCmdBuffer->Clear(LLGL::ClearFlags::Depth);                // Clear depth attachment
```

After:
```cpp
// Interface:
long       AttachmentClear::flags;
uint32_t   AttachmentClear::colorAttachment;
ColorRGBAf AttachmentClear::clearValue::color;
float      AttachmentClear::clearValue::depth;
uint32_t   AttachmentClear::clearValue::stencil;

void CommandBuffer::Clear(long flags);
void CommandBuffer::ClearAttachments(uint32_t numAttachments, const AttachmentClear* attachments);

// Usage:
LLGL::AttachmentClear myClearCmds[3] =
{
    LLGL::AttachmentClear { LLGL::ColorRGBAf { /* ... */ }, 0 }, // Clear color attachment 1
    LLGL::AttachmentClear { LLGL::ColorRGBAf { /* ... */ }, 1 }, // Clear color attachment 1
    LLGL::AttachmentClear { 1.0f }                               // Clear depth attachment
}
myCmdBuffer->ClearAttachments(3, myClearCmds);
```


Viewport and scissor arrays:
----------------------------

Functions to set viewport and scissor arrays have been renamed for a persistent nomenclature. A `Set...Array` function in the `CommandBuffer` interface is only used for `...Array` objects (such as `BufferArray`). For basic C/C++ arrays functions like `SetViewports`, `SetScissors`, or `ClearAttachments` are used.

Before:
```cpp
void SetViewportArray(uint32_t numViewports, const Viewport* viewportArray);
void SetScissorArray(uint32_t numScissors, const Scissors* scissorArray);
```

After:
```cpp
void SetViewports(uint32_t numViewports, const Viewport* viewports);
void SetScissors(uint32_t numScissors, const Scissors* scissors);
```


Persistent states in `CommandBuffer` interface:
-----------------------------------------------

Viewports and scissors are no longer guaranteed to be a persistent state. They must be updated after a new render target is set.


Depth biasing in `RasterizerDescriptor` interface:
--------------------------------------------------

The rasterizer parameters for depth biasing have been moved into a separate structure named `DepthBiasDescriptor`.

Before:
```cpp
// Interface:
float RasterizerDescriptor::depthBiasConstantFactor;
float RasterizerDescriptor::depthBiasSlopeFactor;
float RasterizerDescriptor::depthBiasClamp;
```

After:
```cpp
// Interface:
float               DepthBiasDescriptor::constantFactor;
float               DepthBiasDescriptor::slopeFactor;
float               DepthBiasDescriptor::clamp;
DepthBiasDescriptor RasterizerDescriptor::depthBias;
```


`ShaderUniform` interface:
--------------------------

All string parameters have been changed from `const std::string&` to `const char*`, and all vector and matrix types from the GaussianLib have been replaced by pointers to base types.

Before:
```cpp
// Interface (brief selection):
class ShaderUniform {
    /* ... */
    void SetUniform(const std::string& name, const float value);
    void SetUniform(const std::string& name, const Gs::Vector2f& value);
    void SetUniform(const std::string& name, const Gs::Matrix4f& value);
    void SetUniformArray(const std::string& name, const Gs::Matrix4f& value, std::size_t count);
};

// Usage:
if (auto myUniforms = myShaderProgram->LockShaderUniforms()) {
    myUniforms->SetUniform("myUniformVec2", { 1.0f, 2.0f });
    myUniforms->SetUniform("myUniformMat4", myWorldMatrix);
    myShaderProgram->UnlockShaderUniform();
}
```

After:
```cpp
// Interface (brief selection):
class ShaderUniform {
    /* ... */
    void SetUniform1f(const char* name, float value);
    void SetUniform2f(const char* name, float value0, float value1);
    void SetUniform4x4fv(const char* name, const float* value, std::size_t count = 1);
};

// Usage:
if (auto myUniforms = myShaderProgram->LockShaderUniforms()) {
    myUniforms->SetUniform2f("myUniformVec2", 1.0f, 2.0f);
    myUniforms->SetUniform4x4fv("myUniformMat4", &myWorldMatrix[0]);
    myShaderProgram->UnlockShaderUniform();
}
```


Dependency removal of GaussianLib:
----------------------------------

All dependencies to the GaussianLib project have been removed from the project (except the tutorials and tests!).

Before:
```cpp
// Usage:
#include <LLGL/ColorRGB.h>
#include <LLGL/ColorRGBA.h>
LLGL::ColorRGB   myRealTypeColorRGB;
LLGL::ColorRGBA  myRealTypeColorRGBA;
LLGL::ColorRGBAf myUninitializedColor{ Gs::UninitializeTag{} };

// Usage (with breaking change):
LLGL::ColorRGBAf myColorA, myColorB;
if (myColorA == myColorB) {
    // uses 'Gs::Equals' with 'std::abs' to circumvent rounding errors
}
```

After:
```cpp
// Usage:
#include <LLGL/ColorRGB.h>
#include <LLGL/ColorRGBA.h>
#include <Gauss/Real.h>
LLGL::ColorRGBT<Gs::Real>  myRealTypeColorRGB;
LLGL::ColorRGBAT<Gs::Real> myRealTypeColorRGBA;
LLGL::ColorRGBAf           myUninitializedColor{ LLGL::UninitializeTag{} };

// Usage (with breaking change):
LLGL::ColorRGBAf myColorA, myColorB;
if (myColorA == myColorB) {
    // uses 'operator ==' of float type for comparison
}
```


Renamed identifiers:
--------------------

Various structures, enumerations, and fields have been renamed to either fit LLGL's nomenclature or to simplify their identifiers when they are frequently used.

Before:
```cpp
LLGL::ShaderStageFlags;
LLGL::BlendTargetDescriptor::destColor;
LLGL::BlendTargetDescriptor::destAlpha;
LLGL::BlendOp::DestColor;
LLGL::BlendOp::InvDestColor;
LLGL::BlendOp::DestAlpha;
LLGL::BlendOp::InvDestAlpha;
LLGL::ImageDescriptor;
LLGL::BufferCPUAccess;
LLGL::TextureFilter;
LLGL::TextureWrap;
```

After:
```cpp
LLGL::StageFlags;
LLGL::BlendTargetDescriptor::dstColor;
LLGL::BlendTargetDescriptor::dstAlpha;
LLGL::BlendOp::DstColor;
LLGL::BlendOp::InvDstColor;
LLGL::BlendOp::DstAlpha;
LLGL::BlendOp::InvDstAlpha;
LLGL::SrcImageDescriptor;
LLGL::CPUAccess;
LLGL::SamplerFilter;
LLGL::SamplerAddressMode;
```


Graphics API dependent state:
-----------------------------

The common graphics API dependent state structure `LLGL::GraphicsAPIDependentStateDescriptor` has been replaced by individual structures for the respective rendering API.

Before:
```cpp
// Interface:
bool LLGL::GraphicsAPIDependentStateDescriptor::StateOpenGL::screenSpaceOriginLowerLeft;
bool LLGL::GraphicsAPIDependentStateDescriptor::StateOpenGL::invertFrontFace;

// Usage:
LLGL::GraphicsAPIDependentStateDescriptor myStateDesc;
myStateDesc.stateOpenGL.screenSpaceOriginLowerLeft = true;
myStateDesc.stateOpenGL.invertFrontFace            = true;
myCommandBuffer->SetGraphicsAPIDependentState(myStateDesc);
```

After:
```cpp
// Interface:
bool LLGL::OpenGLDependentStateDescriptor::originLowerLeft;
bool LLGL::OpenGLDependentStateDescriptor::invertFrontFace;

// Usage:
LLGL::OpenGLDependentStateDescriptor myStateDesc;
myStateDesc.originLowerLeft = true;
myStateDesc.invertFrontFace = true;
myCommandBuffer->SetGraphicsAPIDependentState(&myStateDesc, sizeof(myStateDesc));
```


`TextureFormat` and `VectorType` enumerations:
----------------------------------------------

Both `TextureFormat` and `VectorType` enumerations have been merged into the new `Format` enumeration.

Before:
```cpp
// Usage:
LLGL::TextureDescriptor myTextureDesc;
myTextureDesc.format = LLGL::TextureFormat::RGBA8; // 8-bit normalized unsigned byte format

LLGL::VertexAttribute myVertexPositionAttrib { "myPosition", LLGL::VectorType::Float3 }; // 32-bit floating-point format
LLGL::VertexAttribute myVertexColorAttrib    { "myColor",    LLGL::VectorType::Float4 }; // 32-bit floating-point format
LLGL::VertexAttribute myVertexFlagsAttrib    { "myFlags",    LLGL::VectorType::UInt   }; // 32-bit unsigned integer format
```

After:
```cpp
// Usage:
LLGL::TextureDescriptor myTextureDesc;
myTextureDesc.format = LLGL::TextureFormat::RGBA8UNorm; // 8-bit normalized unsigned byte format

LLGL::VertexAttribute myVertexPositionAttrib { "myPosition", LLGL::Format::RGB32Float }; // 32-bit floating-point format
LLGL::VertexAttribute myVertexColorAttrib    { "myColor",    LLGL::Format::RGBA8UNorm }; // 8-bit normalized unsigned byte format
LLGL::VertexAttribute myVertexFlagsAttrib    { "myFlags",    LLGL::Format::R32UInt    }; // 32-bit unsigned integer format
```


`TextureDescriptor` struct:
---------------------------

The descriptor structure `TextureDescriptor` has been simplified and generalized. All nested structures have been removed.

Before:
```cpp
// Usage:
LLGL::TextureDescriptor my2DTexDesc;
my2DTexDesc.type             = LLGL::TextureType::Texture2DArray;
my2DTexDesc.format           = LLGL::TextureFormat::RGBA8;
my2DTexDesc.texture2D.width  = 512;
my2DTexDesc.texture2D.height = 512;
my2DTexDesc.texture2D.layers = 4;
LLGL::Texture* my2DTex = myRenderer->CreateTexture(my2DTexDesc);

LLGL::TextureDescriptor my3DTexDesc;
my3DTexDesc.type             = LLGL::TextureType::Texture3D;
my3DTexDesc.format           = LLGL::TextureFormat::R32Float;
my3DTexDesc.texture3D.width  = 16;
my3DTexDesc.texture3D.height = 16;
my3DTexDesc.texture3D.depth  = 16;
LLGL::Texture* my3DTex = myRenderer->CreateTexture(my3DTexDesc);
```

After:
```cpp
// Usage:
LLGL::TextureDescriptor my2DTexDesc;
my2DTexDesc.type   = LLGL::TextureType::Texture2DArray;
my2DTexDesc.format = LLGL::Format::RGBA8UNorm;
my2DTexDesc.width  = 512;
my2DTexDesc.height = 512;
my2DTexDesc.layers = 4;
LLGL::Texture* my2DTex = myRenderer->CreateTexture(my2DTexDesc);

LLGL::TextureDescriptor my3DTexDesc;
my3DTexDesc.type   = LLGL::TextureType::Texture3D;
my3DTexDesc.format = LLGL::Format::R32Float;
my3DTexDesc.width  = 16;
my3DTexDesc.height = 16;
my3DTexDesc.depth  = 16;
LLGL::Texture* my3DTex = myRenderer->CreateTexture(my3DTexDesc);
```


Removal of `TextureArray` and `SamplerArray` interfaces:
--------------------------------------------------------

The `TextureArray` and `SamplerArray` interfaces have been removed. The new `ResourceHeap` interface can be used instead.

Before:
```cpp
// Usage:
LLGL::Texture* myTextures[] = { myTex0, myTex1 };
LLGL::TextureArray* myTextureArray = myRenderer->CreateTextureArray(2, myTextures);

LLGL::Sampler* mySamplers[] = { mySmp0, mySmp1 };
LLGL::SamplerArray* mySamplerArray = myRenderer->CreateSamplerArray(2, mySamplers);

myCmdBuffer->SetTextureArray(*myTextureArray, 2, LLG::StageFlags::FragmentStage);
myCmdBuffer->SetSamplerArray(*mySamplerArray, 7, LLG::StageFlags::FragmentStage);
```

After:
```cpp
LLGL::PipelineLayoutDescriptor myLayoutDesc;
myLayoutDesc.bindings = {
    LLGL::BindingDescriptor { LLGL::ResourceType::Texture, LLG::StageFlags::FragmentStage, 2 },
    LLGL::BindingDescriptor { LLGL::ResourceType::Texture, LLG::StageFlags::FragmentStage, 3 },
    LLGL::BindingDescriptor { LLGL::ResourceType::Sampler, LLG::StageFlags::FragmentStage, 7 },
    LLGL::BindingDescriptor { LLGL::ResourceType::Sampler, LLG::StageFlags::FragmentStage, 8 },
};
LLGL::PipelineLayout* myPipelineLayout = myRenderer->CreatePipelineLayout(myLayoutDesc);

LLGL::ResourceHeapDescriptor myHeapDesc;
myHeapDesc.pipelineLayout = myPipelineLayout;
myHeapDesc.resourceViews  = { myTex0, myTex1, mySmp0, mySmp1 };
LLGL::ResourceHeap* myResourceHeap = myRenderer->CreateResourceHeap(myHeapDesc);

myCmdBuffer->SetGraphicsResourceHeap(*myResourceHeap);
```






