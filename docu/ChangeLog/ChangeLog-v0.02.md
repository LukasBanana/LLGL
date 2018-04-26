ChangeLog v0.02
===============

`ShaderProgram` interface:
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


Rasterization line width:
-------------------------

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
bool ProfileOpenGLDescriptor::extProfile;
bool ProfileOpenGLDescriptor::coreProfile;
bool ProfileOpenGLDescriptor::debugDump;
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
int ProfileOpenGLDescriptor::majorVersion;
int ProfileOpenGLDescriptor::minorVersion;

// Usage:
LLGL::RenderContextDescriptor myContextDesc;
/* ... */
myContextDesc.profileOpenGL.contextProflie = LLGL::OpenGLContextProfile::CoreProfile;
myContextDesc.profileOpenGL.majorVersion   = 4;
myContextDesc.profileOpenGL.minorVersion   = 6;
myRenderer->CreateRenderContext(myContextDesc);
```







