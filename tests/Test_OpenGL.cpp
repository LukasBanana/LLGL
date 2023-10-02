/*
 * Test_OpenGL.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/VertexFormat.h>
#include <Gauss/Gauss.h>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>


//#define TEST_RENDER_TARGET
//#define TEST_QUERY
//#define TEST_STORAGE_BUFFER


int main()
{
    try
    {
        // Setup profiler and debugger
        std::shared_ptr<LLGL::RenderingDebugger> debugger;

        debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        LLGL::RenderSystemDescriptor rendererDesc = "OpenGL";
        {
            rendererDesc.debugger = debugger.get();
        }
        auto renderer = LLGL::RenderSystem::Load(rendererDesc);

        // Create swap-chain
        LLGL::SwapChainDescriptor swapChainDesc;

        swapChainDesc.resolution    = { 800, 600 };
        swapChainDesc.samples       = 8;
        //swapChainDesc.fullscreen    = true;

        LLGL::RendererConfigurationOpenGL rendererConfig;
        rendererConfig.contextProfile   = LLGL::OpenGLContextProfile::CoreProfile;
        rendererConfig.majorVersion     = 3;
        rendererConfig.minorVersion     = 0;

        #ifdef __linux__

        auto swapChain = renderer->CreateSwapChain(swapChainDesc);

        auto window = static_cast<LLGL::Window*>(&(swapChain->GetSurface()));

        #else

        LLGL::WindowDescriptor windowDesc;
        {
            windowDesc.size     = swapChainDesc.resolution;
            windowDesc.flags    = LLGL::WindowFlags::Resizable | (swapChainDesc.fullscreen ? LLGL::WindowFlags::Borderless : LLGL::WindowFlags::Centered);
        }
        auto window = std::shared_ptr<LLGL::Window>(LLGL::Window::Create(windowDesc));

        auto swapChain = renderer->CreateSwapChain(swapChainDesc, window);

        #endif

        window->Show();

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        //const auto& renderCaps = renderer->GetRenderingCaps();

        // Setup window title
        window->SetTitle("LLGL Test 2 ( " + std::string(renderer->GetName()) + " )");

        // Setup input controller
        LLGL::Input input{ *window };

        class ResizeEventHandler : public LLGL::Window::EventListener
        {
        public:
            explicit ResizeEventHandler(LLGL::SwapChain* swapChain) :
                swapChain_ { swapChain  }
            {
            }
            void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override
            {
                swapChain_->ResizeBuffers(clientAreaSize);
            }
        private:
            LLGL::SwapChain* swapChain_;
        };

        auto resizeEventHandler = std::make_shared<ResizeEventHandler>(swapChain);
        window->AddEventListener(resizeEventHandler);

        // Create vertex buffer
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });

        const Gs::Vector2f vertices[] =
        {
            { 0, 0 }, { 110, 100 },
            { 0, 0 }, { 100, 200 },
            { 0, 0 }, { 200, 100 },
            { 0, 0 }, { 200, 200 },
        };

        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create vertex buffer array
        //auto vertexBufferArray = renderer->CreateBufferArray(1, &vertexBuffer);

        // Create vertex shader
        auto vertShaderSource =
        (
            #ifdef TEST_STORAGE_BUFFER
            "#version 430\n"
            #else
            "#version 130\n"
            #endif
            "uniform mat4 projection;\n"
            #ifdef TEST_STORAGE_BUFFER
            "layout(std430) buffer outputBuffer {\n"
            "    float v[4];\n"
            "} outputData;\n"
            #endif
            "in vec2 position;\n"
            "out vec2 vertexPos;\n"
            "void main() {\n"
            "    gl_Position = projection * vec4(position, 0.0, 1.0);\n"
            "    vertexPos = (position - vec2(125, 125))*vec2(0.02);\n"
            #ifdef TEST_STORAGE_BUFFER
            "    outputData.v[gl_VertexID] = vertexPos.x;\n"
            #endif
            "}\n"
        );

        LLGL::ShaderDescriptor vertShaderDesc;
        {
            vertShaderDesc.type                 = LLGL::ShaderType::Vertex;
            vertShaderDesc.source               = vertShaderSource;
            vertShaderDesc.sourceType           = LLGL::ShaderSourceType::CodeString;
            vertShaderDesc.vertex.inputAttribs  = vertexFormat.attributes;
        }
        auto vertShader = renderer->CreateShader(vertShaderDesc);

        if (auto report = vertShader->GetReport())
            std::cerr << report->GetText() << std::endl;

        // Create fragment shader
        auto fragShaderSource =
        (
            "#version 130\n"
            "out vec4 fragColor;\n"
            "uniform sampler2D tex;\n"
            "uniform vec4 color;\n"
            "in vec2 vertexPos;\n"
            "void main() {\n"
            "    fragColor = texture(tex, vertexPos) * color;\n"
            "}\n"
        );

        LLGL::ShaderDescriptor fragShaderDesc;
        {
            fragShaderDesc.type         = LLGL::ShaderType::Fragment;
            fragShaderDesc.source       = fragShaderSource;
            fragShaderDesc.sourceType   = LLGL::ShaderSourceType::CodeString;
        }
        auto fragShader = renderer->CreateShader(fragShaderDesc);

        #if 0//TODO
        // Reflect shader
        LLGL::ShaderReflection reflection;
        vertShader.Reflect(reflection);
        fragShader.Reflect(reflection);

        for (const auto& uniform : reflection.uniforms)
        {
            std::cout << "uniform: name = \"" << uniform.name << "\", location = " << uniform.location << ", size = " << uniform.size << std::endl;
        }
        #endif

        // Create texture
        LLGL::ColorRGBub image[4] =
        {
            { 255, 0, 0 },
            { 0, 255, 0 },
            { 0, 0, 255 },
            { 255, 0, 255 }
        };

        LLGL::ImageView imageView;
        {
            imageView.format    = LLGL::ImageFormat::RGB;
            imageView.dataType  = LLGL::DataType::UInt8;
            imageView.data      = image;
            imageView.dataSize  = 2*2*3;
        }
        LLGL::TextureDescriptor textureDesc;
        {
            textureDesc.type            = LLGL::TextureType::Texture2D;
            textureDesc.format          = LLGL::Format::RGBA8UNorm;
            textureDesc.extent.width    = 2;
            textureDesc.extent.height   = 2;
        }
        auto& texture = *renderer->CreateTexture(textureDesc, &imageView);

        LLGL::TextureRegion subTexDesc;
        {
            subTexDesc.offset.x                     = 0;
            subTexDesc.offset.y                     = 1;
            subTexDesc.extent.width                 = 2;
            subTexDesc.extent.height                = 1;
            subTexDesc.subresource.baseArrayLayer   = 0;
            subTexDesc.subresource.numArrayLayers   = 1;
            subTexDesc.subresource.baseMipLevel     = 0;
            subTexDesc.subresource.numMipLevels     = 1;
        }
        //renderer->WriteTexture(texture, subTexDesc, imageView); // update 2D texture

        //auto textureQueryDesc = texture.GetDesc();

        // Create render target
        LLGL::RenderTarget* renderTarget = nullptr;
        LLGL::Texture* renderTargetTex = nullptr;

        #ifdef TEST_RENDER_TARGET

        renderTarget = renderer->CreateRenderTarget(8);

        auto renderTargetSize = swapChainDesc.videoMode.resolution;

        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type                    = LLGL::TextureType::Texture2D;
            texDesc.format                  = LLGL::Format::RGBA8UNorm;
            texDesc.texture2DDesc.width     = renderTargetSize.x;
            texDesc.texture2DDesc.height    = renderTargetSize.y;
        }
        renderTargetTex = renderer->CreateTexture(texDesc);

        //auto numMips = LLGL::NumMipLevels({ renderTargetSize.x, renderTargetSize.y, 1 });

        //renderTarget->AttachDepthBuffer(renderTargetSize);
        renderTarget->AttachTexture2D(*renderTargetTex);

        #endif

        // Create pipeline layout
        auto pipelineLayout = renderer->CreatePipelineLayout(LLGL::Parse("texture(0):frag, sampler(0):frag"));

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.vertexShader                   = vertShader;
            pipelineDesc.fragmentShader                 = fragShader;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;

            pipelineDesc.rasterizer.multiSampleEnabled  = (swapChainDesc.samples > 1);

            pipelineDesc.blend.targets[0].dstColor      = LLGL::BlendOp::Zero;
        }
        auto& pipeline = *renderer->CreatePipelineState(pipelineDesc);

        if (auto report = pipeline.GetReport())
        {
            if (report->HasErrors())
                throw std::runtime_error(report->GetText());
        }

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.magFilter       = LLGL::SamplerFilter::Nearest;
            samplerDesc.minFilter       = LLGL::SamplerFilter::Linear;
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Border;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Border;
            #ifdef __linux__
            samplerDesc.mipMapEnabled   = false;
            #endif
            samplerDesc.borderColor[0]  = 0.0f;
            samplerDesc.borderColor[1]  = 0.7f;
            samplerDesc.borderColor[2]  = 0.5f;
            samplerDesc.borderColor[3]  = 1.0f;
        }
        auto& sampler = *renderer->CreateSampler(samplerDesc);

        #ifdef TEST_QUERY
        auto query = renderer->CreateQueryHeap(LLGL::QueryType::SamplesPassed);
        bool hasQueryResult = false;
        #endif

        #ifdef TEST_STORAGE_BUFFER

        LLGL::StorageBuffer* storage = nullptr;

        if (renderCaps.hasStorageBuffers)
        {
            storage = renderer->CreateStorageBuffer();
            renderer->SetupStorageBuffer(*storage, nullptr, sizeof(float)*4, LLGL::BufferUsage::Static);
            shaderProgram.BindStorageBuffer("outputBuffer", 0);
            commands->SetStorageBuffer(0, *storage);

            auto storeBufferDescs = shaderProgram.QueryStorageBuffers();
            for (const auto& desc : storeBufferDescs)
                std::cout << "storage buffer: name = \"" << desc.name << '\"' << std::endl;
        }

        #endif

        // Main loop
        while (LLGL::Surface::ProcessEvents() && !window->HasQuit() && !input.KeyDown(LLGL::Key::Escape))
        {
            debugger->FlushProfile();

            commands->Begin();
            {
                commands->SetViewport(swapChain->GetResolution());

                commands->BeginRenderPass(*swapChain);
                {
                    commands->Clear(LLGL::ClearFlags::Color, { 0.3f, 0.3f, 1.0f, 1.0f });

                    commands->SetPipelineState(pipeline);
                    commands->SetVertexBuffer(*vertexBuffer);

                    //#ifndef __linux__
                    commands->SetResource(1, sampler);
                    //#endif

                    #if 0//TODO
                    auto projection = Gs::ProjectionMatrix4f::Planar(
                        static_cast<Gs::Real>(swapChain->GetResolution().width),
                        static_cast<Gs::Real>(swapChain->GetResolution().height)
                    );
                    commands->SetUniforms(0, projection.Ptr(), sizeof(projection));

                    const LLGL::ColorRGBAf color{ 1.0f, 1.0f, 1.0f, 1.0f };
                    commands->SetUniforms(1, &color, sizeof(color));
                    #endif

                    if (renderTarget && renderTargetTex)
                    {
                        commands->EndRenderPass();
                        commands->BeginRenderPass(*renderTarget);
                        commands->Clear(LLGL::ClearFlags::Color, { 1, 1, 1, 1 });
                    }

                    #ifndef __linux__

                    // Switch fullscreen mode
                    static bool isFullscreen;
                    if (input.KeyDown(LLGL::Key::Return))
                    {
                        isFullscreen = !isFullscreen;
                        windowDesc.flags = LLGL::WindowFlags::Visible | LLGL::WindowFlags::Resizable | (isFullscreen ? LLGL::WindowFlags::Borderless : LLGL::WindowFlags::Centered);
                        windowDesc.position = { 0, 0 };
                        window->SetDesc(windowDesc);

                        swapChain->SwitchFullscreen(true);

                        commands->SetViewport(swapChainDesc.resolution);
                    }

                    #endif

                    #ifdef TEST_QUERY

                    if (!hasQueryResult)
                        commands->BeginQuery(*query);

                    #endif

                    commands->SetResource(1, texture);
                    commands->Draw(4, 0);

                    #ifdef TEST_STORAGE_BUFFER

                    if (renderCaps.hasStorageBuffers)
                    {
                        static bool outputShown;
                        if (!outputShown)
                        {
                            outputShown = true;
                            auto outputData = renderer->MapBuffer(*storage, LLGL::BufferCPUAccess::ReadOnly);
                            {
                                auto v = reinterpret_cast<Gs::Vector4f*>(outputData);
                                std::cout << "storage buffer output: " << *v << std::endl;
                            }
                            renderer->UnmapBuffer();
                        }
                    }

                    #endif

                    #ifdef TEST_QUERY

                    if (!hasQueryResult)
                    {
                        commands->EndQuery(*query);
                        hasQueryResult = true;
                    }

                    std::uint64_t result = 0;
                    if (commands->QueryResult(*query, result))
                    {
                        static std::uint64_t prevResult;
                        if (prevResult != result)
                        {
                            prevResult = result;
                            std::cout << "query result = " << result << std::endl;
                        }
                        hasQueryResult = false;
                    }

                    #endif

                    if (renderTarget && renderTargetTex)
                    {
                        commands->EndRenderPass();
                        commands->BeginRenderPass(*swapChain);
                        commands->SetResource(0, *renderTargetTex);
                        commands->Draw(4, 0);
                    }
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
