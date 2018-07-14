/*
 * Test3_Direct3D12.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"

//#define TEST_PRINT_SHADER_INFO

int main()
{
    try
    {
        // Setup profiler and debugger
        std::shared_ptr<LLGL::RenderingProfiler> profiler;
        std::shared_ptr<LLGL::RenderingDebugger> debugger;

        //profiler = std::make_shared<LLGL::RenderingProfiler>();
        //debugger = std::make_shared<TestDebugger>();

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Direct3D12", profiler.get(), debugger.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        //contextDesc.videoMode.fullscreen    = true;

        #if 0
        contextDesc.multiSampling.enabled   = true;
        contextDesc.multiSampling.samples   = 8;
        #endif

        contextDesc.vsync.enabled           = true;

        auto context = renderer->CreateRenderContext(contextDesc);

        auto window = static_cast<LLGL::Window*>(&(context->GetSurface()));

        auto title = "LLGL Test 3 ( " + renderer->GetName() + " )";
        window->SetTitle(std::wstring(title.begin(), title.end()));
        window->Show();

        auto renderCaps = renderer->GetRenderingCaps();

        // Create command buffer
        auto commands = renderer->CreateCommandBuffer();

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "POSITION", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "COLOR", LLGL::Format::RGB32Float });

        const float triSize = 0.5f;

        struct Vertex
        {
            Gs::Vector2f    position;
            LLGL::ColorRGBf color;
        }
        vertices[] =
        {
            { {        0,  triSize }, { 1, 0, 0 } },
            { {  triSize, -triSize }, { 0, 1, 0 } },
            { { -triSize, -triSize }, { 0, 0, 1 } },
        };

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.type                   = LLGL::BufferType::Vertex;
            vertexBufferDesc.size                   = sizeof(vertices);
            vertexBufferDesc.flags                  = LLGL::BufferFlags::DynamicUsage;
            vertexBufferDesc.vertexBuffer.format    = vertexFormat;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create constant buffer
        struct Matrices
        {
            Gs::Matrix4f projection;
        }
        matrices;

        float orthoSize = 0.0025f;
        matrices.projection = Gs::ProjectionMatrix4f::Orthogonal(800.0f * orthoSize, 600.0f * orthoSize, 0.1f, 100.0f).ToMatrix4();

        float rotation = 45.0f;
        Gs::RotateFree(matrices.projection, Gs::Vector3f(0, 0, 1), Gs::Deg2Rad(rotation));

        LLGL::BufferDescriptor constantBufferDesc;
        {
            constantBufferDesc.type     = LLGL::BufferType::Constant;
            constantBufferDesc.size     = sizeof(matrices);
            constantBufferDesc.flags    = LLGL::BufferFlags::DynamicUsage;
        }
        auto constantBuffer = renderer->CreateBuffer(constantBufferDesc, &matrices);

        // Load shader
        auto vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "TestShader.hlsl", "VS", "vs_5_0" });
        auto fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "TestShader.hlsl", "PS", "ps_5_0" });

        #ifdef TEST_PRINT_SHADER_INFO
        std::cout << "VERTEX OUTPUT:" << std::endl;
        #endif

        if (vertShader->HasErrors())
            std::cerr << vertShader->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << vertShader->Disassemble(LLGL::ShaderDisassembleFlags::InstructionOnly) << std::endl << std::endl;
        #endif

        #ifdef TEST_PRINT_SHADER_INFO
        std::cout << "PIXEL OUTPUT:" << std::endl;
        #endif

        if (fragShader->HasErrors())
            std::cerr << fragShader->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << fragShader->Disassemble(LLGL::ShaderDisassembleFlags::InstructionOnly) << std::endl << std::endl;
        #endif

        // Create shader program
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexFormats     = { vertexFormat };
            shaderProgramDesc.vertexShader      = vertShader;
            shaderProgramDesc.fragmentShader    = fragShader;
        }
        auto shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

        if (shaderProgram->HasErrors())
            std::cerr << shaderProgram->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << "Constant Buffers: " << shaderProgram->QueryConstantBuffers().size() << std::endl;
        #endif

        auto reflectionDesc = shaderProgram->QueryReflectionDesc();

        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;
        {
            layoutDesc.bindings =
            {
                LLGL::BindingDescriptor { LLGL::ResourceType::ConstantBuffer, LLGL::StageFlags::VertexStage, 0 }
            };
        }
        auto pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create resource heap
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews  = { constantBuffer };
        }
        auto resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.pipelineLayout             = pipelineLayout;

            #if 0
            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.depth.writeEnabled         = true;
            pipelineDesc.depth.compareOp            = LLGL::CompareOp::Less;
            #endif

            #if 0
            pipelineDesc.rasterizer.multiSampling    = contextDesc.multiSampling;
            #endif
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        commands->SetClearColor({ 0.1f, 0.1f, 0.4f });
        //context->SetClearColor({ 0, 0, 0 });

        #if 0

        // Create texture
        LLGL::ColorRGBAub imageData[4] =
        {
            { 255,   0,   0, 255 },
            {   0, 255,   0, 255 },
            {   0,   0, 255, 255 },
            { 255, 255,   0, 255 },
        };

        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type                = LLGL::TextureType::Texture2D;
            texDesc.format              = LLGL::Format::RGBA;
            texDesc.texture2D.width     = 2;
            texDesc.texture2D.height    = 2;
            texDesc.texture2D.layers    = 1;
        }
        LLGL::ImageDescriptor imageDesc;
        {
            imageDesc.buffer    = imageData;
            imageDesc.dataType  = LLGL::DataType::UInt8;
            imageDesc.format    = LLGL::ImageFormat::RGBA;
        }
        auto texture = renderer->CreateTexture(texDesc, &imageDesc);

        #endif

        // Main loop
        while (window->ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            commands->SetRenderTarget(*context);
            commands->SetViewport(LLGL::Viewport { { 0, 0 }, contextDesc.videoMode.resolution });

            commands->Clear(LLGL::ClearFlags::Color);

            commands->SetGraphicsPipeline(*pipeline);
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetGraphicsResourceHeap(*resourceHeap);

            commands->Draw(3, 0);

			context->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        system("pause");
    }

    return 0;
}
