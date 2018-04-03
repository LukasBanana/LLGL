/*
 * Test5_Vulkan.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"
#include <LLGL/Utility.h>
#include <chrono>


//#define TEST_QUERY


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
            
                void OnResize(LLGL::Window& sender, const LLGL::Size& clientAreaSize) override
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
        struct Vertex
        {
            Gs::Vector2f    coord;
            LLGL::ColorRGBf color;
        }
        vertices[] =
        {
            { {  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
            { { +0.5f, +0.5f }, { 0.0f, 1.0f, 0.0f } },
            { { -0.5f, +0.5f }, { 0.0f, 0.0f, 1.0f } },
        };

        // Create vertex format
        LLGL::VertexFormat vertexFormat;

        vertexFormat.AppendAttribute({ "coord", LLGL::VectorType::Float2 });
        vertexFormat.AppendAttribute({ "color", LLGL::VectorType::Float3 });

        shaderProgram->BuildInputLayout(1, &vertexFormat);

        // Create vertex buffer
        auto vertexBuffer = renderer->CreateBuffer(LLGL::VertexBufferDesc(sizeof(vertices), vertexFormat), vertices);

        // Create constant buffers
        struct Matrices
        {
            Gs::Matrix4f projection;
        }
        matrices;

        Gs::RotateFree(matrices.projection, Gs::Vector3f(0, 0, 1), Gs::pi * 0.5f);

        auto constBufferMatrices = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(matrices)), &matrices);

        struct Colors
        {
            LLGL::ColorRGBAf diffuse;
        }
        colors;

        colors.diffuse = { 0.0f, 1.0f, 0.0f };

        auto constBufferColors = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(colors)), &colors);

        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;

        LLGL::LayoutBinding layoutBinding;
        {
            layoutBinding.type          = LLGL::ResourceViewType::ConstantBuffer;
            layoutBinding.startSlot     = 0;
            layoutBinding.numSlots      = 1;
            layoutBinding.stageFlags    = LLGL::ShaderStageFlags::VertexStage;
        }
        layoutDesc.bindings.push_back(layoutBinding);

        #if 0
        {
            layoutBinding.startSlot     = 1;
            layoutBinding.stageFlags    = LLGL::ShaderStageFlags::FragmentStage;
        }
        layoutDesc.bindings.push_back(layoutBinding);
        #endif

        auto pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create resource view heap
        LLGL::ResourceViewHeapDescriptor rsvHeapDesc;
        {
            rsvHeapDesc.pipelineLayout  = pipelineLayout;
            rsvHeapDesc.resourceViews   = { LLGL::ResourceViewDesc(constBufferMatrices)/*, LLGL::ResourceViewDesc(constBufferColors)*/ };
        }
        auto resourceViewHeap = renderer->CreateResourceViewHeap(rsvHeapDesc);

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;

        const auto resolution = contextDesc.videoMode.resolution;
        const auto viewportSize = resolution.Cast<float>();

        pipelineDesc.shaderProgram = shaderProgram;
        pipelineDesc.pipelineLayout = pipelineLayout;
        pipelineDesc.viewports.push_back(LLGL::Viewport(0.0f, 0.0f, viewportSize.x, viewportSize.y));
        pipelineDesc.scissors.push_back(LLGL::Scissor(0, 0, resolution.x, resolution.y));
        pipelineDesc.blend.targets.push_back({});

        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

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

        auto fence = renderer->CreateFence();

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

            // Render scene
            commands->SetRenderTarget(*context);

            //commands->Clear(LLGL::ClearFlags::ColorDepth);

            commands->SetGraphicsPipeline(*pipeline);

            commands->SetVertexBuffer(*vertexBuffer);
            //commands->SetConstantBuffer(*constBufferMatrices, 0);
            commands->SetGraphicsResourceViewHeap(*resourceViewHeap, 0);

            //commands->UpdatePipelineLayout(*pipelineLayout);

            #ifdef TEST_QUERY
            commands->BeginQuery(*query);
            {
                commands->Draw(3, 0);
            }
            commands->EndQuery(*query);
            #else
            commands->Draw(3, 0);
            #endif

            context->Present();

            #if 1//TODO: currently required for "SetConstantBuffer" to work
            // Wait for command buffer to complete
            queue->Submit(*fence);
            queue->WaitForFence(*fence, ~0);
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
