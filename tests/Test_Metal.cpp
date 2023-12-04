/*
 * Test_Metal.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/Utility.h>
#include <LLGL/Utils/VertexFormat.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Metal");

        // Create swap chain
        LLGL::SwapChainDescriptor swapChainDesc;
        {
            swapChainDesc.resolution = { 800, 600 };
        }
        auto swapChain = renderer->CreateSwapChain(swapChainDesc);
        
        // Renderer info
        const auto& info = renderer->GetRendererInfo();
        
        std::cout << "Device: " << info.deviceName << std::endl;
        std::cout << "Renderer: " << info.rendererName << std::endl;
        std::cout << "Vendor: " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        auto& window = static_cast<LLGL::Window&>(swapChain->GetSurface());
        
        // Setup window title
        window.SetTitle("LLGL Test 9 ( " + std::string(renderer->GetName()) + " )");

        // Setup input controller
        LLGL::Input input{ window };

        // Create vertex buffer
        struct Vertex
        {
            float   position[2];
            float   texCoord[2];
            uint8_t color[4];
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

        LLGL::ImageView imageView;
        {
            imageView.format    = LLGL::ImageFormat::RGBA;
            imageView.dataType  = LLGL::DataType::UInt8;
            imageView.data      = imageBuffer;
            imageView.dataSize  = static_cast<std::size_t>(width*height*4);
        }

        // Create texture and upload image data onto hardware texture
        auto texture = renderer->CreateTexture(
            LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, width, height), &imageView
        );
        
        stbi_image_free(imageBuffer);

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Mirror;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Border;
            samplerDesc.mipMapEnabled   = true;
            samplerDesc.borderColor[0]  = 1;
            samplerDesc.borderColor[1]  = 1;
            samplerDesc.borderColor[2]  = 1;
            samplerDesc.borderColor[3]  = 1;
            #if 0
            samplerDesc.magFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.minFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.minLOD          = 2.0f;
            #endif
        }
        auto sampler = renderer->CreateSampler(samplerDesc);
        
        // Create shader program
        LLGL::ShaderDescriptor vsDesc{ LLGL::ShaderType::Vertex,   "TestShader.metal", "VMain", "1.1" };
        LLGL::ShaderDescriptor fsDesc{ LLGL::ShaderType::Fragment, "TestShader.metal", "FMain", "1.1" };

        vsDesc.vertex.inputAttribs = vertexFormat.attributes;

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.pipelineLayout     = renderer->CreatePipelineLayout(LLGL::Parse("texture(tex@0):frag,sampler(smpl@0):frag"));
            pipelineDesc.vertexShader       = renderer->CreateShader(vsDesc);
            pipelineDesc.fragmentShader     = renderer->CreateShader(fsDesc);
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        // Main loop
        while (window.ProcessEvents() && !input.KeyDown(LLGL::Key::Escape))
        {
            commands->Begin();
            {
                commands->BeginRenderPass(*swapChain);
                {
                    commands->SetViewport(swapChain->GetResolution());
                    
                    commands->Clear(LLGL::ClearFlags::Color, LLGL::ClearValue{ 0.3f, 0.3f, 1.0f, 1.0f });

                    commands->SetPipelineState(*pipeline);
                    commands->SetVertexBuffer(*vertexBuffer);
                    
                    commands->SetResource(0, *texture);
                    commands->SetResource(1, *sampler);

                    commands->Draw(4, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);
            
            swapChain->Present();
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
