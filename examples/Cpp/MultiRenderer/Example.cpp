/*
 * Example.cpp (Example_MultiRenderer)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifdef _WIN32

#include <ExampleBase.h>

#include <LLGL/Platform/NativeHandle.h>


struct Matrices
{
    Gs::Matrix4f wvpMatrix;
    Gs::Matrix4f wMatrix;
};

/*
 * MyRenderer class
 */

class MyRenderer
{

    LLGL::RenderSystemPtr               renderer;
    std::shared_ptr<LLGL::Window>       subWindow;

    LLGL::SwapChain*                    swapChain       = nullptr;
    LLGL::CommandQueue*                 cmdQueue        = nullptr;
    LLGL::CommandBuffer*                cmdBuffer       = nullptr;
    LLGL::Buffer*                       constantBuffer  = nullptr;
    LLGL::Buffer*                       vertexBuffer    = nullptr;
    LLGL::Buffer*                       indexBuffer     = nullptr;
    LLGL::Sampler*                      sampler         = nullptr;
    LLGL::Texture*                      texture         = nullptr;
    LLGL::ResourceHeap*                 resourceHeap    = nullptr;
    LLGL::Shader*                       vertShader      = nullptr;
    LLGL::Shader*                       fragShader      = nullptr;
    LLGL::PipelineLayout*               layout          = nullptr;
    LLGL::PipelineState*                pipeline        = nullptr;

    const std::uint32_t                 samples;
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
        const LLGL::ArrayView<TexturedVertex>&  vertices,
        const LLGL::ArrayView<std::uint32_t>&   indices
    );

    // Renders the scene from the specified view.
    void Render(const Gs::Matrix4f& vpMatrix, const Gs::Matrix4f& wMatrix);

    // Builds a perspective projection matrix for this renderer.
    Gs::Matrix4f BuildPerspectiveProjection(float aspectRatio, float nearPlane, float farPlane, float fieldOfView) const;

    // Returns the sub window of this renderer.
    LLGL::Window& GetSubWindow();

};

MyRenderer::MyRenderer(
    const char*             rendererModule,
    LLGL::Window&           mainWindow,
    const LLGL::Offset2D&   subWindowOffset,
    const LLGL::Viewport&   viewport)
:
    viewport       { viewport       },
    samples        { 8u             },
    rendererModule { rendererModule }
{
    // Load render system module
    renderer = LLGL::RenderSystem::Load(rendererModule);

    // Get native handle (HWND for Win32) from main window
    LLGL::NativeHandle mainWindowHandle;
    mainWindow.GetNativeHandle(&mainWindowHandle, sizeof(mainWindowHandle));

    // Create sub window for swap-chain
    LLGL::WindowDescriptor windowDesc;
    {
        windowDesc.position             = subWindowOffset;
        windowDesc.size                 = { static_cast<std::uint32_t>(viewport.width)/2, static_cast<std::uint32_t>(viewport.height)/2 };
        windowDesc.flags                = (LLGL::WindowFlags::Visible | LLGL::WindowFlags::Borderless);
        windowDesc.windowContext        = (&mainWindowHandle);
        windowDesc.windowContextSize    = sizeof(mainWindowHandle);
    }
    subWindow = LLGL::Window::Create(windowDesc);

    // Create swap-chain with viewport size
    LLGL::SwapChainDescriptor swapChainDesc;
    {
        swapChainDesc.resolution    = windowDesc.size;
        swapChainDesc.samples       = samples;
    }
    swapChain = renderer->CreateSwapChain(swapChainDesc, subWindow);

    // Enable V-sync
    swapChain->SetVsyncInterval(1);
}

