/*
 * Test3_Direct3D12.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        profiler = std::make_shared<LLGL::RenderingProfiler>();
        debugger = std::make_shared<TestDebugger>();

        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Direct3D12", profiler.get(), debugger.get());

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        //contextDesc.videoMode.fullscreen    = true;

        //contextDesc.sampling.enabled        = true;
        //contextDesc.sampling.samples        = 8;

        contextDesc.vsync.enabled           = true;

        auto context = renderer->CreateRenderContext(contextDesc);
        
        auto window = &(context->GetWindow());

        auto title = "LLGL Test 3 ( " + renderer->GetName() + " )";
        window->SetTitle(std::wstring(title.begin(), title.end()));

        auto renderCaps = renderer->GetRenderingCaps();

        // Create command buffer
        auto commands = renderer->CreateCommandBuffer();

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "POSITION", LLGL::VectorType::Float2 });
        vertexFormat.AppendAttribute({ "COLOR", LLGL::VectorType::Float3 });

        struct Vertex
        {
            Gs::Vector2f    position;
            LLGL::ColorRGBf color;
        }
        vertices[] =
        {
            { {  0,  1 }, { 1, 0, 0 } },
            { {  1, -1 }, { 0, 1, 0 } },
            { { -1, -1 }, { 0, 0, 1 } },
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

        float orthoSize = 0.01f;
        matrices.projection = Gs::ProjectionMatrix4f::Orthogonal(800.0f * orthoSize, 600.0f * orthoSize, 0.1f, 100.0f).ToMatrix4();

        LLGL::BufferDescriptor constantBufferDesc;
        {
            constantBufferDesc.type     = LLGL::BufferType::Constant;
            constantBufferDesc.size     = sizeof(matrices);
            constantBufferDesc.flags    = LLGL::BufferFlags::DynamicUsage;
        }
        auto constantBuffer = renderer->CreateBuffer(constantBufferDesc, &matrices);

        // Load shader
        auto shaderSource = ReadFileContent("TestShader.hlsl");

        auto vertShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
        auto fragShader = renderer->CreateShader(LLGL::ShaderType::Fragment);

        #ifdef TEST_PRINT_SHADER_INFO
        std::cout << "VERTEX OUTPUT:" << std::endl;
        #endif

        if (!vertShader->Compile({ shaderSource, "VS", "vs_5_0" }))
            std::cerr << vertShader->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << vertShader->Disassemble(LLGL::ShaderDisassembleFlags::InstructionOnly) << std::endl << std::endl;
        #endif

        #ifdef TEST_PRINT_SHADER_INFO
        std::cout << "PIXEL OUTPUT:" << std::endl;
        #endif

        if (!fragShader->Compile({ shaderSource, "PS", "ps_5_0" }))
            std::cerr << fragShader->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << fragShader->Disassemble(LLGL::ShaderDisassembleFlags::InstructionOnly) << std::endl << std::endl;
        #endif

        // Create shader program
        auto shaderProgram = renderer->CreateShaderProgram();

        shaderProgram->AttachShader(*vertShader);
        shaderProgram->AttachShader(*fragShader);

        shaderProgram->BuildInputLayout(vertexFormat);

        if (!shaderProgram->LinkShaders())
            std::cerr << shaderProgram->QueryInfoLog() << std::endl;
        #ifdef TEST_PRINT_SHADER_INFO
        else
            std::cout << "Constant Buffers: " << shaderProgram->QueryConstantBuffers().size() << std::endl;
        #endif

        auto constBufferDescs = shaderProgram->QueryConstantBuffers();
        auto storeBufferDescs = shaderProgram->QueryStorageBuffers();

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram          = shaderProgram;

            //pipelineDesc.depth.testEnabled      = true;
            //pipelineDesc.depth.writeEnabled     = true;
            //pipelineDesc.depth.compareOp        = LLGL::CompareOp::Less;

            pipelineDesc.rasterizer.sampling    = contextDesc.sampling;
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        commands->SetClearColor({ 0.2f, 0.2f, 0.7f });
        //context->SetClearColor({ 0, 0, 0 });

        // Main loop
        while (window->ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            commands->ClearBuffers(LLGL::ClearBuffersFlags::Color);

            commands->SetViewport(LLGL::Viewport(0, 0, 800, 600));
            commands->SetScissor(LLGL::Scissor(0, 0, 800, 600));

            commands->SetGraphicsPipeline(*pipeline);
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetConstantBuffer(*constantBuffer, 0);

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
