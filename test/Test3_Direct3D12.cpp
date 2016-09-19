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

        //contextDesc.antiAliasing.enabled    = true;
        //contextDesc.antiAliasing.samples    = 8;

        contextDesc.vsync.enabled           = true;

        auto context = renderer->CreateRenderContext(contextDesc);
        
        auto window = &(context->GetWindow());

        auto title = "LLGL Test 3 ( " + renderer->GetName() + " )";
        window->SetTitle(std::wstring(title.begin(), title.end()));

        auto renderCaps = renderer->QueryRenderingCaps();
        auto shadingLang = renderer->QueryShadingLanguage();

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AddAttribute("POSITION", LLGL::DataType::Float32, 2);
        vertexFormat.AddAttribute("COLOR", LLGL::DataType::Float32, 3);

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

        auto vertexBuffer = renderer->CreateVertexBuffer();
        renderer->SetupVertexBuffer(*vertexBuffer, vertices, sizeof(vertices), LLGL::BufferUsage::Static, vertexFormat);

        // Create constant buffer
        struct Matrices
        {
            Gs::Matrix4f projection;
        }
        matrices;

        float orthoSize = 0.01f;
        matrices.projection = Gs::ProjectionMatrix4f::Orthogonal(800.0f * orthoSize, 600.0f * orthoSize, 0.1f, 100.0f).ToMatrix4();

        auto constantBuffer = renderer->CreateConstantBuffer();
        renderer->SetupConstantBuffer(*constantBuffer, &matrices, sizeof(matrices), LLGL::BufferUsage::Static);

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

        shaderProgram->BindVertexAttributes(vertexFormat.GetAttributes());

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
            //pipelineDesc.depth.testEnabled  = true;
            //pipelineDesc.depth.writeEnabled = true;
            //pipelineDesc.depth.compareOp    = LLGL::CompareOp::Less;

            pipelineDesc.rasterizer.multiSampleEnabled  = true;
            pipelineDesc.rasterizer.samples             = 8;

            pipelineDesc.shaderProgram      = shaderProgram;
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        context->SetClearColor({ 0.2f, 0.2f, 0.7f });
        //context->SetClearColor({ 0, 0, 0 });

        // Main loop
        while (window->ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            context->ClearBuffers(LLGL::ClearBuffersFlags::Color);

            context->SetViewports({ LLGL::Viewport(0, 0, 800, 600) });
            context->SetScissors({ LLGL::Scissor(0, 0, 800, 600) });

            context->SetGraphicsPipeline(*pipeline);
            context->SetVertexBuffer(*vertexBuffer);
            context->SetConstantBuffer(*constantBuffer, 0);

            context->Draw(3, 0);

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
