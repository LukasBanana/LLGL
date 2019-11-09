# Hello Triangle

<p align="center"><img src="Example.png"/></p>

## Initialization

The first thing we need to use LLGL is an instance of the `RenderSystem` interface. This instannce will take care of all objects used for rendering:
```cpp
std::unique_ptr<LLGL::RenderSystem> myRenderer = LLGL::RenderSystem::Load("Vulkan");
```
This is one of the few functions that takes a string rather than an enumeration to select something. This is because LLGL loads a render system dynamically at runtime from a module (i.e. a shared library, **.dll** on Windows, **.so** on GNU/Linux, and **.dylib** on macOS). On the one hand, we only need to link our project against **LLGL.lib**, and on the other hand we can catch an exception if our desired Vulkan renderer is not supported on the target platform. In this case we can load another renderer (e.g. "OpenGL") rather than disturbing the user with an error message such as "vulkan-1.dll could not be loaded".

The exception handling to find a suitable render system can look like this:
```cpp
std::unique_ptr<LLGL::RenderSystem> myRenderer;
for (auto module : { "Direct3D12", "Direct3D11", "Metal", "Vulkan", "OpenGL" }) {
    try {
        myRenderer = LLGL::RenderSystem::Load(module);
        break;
    } catch (const std::exception& e) {
        /* Log exception or ignore ... */
    }
}
if (myRenderer == nullptr) {
    /* Error: no suitable renderer found ... */
}
```
After we have a valid render system, we can continue with a render context to draw something into. We can connect a render context to a custom window (if we were using GLFW, wxWidgets, or Qt for instance), but if we omit the second parameter of the following call, LLGL will create a default surface. `Surface` is the base for the `Window` interface on desktop platforms and `Canvas` interface on mobile platforms. For this example we only use the default surface:
```cpp
LLGL::RenderContextDescriptor myContextDesc;
myContextDesc.videoMode.resolution = { 800, 600 };
myContextDesc.videoMode.fullscreen = false;
myContextDesc.vsync.enabled        = true;
myContextDesc.samples              = 8;
LLGL::RenderContext* myContext = myRenderer->CreateRenderContext(myContextDesc);
```
Most objects in LLGL are created with descriptors (similar to Direct3D and Vulkan). This one describes that we want a render context with a resolution of 800 x 600 pixels, windowed-mode (no fullscreen), V-sync enabled, and 8 samples for anti-aliasing.


## Vertex Buffer

Next we create our vertex data for our triangle we want to render and then declare the vertex format which will be passed to the vertex buffer and shader program:
```cpp
struct MyVertex {
    float   position[2]; // 2D vector for X and Y coordinates
    uint8_t color[4];    // 4D vector for red, green, blue, alpha color components
};
```
For this tutorial we only want to render a single triangle so we define our 3 vertices:
```cpp
MyVertex myVertices[3] = {
    MyVertex{ {  0.0f,  0.5f }, { 255,   0,   0, 255 } },
    MyVertex{ {  0.5f, -0.5f }, {   0, 255,   0, 255 } },
    MyVertex{ { -0.5f, -0.5f }, {   0,   0, 255, 255 } },
};
```
The `VertexFormat` structure has a couple of member functions to simplify the description of a vertex format, but we could also use the member variables directly. The `AppendAttribute` function determines the data offset for each vertex attribute automatically:
```cpp
LLGL::VertexFormat myVertexFormat;
myVertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float  });
myVertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });
```
The strings "position" and "color" must be equal to the identifiers used in the shader, not the one we used in our `MyVertex` structure! We use an RGBA format for the color components even though the alpha channel is not used, because RGB formats are only supported by OpenGL and Vulkan. The identifier `UNorm` denotes a 'normalized unsigned integral' format, i.e. the unsigned byte values in the range [0, 255] will be normalized to the range [0, 1].

Now we can create the GPU vertex buffer:
```cpp
LLGL::BufferDescriptor myVBufferDesc;
myVBufferDesc.size          = sizeof(myVertices);            // Size (in bytes) of the buffer
myVBufferDesc.bindFlags     = LLGL::BindFlags::VertexBuffer; // Use for vertex buffer binding
myVBufferDesc.vertexAttribs = myVertexFormat.attributes;     // Vertex buffer attributes
LLGL::Buffer* myVertexBuffer = myRenderer->CreateBuffer(myVBufferDesc, myVertices);
```
The parameter `bindFlags` takes a bitwise OR combination of the enumeration entries of `LLG::BindFlags`, to tell the render system for which purposes the buffer will be used. In this case, we will only bind it as a vertex buffer.


