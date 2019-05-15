/*
 * Example.cpp (Example_MultiRenderer)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

    void Render(const Gs::Matrix4f& wvpMatrix);

};

MyRenderer::MyRenderer(
    const char*             rendererModule,
    LLGL::Window&           mainWindow,
    const LLGL::Offset2D&   subWindowOffset,
    const LLGL::Viewport&   viewport)
:   viewport      { viewport },
    multiSampling { 8u       }
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
        windowDesc.size             = { static_cast<std::uint32_t>(viewport.width)/2, static_cast<std::uint32_t>(viewport.height) };
        windowDesc.borderless       = true;
        windowDesc.visible          = true;
        windowDesc.windowContext    = (&mainWindowContextHandle);
    }
    subWindow = LLGL::Window::Create(windowDesc);

    // Create render context with viewport size
    LLGL::RenderContextDescriptor contextDesc;
    {
        contextDesc.videoMode.resolution            = windowDesc.size;
        contextDesc.multiSampling                   = multiSampling;
        contextDesc.profileOpenGL.contextProfile    = LLGL::OpenGLContextProfile::CoreProfile;
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
        texturePath + (renderer->GetRendererID() == LLGL::RendererID::OpenGL ? "Logo_OpenGL.png" : "Logo_Direct3D.png")
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
        vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_4_0" });
        fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_4_0" });
    }
    else
    {
        vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "Example.vert" });
        fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "Example.frag" });
    }

    // Print info log (warnings and errors)
    for (auto shader : { vertShader, fragShader })
    {
        std::string log = shader->QueryInfoLog();
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
        throw std::runtime_error(shaderProgram->QueryInfoLog());

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
    renderer->WriteBuffer(*constantBuffer, 0, &wvpMatrix, sizeof(wvpMatrix));

    cmdBuffer->Begin();
    {
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
        const int halfWidth = static_cast<int>(resolution.width/2);

        MyRenderer myGLRenderer{ "OpenGL", *mainWindow, { 0, 0 }, resolution };
        MyRenderer myD3DRenderer{ "Direct3D11", *mainWindow, { halfWidth, 0 }, LLGL::Viewport{ { -halfWidth, 0 }, resolution } };

        mainWindow->Show();

        // Create resources
        auto cubeVertices = GenerateTexturedCubeVertices();
        auto cubeIndices = GenerateTexturedCubeTriangleIndices();

        myGLRenderer.CreateResources(cubeVertices, cubeIndices);
        myD3DRenderer.CreateResources(cubeVertices, cubeIndices);

        // Initialize matrices (OpenGL needs a unit-cube NDC-space)
        Gs::Matrix4f projMatrixGL, projMatrixD3D, viewMatrix, worldMatrix;

        const float aspectRatio = static_cast<float>(mainWindowDesc.size.width) / static_cast<float>(mainWindowDesc.size.height);
        const float nearPlane   = 0.1f;
        const float farPlane    = 100.0f;
        const float fieldOfView = 45.0f;

        projMatrixGL  = Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fieldOfView), Gs::ProjectionFlags::UnitCube).ToMatrix4();
        projMatrixD3D = Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fieldOfView)).ToMatrix4();

        Gs::Translate(viewMatrix, Gs::Vector3f(0, 0, 5));

        // Enter main loop
        while (mainWindow->ProcessEvents())
        {
            // Update scene transformation
            Gs::RotateFree(worldMatrix, Gs::Vector3f(0, 1, 0), Gs::Deg2Rad(0.005f));

            // Draw scene for OpenGL
            myGLRenderer.Render(projMatrixGL * viewMatrix * worldMatrix);

            // Draw scene for Direct3D
            myD3DRenderer.Render(projMatrixD3D * viewMatrix * worldMatrix);
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

