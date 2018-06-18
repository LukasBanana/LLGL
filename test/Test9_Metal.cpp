/*
 * Test9_Metal.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <memory>
#include <iostream>
#include <string>
#include <sstream>


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Metal");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution    = { 800, 600 };
        contextDesc.vsync.enabled           = true;
        
        auto context = renderer->CreateRenderContext(contextDesc);
        
        // Renderer info
        const auto& info = renderer->GetRendererInfo();
        
        std::cout << "Device: " << info.deviceName << std::endl;
        std::cout << "Renderer: " << info.rendererName << std::endl;
        std::cout << "Vendor: " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        // Create command buffer
        auto commands = renderer->CreateCommandBuffer();

        auto& window = static_cast<LLGL::Window&>(context->GetSurface());
        
        // Setup window title
        auto title = "LLGL Test 9 ( " + renderer->GetName() + " )";
        window.SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window.AddEventListener(input);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::VectorType::Float2 });
        vertexFormat.AppendAttribute({ "color", LLGL::VectorType::Float3 });

        struct Vertex
        {
            Gs::Vector2f    position;
            LLGL::ColorRGBf color;
        }
        vertices[] =
        {
            { {  0.0f,  0.5f }, { 1.0f, 0.0f, 0.0f } },
            { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
            { { -0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f } },
        };

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.type                   = LLGL::BufferType::Vertex;
            vertexBufferDesc.size                   = sizeof(vertices);
            vertexBufferDesc.vertexBuffer.format    = vertexFormat;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);
        
        // Create shaders
        auto vertShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
        auto fragShader = renderer->CreateShader(LLGL::ShaderType::Fragment);
        
        auto shaderSource = ReadFileContent("TestShader.metal");
        
        vertShader->Compile(shaderSource, { "VMain", "1.1" });
        fragShader->Compile(shaderSource, { "FMain", "1.1" });

        // Create shader program
        auto shaderProgram = renderer->CreateShaderProgram();
        
        shaderProgram->AttachShader(*vertShader);
        shaderProgram->AttachShader(*fragShader);
        
        if (!shaderProgram->LinkShaders())
            throw std::runtime_error(shaderProgram->QueryInfoLog());
        
        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram = shaderProgram;
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Main loop
        commands->SetClearColor(LLGL::ColorRGBAf(0.3f, 0.3f, 1));
        
        while (window.ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            commands->SetRenderTarget(*context);
            commands->Clear(LLGL::ClearFlags::Color);

            commands->SetGraphicsPipeline(*pipeline);
            commands->SetVertexBuffer(*vertexBuffer);

            commands->Draw(3, 0);

            context->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        #ifdef _WIN32
        system("pause");
        #endif
    }

    return 0;
}