void MyRenderer::CreateResources(const LLGL::ArrayView<TexturedVertex>& vertices, const LLGL::ArrayView<std::uint32_t>& indices)
{
    // Vertex format
    LLGL::VertexFormat vertexFormat;
    vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
    vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
    vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });

    // Create vertex buffer
    vertexBuffer = renderer->CreateBuffer(
        LLGL::VertexBufferDesc(sizeof(TexturedVertex) * vertices.size(), vertexFormat),
        vertices.data()
    );

    // Create index buffer
    indexBuffer = renderer->CreateBuffer(
        LLGL::IndexBufferDesc(sizeof(std::uint32_t) * indices.size(), LLGL::Format::R32UInt),
        indices.data()
    );

    // Create constant buffer
    constantBuffer = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(Matrices)));

    // Create textures
    texture = LoadTextureWithRenderer(*renderer, "Logo_" + rendererModule + ".png");

    // Create samplers
    LLGL::SamplerDescriptor samplerDesc;
    {
        samplerDesc.maxAnisotropy = 8;
    }
    sampler = renderer->CreateSampler(samplerDesc);

    // Create shaders
    const auto& languages = renderer->GetRenderingCaps().shadingLanguages;

    LLGL::ShaderDescriptor vertShaderDesc, fragShaderDesc;

    if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
    {
        vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_4_0");
        fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_4_0");
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
    {
        vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.vert");
        fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.frag");
    }
    else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
    {
        vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.450core.vert.spv");
        fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv");
    }
    else
        throw std::runtime_error("shaders not supported for active renderer");

    vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

    vertShader = renderer->CreateShader(vertShaderDesc);
    fragShader = renderer->CreateShader(fragShaderDesc);

    // Print info log (warnings and errors)
    for (auto shader : { vertShader, fragShader })
    {
        if (auto report = shader->GetReport())
        {
            if (*report->GetText() != '\0')
                std::cerr << report->GetText() << std::endl;
        }
    }

    // Create pipeline layout
    bool compiledSampler = (renderer->GetRendererID() == LLGL::RendererID::OpenGL);

    if (compiledSampler)
        layout = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(0):vert, texture(0):frag, sampler(0):frag}"));
    else
        layout = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(0):vert, texture(1):frag, sampler(2):frag}"));

    // Create resource heap
    resourceHeap = renderer->CreateResourceHeap(layout, { constantBuffer, texture, sampler });

    // Create graphics pipelines
    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    {
        pipelineDesc.vertexShader                   = vertShader;
        pipelineDesc.fragmentShader                 = fragShader;
        pipelineDesc.pipelineLayout                 = layout;
        pipelineDesc.depth.testEnabled              = true;
        pipelineDesc.depth.writeEnabled             = true;
        pipelineDesc.rasterizer.multiSampleEnabled  = (samples > 1);
    }
    pipeline = renderer->CreatePipelineState(pipelineDesc);

    if (auto report = pipeline->GetReport())
    {
        if (*report->GetText() != '\0')
            std::cerr << report->GetText() << std::endl;
    }

    // Get command queue
    cmdQueue = renderer->GetCommandQueue();

    // Create command buffers
    cmdBuffer = renderer->CreateCommandBuffer();
}

void MyRenderer::Render(const Gs::Matrix4f& vpMatrix, const Gs::Matrix4f& wMatrix)
{
    // Update constant buffer
    cmdBuffer->Begin();
    {
        Matrices matrices;
        {
            matrices.wvpMatrix  = vpMatrix * wMatrix;
            matrices.wMatrix    = wMatrix;
        }
        cmdBuffer->UpdateBuffer(*constantBuffer, 0, &matrices, sizeof(matrices));

        cmdBuffer->SetVertexBuffer(*vertexBuffer);
        cmdBuffer->SetIndexBuffer(*indexBuffer);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Clear color buffer
            cmdBuffer->Clear(LLGL::ClearFlags::ColorDepth, { 0.1f, 0.1f, 0.4f, 1.0f});

            // Set viewport
            cmdBuffer->SetViewport(viewport);

            // Set graphics pipeline and vertex buffer
            cmdBuffer->SetPipelineState(*pipeline);
            cmdBuffer->SetResourceHeap(*resourceHeap);

            // Draw triangulated cube
            cmdBuffer->DrawIndexed(36, 0);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();
    cmdQueue->Submit(*cmdBuffer);

    // Present the result on the screen
    swapChain->Present();
}

Gs::Matrix4f MyRenderer::BuildPerspectiveProjection(float aspectRatio, float nearPlane, float farPlane, float fieldOfView) const
{
    int flags = 0;

    if (renderer->GetRendererID() == LLGL::RendererID::OpenGL)
        flags |= Gs::ProjectionFlags::UnitCube;

    return Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fieldOfView), flags).ToMatrix4();
}