## Shaders

In LLGL, shaders are organized with an instance of the `ShaderProgram` interface to which we can attach one or more instances of the `Shader` interface, but only at creation time. In this tutorial we only need a vertex and fragment shader. We can either store them (of type `LLGL::Shader*`) or we just pass them to the `ShaderProgramDescriptor`. To determine which shading language is supported by the current renderer, we can use the `shadingLanguages` container as shown here:
```cpp
bool IsSupported(LLGL::ShadingLanguage lang) {
    const auto& supportedLangs = myRenderer->GetRenderingCaps().shadingLanguages;
    return (std::find(supportedLangs.begin(), supportedLangs.end(), lang) != supportedLangs.end());
}
```
Here is an example of loading a shader for either HLSL or GLSL source files:
```cpp
LLGL::ShaderDescriptor myVSDesc, myFSDesc;
if (IsSupported(LLGL::ShadingLanguage::HLSL)) {
    myVSDesc = { LLGL::ShaderType::Vertex,   "MyShader.hlsl", "VMain", "vs_4_0" };
    myFSDesc = { LLGL::ShaderType::Fragment, "MyShader.hlsl", "PMain", "ps_4_0" };
} else if (IsSupported(LLGL::ShadingLanguage::GLSL)) {
    myVSDesc = { LLGL::ShaderType::Vertex,   "MyShader.vert" };
    myFSDesc = { LLGL::ShaderType::Fragment, "MyShader.frag" };
} else {
    /* Error: shading language not supported ... */
}
myVSDesc.vertex.inputAttribs = myVertexFormat.attributes;
```
The initializer list within the `CreateShader` function call constructs the descriptor `ShaderDescriptor`. The descriptor provides members to specifiy the shader code as content or as filename which is then loaded by the framework. In our case, we just load the shaders from file (e.g. `MyShader.hlsl`). The descriptor also provides a member to specify whether our shader is in binary or high-level text form. However, not all render systems provide both. For example, Vulkan can only load binary SPIR-V modules. With Direct3D 11 and Direct3D 12, we can load either our HLSL shader directly from a text file, or load a pre-compiled DXBC binary file. Here is an example how to load a shader as content instead of loading it from file:
```cpp
LLGL::ShaderDescriptor myShaderDesc;
myShaderDesc.type                = LLGL::ShaderType::Vertex;
myShaderDesc.source              = "#version 330\n"
                                   "void main() {\n"
                                   "    gl_Position = vec4(0, 0, 0, 1);\n"
                                   "}\n";
myShaderDesc.sourceSize          = 0;                                    // Ignored for null-terminated sources
myShaderDesc.sourceType          = LLGL::ShaderSourceType::CodeString;   // Source is null-terminated string and in high-level format
myShaderDesc.entryPoint          = "";                                   // Only required for HLSL (can be null)
myShaderDesc.profile             = "";                                   // Only required for HLSL (can be null)
myShaderDesc.vertex.inputAttribs = myVertexFormat.attributes;            // Vertex input attribnutes (only for vertex shaders)
LLGL::Shader* myExampleShader = myRenderer->CreateShader(myShaderDesc);
```
The members `entryPoint` and `profile` are only required for HLSL (e.g. `entryPoint="main"` and `profile="vs_4_0"`). The member `source` is of type `const char*` and is either a null-terminated string or a raw byte buffer (for shader byte code). If the source type is either `CodeFile` or `BinaryFile`, the source code is read from file using the C++ standard file streams (i.e. `std::ifstream`).

After we created the shaders we also need to specify the input layout by passing the vertex format to the shader program descriptor:
```cpp
LLGL::ShaderProgramDescritpor myProgramDesc;
myProgramDesc.vertexShader   = myRenderer->CreateShader(myVSDesc);
myProgramDesc.fragmentShader = myRenderer->CreateShader(myFSDesc);
```
However, this is not necessary for a compute shader.

