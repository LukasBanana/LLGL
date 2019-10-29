/*
 * Test9_Metal.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <LLGL/Utility.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "../examples/Cpp/ExampleBase/stb/stb_image.h"


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
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

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
        vertexFormat.SetStride(sizeof(Vertex));

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
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

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Mirror;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Border;
            samplerDesc.mipMapping      = true;
            samplerDesc.borderColor     = { 1, 1, 1, 1 };
            #if 0
            samplerDesc.magFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.minFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.minLOD          = 2.0f;
            #endif
        }
        auto sampler = renderer->CreateSampler(samplerDesc);
        
        // Create shader program
        LLGL::ShaderDescriptor vsDesc { LLGL::ShaderType::Vertex,   "TestShader.metal", "VMain", "1.1" };
        LLGL::ShaderDescriptor fsDesc { LLGL::ShaderType::Fragment, "TestShader.metal", "FMain", "1.1" };

        vsDesc.vertex.inputAttribs = vertexFormat.attributes;

        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexShader      = renderer->CreateShader(vsDesc);
            shaderProgramDesc.fragmentShader    = renderer->CreateShader(fsDesc);
        }
        auto shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);
        
        if (shaderProgram->HasErrors())
            throw std::runtime_error(shaderProgram->GetReport());
        
        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram      = shaderProgram;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        // Main loop
        commands->SetClearColor(LLGL::ColorRGBAf(0.3f, 0.3f, 1));
        
        while (window.ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            commands->Begin();
            {
                commands->BeginRenderPass(*context);
                {
                    commands->SetViewport(LLGL::Viewport{ { 0, 0 }, context->GetVideoMode().resolution });
                    
                    commands->Clear(LLGL::ClearFlags::Color);

                    commands->SetPipelineState(*pipeline);
                    commands->SetVertexBuffer(*vertexBuffer);
                    
                    commands->SetResource(*texture, LLGL::BindFlags::Sampled, 0, LLGL::StageFlags::FragmentStage);
                    commands->SetResource(*sampler, 0, 0, LLGL::StageFlags::FragmentStage);

                    commands->Draw(4, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);
            
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
