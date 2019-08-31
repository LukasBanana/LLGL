/*
 * Example.cpp (Example_MultiRenderer)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef _WIN32

#include <ExampleBase.h>

#include <LLGL/Platform/NativeHandle.h>


/*
 * MyRenderer class
 */

class MyRenderer
{

    std::unique_ptr<LLGL::RenderSystem> renderer;
    std::shared_ptr<LLGL::Window>       subWindow;

    LLGL::RenderContext*                context         = nullptr;
    LLGL::CommandQueue*                 cmdQueue        = nullptr;
    LLGL::CommandBuffer*                cmdBuffer       = nullptr;
    LLGL::Buffer*                       constantBuffer  = nullptr;
    LLGL::Buffer*                       vertexBuffer    = nullptr;
    LLGL::Buffer*                       indexBuffer     = nullptr;
    LLGL::Sampler*                      sampler         = nullptr;
    LLGL::Texture*                      texture         = nullptr;
    LLGL::ResourceHeap*                 resourceHeap    = nullptr;
    LLGL::ShaderProgram*                shaderProgram   = nullptr;
    LLGL::PipelineLayout*               layout          = nullptr;
    LLGL::GraphicsPipeline*             pipeline        = nullptr;

    const LLGL::MultiSamplingDescriptor multiSampling;
    const LLGL::Viewport                viewport;
    const std::string                   rendererModule;

public:

    MyRenderer(
        const char*             rendererModule,
        LLGL::Window&           mainWindow,
        const LLGL::Offset2D&   subWindowOffset,
        const LLGL::Viewport&   viewport
    );

    void CreateResources(
        const std::vector<VertexPos3Tex2>&  vertices,
        const std::vector<std::uint32_t>&   indices
    );

    // Renders the scene from the specified view.
    void Render(const Gs::Matrix4f& wvpMatrix);

    // Builds a perspective projection matrix for this renderer.
    Gs::Matrix4f BuildPerspectiveProjection(float aspectRatio, float nearPlane, float farPlane, float fieldOfView) const;

};

MyRenderer::MyRenderer(
    const char*             rendererModule,
    LLGL::Window&           mainWindow,
    const LLGL::Offset2D&   subWindowOffset,
    const LLGL::Viewport&   viewport)
:   viewport       { viewport       },
    multiSampling  { 8u             },
    rendererModule { rendererModule }
{
    // Load render system module
    renderer = LLGL::RenderSystem::Load(rendererModule);

    // Get native handle (HWND for Win32) from main window
    LLGL::NativeHandle mainWindowHandle;
    mainWindow.GetNativeHandle(&mainWindowHandle);

    // Copy native handle from main window into context handle as parent window
    LLGL::NativeContextHandle mainWindowContextHandle;
    mainWindowContextHandle.parentWindow = mainWindowHandle.window;

    // Create sub window for render context
    LLGL::WindowDescriptor windowDesc;
    {
        windowDesc.position         = subWindowOffset;
        windowDesc.size             = { static_cast<std::uint32_t>(viewport.width)/2, static_cast<std::uint32_t>(viewport.height)/2 };
        windowDesc.borderless       = true;
        windowDesc.visible          = true;
        windowDesc.windowContext    = (&mainWindowContextHandle);
    }
    subWindow = LLGL::Window::Create(windowDesc);

    // Create render context with viewport size
    LLGL::RenderContextDescriptor contextDesc;
    {
        contextDesc.videoMode.resolution    = windowDesc.size;
        contextDesc.multiSampling           = multiSampling;
    }
    context = renderer->CreateRenderContext(contextDesc, subWindow);
}

