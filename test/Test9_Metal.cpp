/*
 * Test9_Metal.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <LLGL/Utility.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../tutorial/TutorialBase/stb/stb_image.h"


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
        auto commands = renderer->CreateCommandBufferExt();

        auto& window = static_cast<LLGL::Window&>(context->GetSurface());
        
        // Setup window title
        auto title = "LLGL Test 9 ( " + renderer->GetName() + " )";
        window.SetTitle(std::wstring(title.begin(), title.end()));

        // Setup input controller
        auto input = std::make_shared<LLGL::Input>();
        window.AddEventListener(input);

        // Create vertex buffer
        struct Vertex
        {
            Gs::Vector2f     position;
            Gs::Vector2f     texCoord;
            LLGL::ColorRGBub color;
        }
        vertices[] =
        {
            { { -0.5f, -0.5f }, { 0.0f, 1.0f }, { 255, 0, 0 } },
            { { -0.5f,  0.5f }, { 0.0f, 0.0f }, { 0, 255, 0 } },
            { {  0.5f, -0.5f }, { 1.0f, 1.0f }, { 0, 0, 255 } },
            { {  0.5f,  0.5f }, { 1.0f, 0.0f }, { 255, 0, 255 } },
        };

        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB8UNorm });
        vertexFormat.stride = sizeof(Vertex);

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.type                   = LLGL::BufferType::Vertex;
            vertexBufferDesc.size                   = sizeof(vertices);
            vertexBufferDesc.vertexBuffer.format    = vertexFormat;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);
        
        // Create texture
        const std::string textureFilename = "Media/Textures/Grid.png";
        int width = 0, height = 0, components = 0;
        auto imageBuffer = stbi_load(textureFilename.c_str(), &width, &height, &components, 4);
        if (!imageBuffer)
            throw std::runtime_error("failed to load texture from file: \"" + textureFilename + "\"");

        LLGL::SrcImageDescriptor imageDesc;
        {
            imageDesc.format    = LLGL::ImageFormat::RGBA;
            imageDesc.dataType  = LLGL::DataType::UInt8;
            imageDesc.data      = imageBuffer;
            imageDesc.dataSize  = static_cast<std::size_t>(width*height*4);
        }

        // Create texture and upload image data onto hardware texture
        auto texture = renderer->CreateTexture(
            LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, width, height), &imageDesc
        );
        
        stbi_image_free(imageBuffer);
        
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
        
        shaderProgram->BuildInputLayout(1, &(vertexBufferDesc.vertexBuffer.format));
        
        if (!shaderProgram->LinkShaders())
            throw std::runtime_error(shaderProgram->QueryInfoLog());
        
        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram      = shaderProgram;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Main loop
        commands->SetClearColor(LLGL::ColorRGBAf(0.3f, 0.3f, 1));
        
        while (window.ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            commands->SetRenderTarget(*context);
            commands->SetViewport(LLGL::Viewport{ { 0, 0 }, context->GetVideoMode().resolution });
            
            commands->Clear(LLGL::ClearFlags::Color);

            commands->SetGraphicsPipeline(*pipeline);
            commands->SetVertexBuffer(*vertexBuffer);
            
            commands->SetTexture(*texture, 0, LLGL::StageFlags::FragmentStage);

            commands->Draw(4, 0);

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