Even if a shader compiles successfully, we can query the information log if the shader compiler reports some warnings:
```cpp
for (auto shader : { myProgramDesc.vertexShader, myProgramDesc.fragmentShader }) {
    std::string log = shader->GetReport();
    if (!log.empty()) {
        std::cerr << log << std::endl;
    }
}
```
Now we can finally create the shader program and check for linking errors:
```cpp
LLGL::ShaderProgram* myShaderProgram = myRenderer->CreateShaderProgram(myProgramDesc);
if (myShaderProgram->HasErrors()) {
    throw std::runtime_error(myShaderProgram->GetReport());
}
```


## Graphics Pipeline & Command Buffer

Before we enter our render loop we need a pipeline state object and a command buffer to submit draw commands to the GPU. For this tutorial we can use almost all default values in the graphics pipeline state descriptor, but we always need to set the shader program:
```cpp
LLGL::GraphicsPipelineDescriptor myPipelineDesc;
myPipelineDesc.shaderProgram                 = myShaderProgram;
myPipelineDesc.rasterizer.multiSampleEnabled = (myContextDesc.samples > 1);
LLGL::PipelineState* myPipeline = myRenderer->CreatePipelineState(myPipelineDesc);
```
The members `depth`, `stencil`, `rasterizer`, and `blend` from the  `GraphicsPipelineDescriptor` structure can be used to specify a lot more configurations for a graphics pipeline. But for now, we leave them as is.

The command buffer is used to submit draw and compute commands to the command queue. In LLGL, there is only a single command queue instance:
```cpp
LLGL::CommandQueue* myCmdQueue = myRenderer->GetCommandQueue();
LLGL::CommandBuffer* myCmdBuffer = myRenderer->CreateCommandBuffer();
```


## Render Loop

Our render loop can be implemented with a simple `while`-statement in which is update the window events. For this we need the instance of the `Window` interface, which was created when we called `CreateRenderContext`:
```cpp
// Cast "Window" from base class "Surface" (only on desktop platforms such as Windows, GNU/Linux, and macOS)
LLGL::Window& myWindow = LLGL::CastTo<LLGL::Window>(myContext->GetSurface());

// Process window events (such as user input)
while (myWindow.ProcessEvents()) {
    /* Render code goes here ... */
}
```
The `ProcessEvents` function will return false when the user clicks on the window close button.

The render code inside the loop statement comes next. We first start encoding graphics commands for our command buffer:
```cpp
myCmdBuffer->Begin();
```
Now we can bind resources that are independent of a render pass, such as the vertex buffer:
```cpp
myCmdBuffer->SetVertexBuffer(*myVertexBuffer);
```
Before we can start encoding drawing commands and other operations that are dependent on render passes, we need to start such a render pass:
```cpp
myCmdBuffer->BeginRenderPass(*myContext);
```
We currently only use the default render pass that is created by the render context automatically. More about render passes in another tutorial. The `BeginRenderPass` function not only starts a render pass, but also specifies which render target is to be used. In this case `myContext` to render directly into the window content.

After binding the render target, we specify into which area the scene is rendered. For this simple example, we render into the entire render context:
```cpp
myCmdBuffer->SetViewport(myContext->GetResolution());
```
Contrary to Direct3D 12, LLGL manages the scissor rectangle automatically if the scissor test is disabled in the graphics pipeline (which is the default). Hence, we only need to set the viewport, but not the scissor rectangle.

Next we clear the previous content of the color buffer:
```cpp
myCmdBuffer->Clear(LLGL::ClearFlags::Color);
```
To clear multiple attachments of the active render target, the `ClearAttachments` function can be used instead. The last state we set before rendering is the pipeline state we created earlier:
```cpp
myCmdBuffer->SetPipelineState(*myPipeline);
```
Now we can finally draw our first triangle:
```cpp
myCmdBuffer->Draw(3, 0);
```
This call generates three vertices and starts with the vertex ID zero. This is analogous to the drawing commands of all modern rendering APIs (i.e. Direct3D 12, Vulkan, Metal) as well as the legacy rendering APIs (i.e. Direct3D 11, OpenGL). The same holds true for the other `DrawInstanced`, `DrawIndexed`, and `DrawIndexedInstanced` functions. The nomenclature for these functions is derived from Direct3D.

Before we can present the result, we need to end the render pass as well as encoding the command buffer:
```cpp
myCmdBuffer->EndRenderPass();
myCmdBuffer->End();
```
The last thing we have to do is to submit the encoded command buffer to the command queue and present the result on the screen:
```cpp
myCmdQueue->Submit(*myCmdBuffer);
myContext->Present();
```


That's all folks :-)