void MyRenderer::CreateResources(const std::vector<VertexPos3Tex2>& vertices, const std::vector<std::uint32_t>& indices)
{
    // Vertex format
    LLGL::VertexFormat vertexFormat;
    vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
    vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });

    // Create vertex buffer
    vertexBuffer = renderer->CreateBuffer(
        LLGL::VertexBufferDesc(sizeof(VertexPos3Tex2) * vertices.size(), vertexFormat),
        vertices.data()
    );

    // Create index buffer
    indexBuffer = renderer->CreateBuffer(
        LLGL::IndexBufferDesc(sizeof(std::uint32_t) * indices.size(), LLGL::Format::R32UInt),
        indices.data()
    );

    // Create constant buffer
    constantBuffer = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(Gs::Matrix4f)));

    // Create textures
    const std::string texturePath = "../../Media/Textures/";
    texture = LoadTextureWithRenderer(
        *renderer,
        (texturePath + "Logo_" + rendererModule + ".png")
    );

    // Create samplers
    LLGL::SamplerDescriptor samplerDesc;
    {
        samplerDesc.maxAnisotropy = 8;
    }
    sampler = renderer->CreateSampler(samplerDesc);

    // Create shaders
    LLGL::Shader* vertShader = nullptr;
    LLGL::Shader* fragShader = nullptr;

    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;

    if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
    {
        vertShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_4_0"));
        fragShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_4_0"));
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
    {
        vertShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.vert"));
        fragShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.frag"));
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
    {
        vertShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.450core.vert.spv"));
        fragShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv"));
    }
    else
        throw std::runtime_error("shaders not supported for active renderer");

    // Print info log (warnings and errors)
    for (auto shader : { vertShader, fragShader })
    {
        std::string log = shader->GetReport();
        if (!log.empty())
            std::cerr << log << std::endl;
    }

    // Create shader program which is used as composite
    LLGL::ShaderProgramDescriptor shaderProgramDesc;
    {
        shaderProgramDesc.vertexFormats     = { vertexFormat };
        shaderProgramDesc.vertexShader      = vertShader;
        shaderProgramDesc.fragmentShader    = fragShader;
    }
    shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);
    if (shaderProgram->HasErrors())
        throw std::runtime_error(shaderProgram->GetReport());

    // Create pipeline layout
    bool compiledSampler = (renderer->GetRendererID() == LLGL::RendererID::OpenGL);

    if (compiledSampler)
        layout = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(0):vert, texture(0):frag, sampler(0):frag"));
    else
        layout = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(0):vert, texture(1):frag, sampler(2):frag"));

    // Create resource heap
    LLGL::ResourceHeapDescriptor resHeapDesc;
    {
        resHeapDesc.pipelineLayout  = layout;
        resHeapDesc.resourceViews   = { constantBuffer, texture, sampler };
    }
    resourceHeap = renderer->CreateResourceHeap(resHeapDesc);

    // Create graphics pipelines
    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    {
        pipelineDesc.shaderProgram              = shaderProgram;
        pipelineDesc.pipelineLayout             = layout;
        pipelineDesc.depth.testEnabled          = true;
        pipelineDesc.depth.writeEnabled         = true;
        pipelineDesc.rasterizer.multiSampling   = multiSampling;
    }
    pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

    // Get command queue
    cmdQueue = renderer->GetCommandQueue();

    // Create command buffers
    cmdBuffer = renderer->CreateCommandBuffer();
}

void MyRenderer::Render(const Gs::Matrix4f& wvpMatrix)
{
    // Update constant buffer
    cmdBuffer->Begin();
    {
        cmdBuffer->UpdateBuffer(*constantBuffer, 0, &wvpMatrix, sizeof(wvpMatrix));

        cmdBuffer->SetVertexBuffer(*vertexBuffer);
        cmdBuffer->SetIndexBuffer(*indexBuffer);

        cmdBuffer->BeginRenderPass(*context);
        {
            // Clear color buffer
            cmdBuffer->SetClearColor({ 0.1f, 0.1f, 0.4f });
            cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth);

            // Set viewport
            cmdBuffer->SetViewport(viewport);

            // Set graphics pipeline and vertex buffer
            cmdBuffer->SetGraphicsPipeline(*pipeline);
            cmdBuffer->SetGraphicsResourceHeap(*resourceHeap);

            // Draw triangulated cube
            cmdBuffer->DrawIndexed(36, 0);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();
    cmdQueue->Submit(*cmdBuffer);

    // Present the result on the screen
    context->Present();
}

Gs::Matrix4f MyRenderer::BuildPerspectiveProjection(float aspectRatio, float nearPlane, float farPlane, float fieldOfView) const
{
    int flags = 0;

    if (renderer->GetRendererID() == LLGL::RendererID::OpenGL)
        flags |= Gs::ProjectionFlags::UnitCube;

    return Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fieldOfView), flags).ToMatrix4();
}


