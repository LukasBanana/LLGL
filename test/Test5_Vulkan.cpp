/*
 * Test5_Vulkan.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <LLGL/Utility.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


//#define TEST_QUERY
#define TEST_RENDER_TARGET


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Vulkan");

        // Print renderer information
        const auto& info = renderer->GetRendererInfo();
        const auto& caps = renderer->GetRenderingCaps();

        std::cout << "Renderer:         " << info.rendererName << std::endl;
        std::cout << "Device:           " << info.deviceName << std::endl;
        std::cout << "Vendor:           " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution        = { 800, 600 };
        contextDesc.videoMode.swapChainSize     = 2;
        //contextDesc.videoMode.fullscreen        = true;

        contextDesc.multiSampling.enabled       = true;
        contextDesc.multiSampling.samples       = 8;

        contextDesc.vsync.enabled               = true;

        const auto resolution = contextDesc.videoMode.resolution;
        const Gs::Vector2f viewportSize { static_cast<float>(resolution.width), static_cast<float>(resolution.height) };

        LLGL::WindowDescriptor windowDesc;
        {
            windowDesc.size         = contextDesc.videoMode.resolution;
            windowDesc.resizable    = true;
            windowDesc.centered     = true;
            windowDesc.visible      = true;
        }
        auto window = std::shared_ptr<LLGL::Window>(std::move(LLGL::Window::Create(windowDesc)));

        window->SetTitle(L"LLGL Vulkan Test");

        auto context = renderer->CreateRenderContext(contextDesc, window);

        // Add resize event handler
        class ResizeHandler : public LLGL::Window::EventListener
        {

            public:

                ResizeHandler(LLGL::RenderContext& context) :
                    context_ { context }
                {
                }

                void OnResize(LLGL::Window& sender, const LLGL::Extent2D& clientAreaSize) override
                {
                    auto videoMode = context_.GetVideoMode();
                    videoMode.resolution = clientAreaSize;
                    context_.SetVideoMode(videoMode);
                }

            private:

                LLGL::RenderContext& context_;

        };

        window->AddEventListener(std::make_shared<ResizeHandler>(*context));

        // Get command queue
        auto queue = renderer->GetCommandQueue();

        // Create command buffer
        auto commands = renderer->CreateCommandBuffer();

        // Load shaders
        auto LoadSPIRVModule = [](const std::string& filename)
        {
            std::ifstream file(filename, std::ios::ate | std::ios::binary);

            if (!file.good())
                throw std::runtime_error("failed to read file: \"" + filename + "\"");

            auto fileSize = static_cast<size_t>(file.tellg());
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            return buffer;
        };

        auto shaderVert = renderer->CreateShader(LLGL::ShaderType::Vertex);
        auto shaderFrag = renderer->CreateShader(LLGL::ShaderType::Fragment);

        shaderVert->LoadBinary(LoadSPIRVModule("Triangle.vert.spv"));
        shaderFrag->LoadBinary(LoadSPIRVModule("Triangle.frag.spv"));

        // Create shader program
        auto shaderProgram = renderer->CreateShaderProgram();

        shaderProgram->AttachShader(*shaderVert);
        shaderProgram->AttachShader(*shaderFrag);

        shaderProgram->LinkShaders();

        // Create vertex data
        auto PointOnCircle = [](float angle, float radius)
        {
            return Gs::Vector2f { std::sin(angle) * radius, std::cos(angle) * radius };
        };

        const float uScale = 25.0f, vScale = 25.0f;

        struct Vertex
        {
            Gs::Vector2f    coord;
            Gs::Vector2f    texCoord;
            LLGL::ColorRGBf color;
        }
        vertices[] =
        {
            { { -1.0f,  1.0f }, { 0.0f  , vScale }, { 1.0f, 1.0f, 1.0f } },
            { { -1.0f, -1.0f }, { 0.0f  , 0.0f   }, { 1.0f, 1.0f, 1.0f } },
            { {  1.0f,  1.0f }, { uScale, vScale }, { 1.0f, 1.0f, 1.0f } },
            { {  1.0f, -1.0f }, { uScale, 0.0f   }, { 1.0f, 1.0f, 1.0f } },
        };

        // Create vertex format
        LLGL::VertexFormat vertexFormat;

        vertexFormat.AppendAttribute({ "coord",    LLGL::VectorType::Float2 });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::VectorType::Float2 });
        vertexFormat.AppendAttribute({ "color",    LLGL::VectorType::Float3 });

        shaderProgram->BuildInputLayout(1, &vertexFormat);

        // Create vertex buffer
        auto vertexBuffer = renderer->CreateBuffer(LLGL::VertexBufferDesc(sizeof(vertices), vertexFormat), vertices);

        // Create constant buffers
        struct Matrices
        {
            Gs::Matrix4f projection;
            Gs::Matrix4f modelView;
        }
        matrices;

        const float projectionScale = 0.005f;
        matrices.projection = Gs::ProjectionMatrix4f::Orthogonal(viewportSize.x * projectionScale, viewportSize.y * projectionScale, -100.0f, 100.0f, 0).ToMatrix4();
        //Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * 0.5f);

        auto constBufferMatrices = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(matrices), LLGL::BufferFlags::MapReadWriteAccess), &matrices);

        struct Colors
        {
            LLGL::ColorRGBAf diffuse;
        }
        colors;

        //colors.diffuse = { 1.0f, 2.0f, 5.0f };
        colors.diffuse = { 1.0f, 1.0f, 1.0f };

        auto constBufferColors = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(colors)), &colors);

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            //samplerDesc.mipMapping = false;
            #if 0
            samplerDesc.minFilter = LLGL::TextureFilter::Nearest;
            samplerDesc.magFilter = LLGL::TextureFilter::Nearest;
            #endif
        }
        auto sampler = renderer->CreateSampler(samplerDesc);

        // Create texture
        std::string texFilename = "../tutorial/Media/Textures/Logo_Vulkan.png";
        int texWidth = 0, texHeight = 0, texComponents = 0;

        auto imageBuffer = stbi_load(texFilename.c_str(), &texWidth, &texHeight, &texComponents, 4);
        if (!imageBuffer)
            throw std::runtime_error("failed to load texture from file: \"" + texFilename + "\"");

        LLGL::SrcImageDescriptor imageDesc;
        {
            imageDesc.data      = imageBuffer;
            imageDesc.dataSize  = texWidth*texHeight*4;
        };
        auto texture = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::TextureFormat::RGBA8UNorm, texWidth, texHeight), &imageDesc);

        renderer->GenerateMips(*texture);

        stbi_image_free(imageBuffer);

        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;

        layoutDesc.bindings =
        {
            LLGL::BindingDescriptor { LLGL::ResourceType::ConstantBuffer, LLGL::StageFlags::VertexStage  , 2 },
            LLGL::BindingDescriptor { LLGL::ResourceType::ConstantBuffer, LLGL::StageFlags::FragmentStage, 5 },
            LLGL::BindingDescriptor { LLGL::ResourceType::Sampler       , LLGL::StageFlags::FragmentStage, 3 },
            LLGL::BindingDescriptor { LLGL::ResourceType::Texture       , LLGL::StageFlags::FragmentStage, 4 },
        };

        auto pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create resource view heap
        LLGL::ResourceHeapDescriptor rsvHeapDesc;
        {
            rsvHeapDesc.pipelineLayout  = pipelineLayout;
            rsvHeapDesc.resourceViews   = { constBufferMatrices, constBufferColors, sampler, texture };
        }
        auto resourceViewHeap = renderer->CreateResourceHeap(rsvHeapDesc);

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram      = shaderProgram;
            pipelineDesc.pipelineLayout     = pipelineLayout;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;

            pipelineDesc.viewports.push_back(LLGL::Viewport{ 0.0f, 0.0f, viewportSize.x, viewportSize.y });

            pipelineDesc.blend.blendEnabled = true;
            pipelineDesc.blend.targets.push_back({});
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        #ifdef TEST_RENDER_TARGET

        // Create texture for render target attachment
        const std::uint32_t renderTargetSize = 512;
        auto renderTargetTex = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::TextureFormat::RGBA8UNorm, renderTargetSize, renderTargetSize));

        // Create render target
        LLGL::RenderTargetDescriptor rtDesc;
        {
            rtDesc.attachments =
            {
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Color, renderTargetTex }
            };
        }
        auto renderTarget = renderer->CreateRenderTarget(rtDesc);

        // Create render target graphics pipeline
        pipelineDesc.renderTarget = renderTarget;
        auto renderTargetPipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        #endif

        // Create query
        #ifdef TEST_QUERY
        auto query = renderer->CreateQuery(LLGL::QueryType::PipelineStatistics);
        #endif

        // Add input event listener
        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        auto frameTimer = LLGL::Timer::Create();
        auto printTime = std::chrono::system_clock::now();

        // Set clear color
        commands->SetClearColor({ 0.2f, 0.2f, 0.4f, 1.0f });

        //auto fence = renderer->CreateFence();

        // Main loop
        while (window->ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            #if 0
            // Show frame time
            frameTimer->MeasureTime();

            auto currentTime = std::chrono::system_clock::now();
            auto elapsedTime = (currentTime - printTime);
            auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

            if (elapsedMs > 250)
            {
                printf("Elapsed Time = %fms (FPS = %f)\n", static_cast<float>(frameTimer->GetDeltaTime()*1000.0f), static_cast<float>(1.0 / frameTimer->GetDeltaTime()));
                printTime = currentTime;
            }
            #endif

            // Update user input
            if (input->KeyDown(LLGL::Key::F1))
            {
                contextDesc.vsync.enabled = !contextDesc.vsync.enabled;
                context->SetVsync(contextDesc.vsync);
            }

            #if 1

            if (auto data = renderer->MapBuffer(*constBufferMatrices, LLGL::CPUAccess::ReadWrite))
            {
                auto ptr = reinterpret_cast<Matrices*>(data);
                Gs::RotateFree(ptr->modelView, Gs::Vector3f(0, 0, 1), Gs::pi * -0.002f);
                renderer->UnmapBuffer(*constBufferMatrices);
            }

            // Render scene
            commands->SetRenderTarget(*context);

            commands->Clear(LLGL::ClearFlags::ColorDepth);

            commands->SetGraphicsPipeline(*pipeline);

            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetGraphicsResourceHeap(*resourceViewHeap, 0);

            //commands->UpdatePipelineLayout(*pipelineLayout);

            #ifdef TEST_QUERY
            commands->BeginQuery(*query);
            {
                commands->Draw(4, 0);
            }
            commands->EndQuery(*query);
            #else
            commands->Draw(4, 0);
            #endif

            #ifdef TEST_RENDER_TARGET
            // Render scene into render target
            commands->SetRenderTarget(*renderTarget);
            commands->Clear(LLGL::ClearFlags::Color);
            commands->SetGraphicsPipeline(*renderTargetPipeline);
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetGraphicsResourceHeap(*resourceViewHeap, 0);
            commands->Draw(4, 0);
            #endif

            // Present result on screen
            context->Present();

            #if 0
            // Wait for command buffer to complete
            queue->WaitIdle();
            #endif

            // Evaluate query
            #ifdef TEST_QUERY
            LLGL::QueryPipelineStatistics statistics;
            while (!commands->QueryPipelineStatisticsResult(*query, statistics)) { /* wait */ }
            #ifdef _DEBUG
            __debugbreak();
            #endif
            #endif

            #else

            commands->BeginRenderPass(*contextRenderPass);
            {
                commands->Clear();

                commands->SetGraphicsPipeline(*pipeline);

                commands->SetVertexBuffer(*vertexBuffer);

                commands->Draw(3, 0);
            }
            commands->EndRenderPass(*contextRenderPass);

            context->Present();

            #endif
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