LLGL::Window& MyRenderer::GetSubWindow()
{
    return *subWindow;
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
            mainWindowDesc.title    = "LLGL Example: Multi Renderer ( OpenGL, Vulkan, Direct3D 11, Direct3D 12 )";
            mainWindowDesc.size     = resolution;
            mainWindowDesc.flags    = LLGL::WindowFlags::Centered;
        }
        std::unique_ptr<LLGL::Window> mainWindow = LLGL::Window::Create(mainWindowDesc);

        // Create renderers
        const int halfWidth     = static_cast<int>(resolution.width/2);
        const int halfHeight    = static_cast<int>(resolution.height/2);

        MyRenderer myRenderers[4] =
        {
            { "Vulkan",       *mainWindow, { 0,         0          }, LLGL::Viewport{ { 0,          0           }, resolution } },
            { "OpenGL",       *mainWindow, { halfWidth, 0          }, LLGL::Viewport{ { -halfWidth, 0           }, resolution } },
            { "Direct3D11",   *mainWindow, { 0,         halfHeight }, LLGL::Viewport{ { 0,          -halfHeight }, resolution } },
            { "Direct3D12",   *mainWindow, { halfWidth, halfHeight }, LLGL::Viewport{ { -halfWidth, -halfHeight }, resolution } },
        };

        mainWindow->Show();

        // Create resources
        auto cubeVertices = GenerateTexturedCubeVertices();
        auto cubeIndices = GenerateTexturedCubeTriangleIndices();

        for (auto& renderer : myRenderers)
            renderer.CreateResources(cubeVertices, cubeIndices);

        LLGL::Input input{ *mainWindow };

        // Initialize matrices (OpenGL needs a unit-cube NDC-space)
        const float aspectRatio = static_cast<float>(mainWindowDesc.size.width) / static_cast<float>(mainWindowDesc.size.height);
        const float nearPlane   = 0.1f;
        const float farPlane    = 100.0f;
        const float fieldOfView = 45.0f;

        Gs::Matrix4f projMatrices[4];
        for (int i = 0; i < 4; ++i)
        {
            projMatrices[i] = myRenderers[i].BuildPerspectiveProjection(aspectRatio, nearPlane, farPlane, fieldOfView);
            input.Listen(myRenderers[i].GetSubWindow());
        }

        Gs::Matrix4f viewMatrix, worldMatrix;
        Gs::Translate(viewMatrix, Gs::Vector3f(0, 0, 5));

        // Enter main loop
        while (LLGL::Surface::ProcessEvents() && !mainWindow->HasQuit() && !input.KeyDown(LLGL::Key::Escape))
        {
            // Update scene transformation
            if (input.KeyPressed(LLGL::Key::LButton))
            {
                const auto mouseMotion = Gs::Vector2f
                {
                    static_cast<float>(input.GetMouseMotion().x),
                    static_cast<float>(input.GetMouseMotion().y),
                } * 0.005f;

                // Rotate model around X and Y axes
                Gs::Matrix4f deltaRotation;
                Gs::RotateFree(deltaRotation, { 1, 0, 0 }, mouseMotion.y);
                Gs::RotateFree(deltaRotation, { 0, 1, 0 }, mouseMotion.x);
                worldMatrix = deltaRotation * worldMatrix;
            }

            // Draw scene for all renderers
            for (int i = 0; i < 4; ++i)
                myRenderers[i].Render(projMatrices[i] * viewMatrix, worldMatrix);

            input.Reset();
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