/*
 * Main function
 */

int main(int argc, char* argv[])
{
    try
    {
        // Create main window
        const LLGL::Extent2D resolution{ 800, 600 };

        LLGL::WindowDescriptor mainWindowDesc;
        {
            mainWindowDesc.title    = L"LLGL Example: Multi Renderer ( OpenGL and Direct3D 11 )";
            mainWindowDesc.size     = resolution;
            mainWindowDesc.centered = true;
        }
        auto mainWindow = LLGL::Window::Create(mainWindowDesc);

        // Create renderers
        const int halfWidth     = static_cast<int>(resolution.width/2);
        const int halfHeight    = static_cast<int>(resolution.height/2);

        MyRenderer myRenderer0{ "Vulkan",       *mainWindow, { 0,         0          }, LLGL::Viewport{ { 0,          0           }, resolution } };
        MyRenderer myRenderer1{ "OpenGL",       *mainWindow, { halfWidth, 0          }, LLGL::Viewport{ { -halfWidth, 0           }, resolution } };
        MyRenderer myRenderer2{ "Direct3D11",   *mainWindow, { 0,         halfHeight }, LLGL::Viewport{ { 0,          -halfHeight }, resolution } };
        MyRenderer myRenderer3{ "Direct3D12",   *mainWindow, { halfWidth, halfHeight }, LLGL::Viewport{ { -halfWidth, -halfHeight }, resolution } };

        mainWindow->Show();

        // Create resources
        auto cubeVertices = GenerateTexturedCubeVertices();
        auto cubeIndices = GenerateTexturedCubeTriangleIndices();

        myRenderer0.CreateResources(cubeVertices, cubeIndices);
        myRenderer1.CreateResources(cubeVertices, cubeIndices);
        myRenderer2.CreateResources(cubeVertices, cubeIndices);
        myRenderer3.CreateResources(cubeVertices, cubeIndices);

        auto timer = LLGL::Timer::Create();

        // Initialize matrices (OpenGL needs a unit-cube NDC-space)
        const float aspectRatio = static_cast<float>(mainWindowDesc.size.width) / static_cast<float>(mainWindowDesc.size.height);
        const float nearPlane   = 0.1f;
        const float farPlane    = 100.0f;
        const float fieldOfView = 45.0f;

        auto projMatrix0 = myRenderer0.BuildPerspectiveProjection(aspectRatio, nearPlane, farPlane, fieldOfView);
        auto projMatrix1 = myRenderer1.BuildPerspectiveProjection(aspectRatio, nearPlane, farPlane, fieldOfView);
        auto projMatrix2 = myRenderer2.BuildPerspectiveProjection(aspectRatio, nearPlane, farPlane, fieldOfView);
        auto projMatrix3 = myRenderer3.BuildPerspectiveProjection(aspectRatio, nearPlane, farPlane, fieldOfView);

        Gs::Matrix4f viewMatrix, worldMatrix;
        Gs::Translate(viewMatrix, Gs::Vector3f(0, 0, 5));

        // Enter main loop
        while (mainWindow->ProcessEvents())
        {
            // Update scene transformation
            timer->MeasureTime();
            auto dt = static_cast<float>(timer->GetDeltaTime());
            Gs::RotateFree(worldMatrix, Gs::Vector3f(0, 1, 0), Gs::Deg2Rad(dt * 20.0f));

            // Draw scene for all renderers
            myRenderer0.Render(projMatrix0 * viewMatrix * worldMatrix);
            myRenderer1.Render(projMatrix1 * viewMatrix * worldMatrix);
            myRenderer2.Render(projMatrix2 * viewMatrix * worldMatrix);
            myRenderer3.Render(projMatrix3 * viewMatrix * worldMatrix);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        system("pause");
    }
    return 0;
}

#else

#include <iostream>

int main()
{
    std::cerr << "this tutorial is only available for the Win32 platform" << std::endl;
    return 0;
}

#endif

