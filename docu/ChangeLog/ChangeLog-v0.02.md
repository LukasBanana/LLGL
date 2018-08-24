# ChangeLog v0.02

## Table of Contents

- [`Shader` interface](#shader-interface)
- [`ShaderProgram` interface](#shaderprogram-interface)
- [`LogicOp` usage](#logicop-usage)
- [Rasterization line width (OpenGL and Vulkan only)](#rasterization-line-width-opengl-and-vulkan-only)
- [OpenGL profile selection](#opengl-profile-selection)
- [Query texture descriptor](#query-texture-descriptor)
- [Render target attachments](#render-target-attachments)
- [Texture initialization with `ImageDescriptor` interface](#texture-initialization-with-imagedescriptor-interface)
- [Dynamic state access for shader resources](#dynamic-state-access-for-shader-resources)
- [Clear specific attachments of active render target](#clear-specific-attachments-of-active-render-target)
- [Viewport and scissor arrays](#viewport-and-scissor-arrays)
- [Persistent states in `CommandBuffer` interface](#persistent-states-in-commandbuffer-interface)
- [Depth biasing in `RasterizerDescriptor` interface](#depth-biasing-in-rasterizerdescriptor-interface)
- [`ShaderUniform` interface](#shaderuniform-interface)
- [Dependency removal of GaussianLib](#dependency-removal-of-gaussianlib)
- [Renamed identifiers](#renamed-identifiers)
- [Graphics API dependent state](#graphics-api-dependent-state)
- [`TextureFormat` and `VectorType` enumerations](#textureformat-and-vectortype-enumerations)
- [`TextureDescriptor` struct](#texturedescriptor-struct)
- [Removal of `TextureArray` and `SamplerArray` interfaces](#removal-of-texturearray-and-samplerarray-interfaces)
- [Array layers for cube textures](#array-layers-for-cube-textures)
- [Introduction of command encoding](#introduction-of-command-encoding)
- [Introduction of render passes](#introduction-of-render-passes)
- [Buffer updates](#buffer-updates)
- [Queries](#queries)
- [Independent blend states](#independent-blend-states)
- [`RenderingProfiler` interface](#renderingprofiler-interface)
- [Binding unordered access views (UAV)](#binding-unordered-access-views-uav)


## `Shader` interface

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


## `ShaderProgram` interface

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


## `LogicOp` usage

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


## Rasterization line width (OpenGL and Vulkan only)

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


## OpenGL profile selection

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


## Query texture descriptor

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


## Render target attachments

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


## Texture initialization with `ImageDescriptor` interface

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


## Dynamic state access for shader resources

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


## Clear specific attachments of active render target

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


## Viewport and scissor arrays

Functions to set viewport and scissor arrays have been renamed for a persistent nomenclature. A `Set...Array` function in the `CommandBuffer` interface is only used for `...Array` objects (such as `BufferArray`). For basic C/C++ arrays, functions like `SetViewports`, `SetScissors`, or `ClearAttachments` are used.

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


## Persistent states in `CommandBuffer` interface

Viewports and scissors are no longer guaranteed to be a persistent state. They must be updated after a new render target is set.


## Depth biasing in `RasterizerDescriptor` interface

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


## `ShaderUniform` interface

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


## Dependency removal of GaussianLib

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


## Renamed identifiers

Various structures, enumerations, and fields have been renamed to either fit LLGL's nomenclature or to simplify their identifiers when they are frequently used.

Before/After:
```cpp
ShaderStageFlags                               --> StageFlags
BlendTargetDescriptor::destColor               --> BlendTargetDescriptor::dstColor
BlendTargetDescriptor::destAlpha               --> BlendTargetDescriptor::dstAlpha
BlendOp::DestColor                             --> BlendOp::DstColor
BlendOp::InvDestColor                          --> BlendOp::InvDstColor
BlendOp::DestAlpha                             --> BlendOp::DstAlpha
BlendOp::InvDestAlpha                          --> BlendOp::InvDstAlpha
ImageDescriptor                                --> SrcImageDescriptor
BufferCPUAccess                                --> CPUAccess
TextureFilter                                  --> SamplerFilter
TextureWrap                                    --> SamplerAddressMode
Surface::Recreate                              --> Surface::ResetPixelFormat
SubTextureDescriptor                           --> TextureRegion
CompareOp::Never                               --> CompareOp::NeverPass
CompareOp::Ever                                --> CompareOp::AlwaysPass
RenderingLimits::maxNumTextureArrayLayers      --> RenderingLimits::maxTextureArrayLayers
RenderingLimits::maxNumRenderTargetAttachments --> RenderingLimits::maxColorAttachments
RenderingLimits::maxNumViewports               --> RenderingLimits::maxViewports
RenderingLimits::maxNumComputeShaderWorkGroups --> RenderingLimits::maxComputeShaderWorkGroups
```


## Graphics API dependent state

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


## `TextureFormat` and `VectorType` enumerations

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
myTextureDesc.format = LLGL::Format::RGBA8UNorm; // 8-bit normalized unsigned byte format

LLGL::VertexAttribute myVertexPositionAttrib { "myPosition", LLGL::Format::RGB32Float }; // 32-bit floating-point format
LLGL::VertexAttribute myVertexColorAttrib    { "myColor",    LLGL::Format::RGBA8UNorm }; // 8-bit normalized unsigned byte format
LLGL::VertexAttribute myVertexFlagsAttrib    { "myFlags",    LLGL::Format::R32UInt    }; // 32-bit unsigned integer format
```


## `TextureDescriptor` struct

The descriptor structure `TextureDescriptor` has been simplified and generalized. All nested structures have been removed.

Before:
```cpp
// Usage:
LLGL::TextureDescriptor my2DTexDesc;
my2DTexDesc.type             = LLGL::TextureType::Texture2DArray;
my2DTexDesc.format           = LLGL::TextureFormat::RGBA8;
my3DTexDesc.flags            = LLGL::TextureFlags::GenerateMips; // full MIP-map chain
my2DTexDesc.texture2D.width  = 512;
my2DTexDesc.texture2D.height = 512;
my2DTexDesc.texture2D.layers = 4;
LLGL::Texture* my2DTex = myRenderer->CreateTexture(my2DTexDesc);

LLGL::TextureDescriptor my3DTexDesc;
my3DTexDesc.type             = LLGL::TextureType::Texture3D;
my3DTexDesc.format           = LLGL::TextureFormat::R32Float;
my3DTexDesc.flags            = 0; // no MIP-mapping
my3DTexDesc.texture3D.width  = 16;
my3DTexDesc.texture3D.height = 16;
my3DTexDesc.texture3D.depth  = 16;
LLGL::Texture* my3DTex = myRenderer->CreateTexture(my3DTexDesc);
```

After:
```cpp
// Usage:
LLGL::TextureDescriptor my2DTexDesc;
my2DTexDesc.type          = LLGL::TextureType::Texture2DArray;
my2DTexDesc.format        = LLGL::Format::RGBA8UNorm;
my2DTexDesc.extent.width  = 512;
my2DTexDesc.extent.height = 512;
my2DTexDesc.arrayLayers   = 4;
my2DTexDesc.mipLevels     = 0; // full MIP-map chain
LLGL::Texture* my2DTex = myRenderer->CreateTexture(my2DTexDesc);

LLGL::TextureDescriptor my3DTexDesc;
my3DTexDesc.type          = LLGL::TextureType::Texture3D;
my3DTexDesc.format        = LLGL::Format::R32Float;
my3DTexDesc.extent.width  = 16;
my3DTexDesc.extent.height = 16;
my3DTexDesc.extent.depth  = 16;
my2DTexDesc.mipLevels     = 1; // no MIP-mapping
LLGL::Texture* my3DTex = myRenderer->CreateTexture(my3DTexDesc);
```


## Removal of `TextureArray` and `SamplerArray` interfaces

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


## Array layers for cube textures

The number of array layers for cube textures is no longer automatically multiplied by 6, but the client programmer must specifiy the correct number of array layers himself.

Before:
```cpp
// Usage:
LLGL::TextureDescriptor myTexDesc;
myTexDesc.type                = LLGL::TextureType::TextureCubeArray;
myTexDesc.textureCube.width   = 512;
myTexDesc.textureCube.height  = 512;
myTexDesc.textureCuber.layers = 2;
LLGL::Texture* myTex = myRenderer->CreateTexture(myTexDesc);
```

After:
```cpp
// Usage:
LLGL::TextureDescriptor myTexDesc;
myTexDesc.type        = LLGL::TextureType::TextureCubeArray;
myTexDesc.extent      = { 512, 512, 1 };
myTexDesc.arrayLayers = 2 * 6;
LLGL::Texture* myTex = myRenderer->CreateTexture(myTexDesc);
```


## Introduction of command encoding

From now on, the encoding of command buffers must be started and ended explicitly. Moreover, the encoded command buffer must be submitted to the command queue explicitly, too.

Before:
```cpp
// Render frame
/* ... */
myContext->Present();
```

After:
```cpp
// Render frame
myCmdBuffer->Begin();
/* ... */
myCmdBuffer->End();
myCmdQueue->Submit(*myCmdBuffer);
myContext->Present();
```


## Introduction of render passes

With the introduction of the `RenderPass` interface (which is optional to use), the `SetRenderTarget` functions have been replaced by `BeginRenderPass` with a subsequent call to `EndRenderPass`.

Before:
```cpp
// Render into texture
myCmdBuffer->SetRenderTarget(*myRenderTarget);
/* ... */

// Render onto screen
myCmdBuffer->SetRenderTarget(*myContext);
/* ... */

myContext->Present();
```

After:
```cpp
myCmdQueue->Begin(*myCmdBuffer);
{
    // Render into texture
    myCmdBuffer->BeginRenderPass(*myRenderTarget);
    /* ... */
    myCmdBuffer->EndRenderPass();

    // Render onto screen
    myCmdBuffer->BeginRenderPass(*myContext);
    /* ... */
    myCmdBuffer->EndRenderPass();
}
myCmdQueue->End(*myCmdBuffer);

myContext->Present();
```


## Buffer updates

Buffers can no longer be updated at an arbitrary time. The function `RenderSystem::WriteBuffer` has been refactored and can be used for large buffers outside of command encoding. The new function `CommandBuffer::UpdateBuffer` can be used during command encoding, but outside of a render pass, for small buffers (maximum of 2^16 = 65536 bytes).

Before:
```cpp
// Interface:
void RenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset);

// Usage:
for (auto myModel : myModelList) {
    myTransform.worldMatrix = myModel.worldMatrix;
    myRenderer->WriteBuffer(*myConstantBuffer, &myTransform, sizeof(myTransform), 0);
    myCmdBuffer->Draw(...);
}
```

After:
```cpp
// Interface:
void RenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize);
void CommandBuffer::UpdateBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint16_t dataSize);

// Usage:
for (auto myModel : myModelList) {
    myTransform.worldMatrix = myModel.worldMatrix;
    myCmdBuffer->UpdateBuffer(*myConstantBuffer, 0, &myTransform, sizeof(myTransform));
    myCmdBuffer->BeginRenderPass(...);
    myCmdBuffer->Draw(...);
    myCmdBuffer->EndRenderPass();
}
```


## Queries

Queries are now allocated as a heap object rather than single queries. The interface `Query` has been renamed to `QueryHeap` to apply to D3D12 and Vulkan. The result is now retrieved by the `CommandQueue` rather than the `CommandBuffer`.

Before:
```cpp
// Interface:
struct QueryDescriptor;
class Query;
Query* RenderSystem::CreateQuery(const QueryDescriptor& desc);
void CommandBuffer::BeginQuery(Query& query);
void CommandBuffer::EndQuery(Query& query);
void CommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode);
bool CommandBuffer::QueryResult(Query& query, std::uint64_t& result);
bool CommandBuffer::QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result);

// Usage:
LLGL::QueryDescriptor myQueryDesc;
myQueryDesc.type = LLGL::QueryType::SamplesPassed;
LLGL::Query* myQuery = myRenderer->CreateQuery(myQueryDesc);
/* ... */
myCmdBuffer->BeginQuery(*myQuery);
/* ... */
myCmdBuffer->EndQuery(*myQuery);
/* ... */
std::uint64_t result = 0;
if (myCmdBuffer->QueryResult(*myQuery, result)) {
    /* ... */
}
```

After:
```cpp
// Interface:
struct QueryHeapDescriptor;
class QueryHeap;
QueryHeap* RenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc);
void CommandBuffer::BeginQuery(Query& queryHeap, std::uint32_t query);
void CommandBuffer::EndQuery(Query& queryHeap, std::uint32_t query);
void CommandBuffer::BeginRenderCondition(QueryHeap& queryHeap, std::uint32_t query, const RenderConditionMode mode);
bool CommandQueue::QueryResult(QueryHeap& queryHeap, std::uint32_t firstQuery, std::uint32_t numQueries, void* data, std::size_t dataSize);

// Usage:
LLGL::QueryHeapDescriptor myQueryHeapDesc;
myQueryHeapDesc.type       = LLGL::QueryType::SamplesPassed;
myQueryHeapDesc.numQueries = 1;
LLGL::QueryHeap* myQueryHeap = myRenderer->CreateQueryHeap(myQueryHeapDesc);
/* ... */
myCmdBuffer->BeginQuery(*myQueryHeap, 0);
/* ... */
myCmdBuffer->EndQuery(*myQueryHeap, 0);
/* ... */
std::uint64_t result = 0;
if (myCmdQueue->QueryResult(*myQueryHeap, 0, 1, &result, sizeof(result))) {
    /* ... */
}
```


## Independent blend states

From now on, independent blend states must be explicitly enabled and the blend targets have been changed from an `std::vector` to a fixed size array of 8 elements (just like in D3D11 and D3D12).

Before:
```cpp
// Interface:
bool                               BlendDescriptor::blendEnabled;
std::vector<BlendTargetDescriptor> BlendDescriptor::targets;

// Usage:
LLGL::BlendTargetDescriptor myBlendTarget;
myBlendTarget.srcColor = LLGL::BlendOp::BlendFactor;

LLGL::GraphicsPipelineDescriptor myPipelineDesc;
myPipelineDesc.blend.blendEnabled   = true;
myPipelineDesc.blend.blendFactor    = { 1.0f, 0.5f, 0.0f, 1.0f };
myPipelineDesc.blend.targets.push_back(myBlendTarget);
```

After:
```cpp
// Interface:
bool                    BlendTargetDescriptor::blendEnabled;
BlendTargetDescriptor   BlendDescriptor::targets[8];

// Usage:
LLGL::GraphicsPipelineDescriptor myPipelineDesc;
myPipelineDesc.blend.blendFactor                = { 1.0f, 0.5f, 0.0f, 1.0f }; // <-- NOTE: will be moved to "CommandBuffer::SetBlendFactor"
myPipelineDesc.blend.targets[0].blendEnabled    = true;
myPipelineDesc.blend.targets[0].srcColor        = LLGL::BlendOp::BlendFactor;
```


## `RenderingProfiler` interface

The profiler has been refactored completely and the counter values are now declared in the `FrameProfile` structure.

Before:
```cpp
// Interface:
RenderingProfile::Counter RenderingProfile::setVertexBuffer;
RenderingProfile::Counter RenderingProfile::setIndexBuffer;
/* For more details, see documentation ... */

// Usage:
LLGL::RenderingProfiler myProfile;
auto myRenderer = LLGL::RenderSystem::Load("OpenGL", &myProfile);

while (/* ... */) {
    /* Render ... */
    
    /* Evaluate frame profile (e.g. 'myProfile.setVertexBuffer') ... */
    
    // Reset counters
    myProfile.ResetCounters();
}
```

After:
```cpp
// Interface:
FrameProfile    RenderingProfile::frameProfile;
std::uint32_t   FrameProfile::vertexBufferBindings;
std::uint32_t   FrameProfile::indexBufferBindings;
/* For more details, see documentation ... */

// Usage:
LLGL::RenderingProfiler myProfile;
auto myRenderer = LLGL::RenderSystem::Load("OpenGL", &myProfile);

while (/* ... */) {
    /* Render ... */
    
    // Get frame profile and reset counters
    LLGL::FrameProfile myFrameProfile;
    myProfile.NextProfile(&myFrameProfile);
    
    /* Evaluate frame profile (e.g. 'myFrameProfile.vertexBufferBindings') ... */
}
```


## Binding unordered access views (UAV)

Unordered access views (UAV) must be now explicitly bound with `StageFlags::StorageUsage`. Previously, the UAV was preferred over a shader resource view (SRV) and could be disabled when binding a resource using `ShaderStageFlags::ReadOnlyResource`.

Before:
```cpp
// Usage:
myCmdBufferExt->SetStorageBuffer(*myStorageBuffer1, 1, LLGL::ShaderStageFlags::ComputeStage | LLGL::ShaderStageFlags::ReadOnlyResource); // SRV
myCmdBufferExt->SetStorageBuffer(*myStorageBuffer2, 2, LLGL::ShaderStageFlags::ComputeStage);                                            // UAV
```

After:
```cpp
myCmdBufferExt->SetStorageBuffer(*myStorageBuffer1, 1, LLGL::StageFlags::ComputeStage);                                  // SRV
myCmdBufferExt->SetStorageBuffer(*myStorageBuffer2, 2, LLGL::StageFlags::ComputeStage | LLGL::StageFlags::StorageUsage); // UAV
```




