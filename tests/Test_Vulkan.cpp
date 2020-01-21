/*
 * Test_Vulkan.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utility.h>
#include <Gauss/Gauss.h>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>


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

        contextDesc.videoMode.resolution    = { 800, 600 };
        contextDesc.videoMode.swapChainSize = 2;
        //contextDesc.videoMode.fullscreen    = true;
        contextDesc.samples                 = 8;
        contextDesc.vsync.enabled           = true;

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

                explicit ResizeHandler(LLGL::RenderContext& context) :
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

        // Create vertex format
        LLGL::VertexFormat vertexFormat;

        vertexFormat.AppendAttribute({ "coord",    LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB32Float });

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

        // Create vertex buffer
        auto vertexBuffer = renderer->CreateBuffer(LLGL::VertexBufferDesc(sizeof(vertices), vertexFormat), vertices);

        // Create shader program
        auto vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Shaders/Triangle.vert.spv");
        auto fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Shaders/Triangle.frag.spv");

        vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexShader      = renderer->CreateShader(vertShaderDesc);
            shaderProgramDesc.fragmentShader    = renderer->CreateShader(fragShaderDesc);
        }
        auto shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

        if (shaderProgram->HasErrors())
            std::cerr << shaderProgram->GetReport() << std::endl;

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

        auto constBufferMatrices = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(matrices), LLGL::CPUAccessFlags::ReadWrite), &matrices);

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
        std::string texFilename = "../examples/Media/Textures/Logo_Vulkan.png";
        int texWidth = 0, texHeight = 0, texComponents = 0;

        auto imageBuffer = stbi_load(texFilename.c_str(), &texWidth, &texHeight, &texComponents, 4);
        if (!imageBuffer)
            throw std::runtime_error("failed to load texture from file: \"" + texFilename + "\"");

        LLGL::SrcImageDescriptor imageDesc;
        {
            imageDesc.data      = imageBuffer;
            imageDesc.dataSize  = texWidth*texHeight*4;
        };
        auto texture = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, texWidth, texHeight), &imageDesc);

        stbi_image_free(imageBuffer);

        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;

        layoutDesc.bindings =
        {
            LLGL::BindingDescriptor { LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage  , 2 },
            LLGL::BindingDescriptor { LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage, 5 },
            LLGL::BindingDescriptor { LLGL::ResourceType::Sampler, 0,                               LLGL::StageFlags::FragmentStage, 3 },
            LLGL::BindingDescriptor { LLGL::ResourceType::Texture, 0,                               LLGL::StageFlags::FragmentStage, 4 },
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
            pipelineDesc.renderPass         = context->GetRenderPass();
            pipelineDesc.pipelineLayout     = pipelineLayout;
            pipelineDesc.primitiveTopology  = LLGL::PrimitiveTopology::TriangleStrip;

            pipelineDesc.viewports.push_back(LLGL::Viewport{ 0.0f, 0.0f, viewportSize.x, viewportSize.y });

            pipelineDesc.blend.targets[0].blendEnabled = true;
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        // Create query
        #ifdef TEST_QUERY
        auto query = renderer->CreateQueryHeap(LLGL::QueryType::PipelineStatistics);
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
            // Update user input
            if (input->KeyDown(LLGL::Key::F1))
            {
                contextDesc.vsync.enabled = !contextDesc.vsync.enabled;
                context->SetVsync(contextDesc.vsync);
            }

            // Render scene
            commands->Begin();
            {
                commands->SetVertexBuffer(*vertexBuffer);
                commands->SetPipelineState(*pipeline);
                commands->SetResourceHeap(*resourceViewHeap);

                // Update constant buffer
                Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * 0.002f);
                commands->UpdateBuffer(*constBufferMatrices, 0, &matrices, sizeof(matrices));

                commands->BeginRenderPass(*context);
                {
                    commands->Clear(LLGL::ClearFlags::ColorDepth);

                    // Draw scene
                    #ifdef TEST_QUERY
                    commands->BeginQuery(*query);
                    commands->Draw(4, 0);
                    commands->EndQuery(*query);

                    queue->WaitIdle();
                    LLGL::QueryPipelineStatistics stats;
                    if (commands->QueryPipelineStatisticsResult(*query, stats))
                    {
                        __debugbreak();
                    }
                    #else
                    commands->Draw(4, 0);
                    #endif
                }
                commands->EndRenderPass();

                // Update constant buffer
                Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * 0.05f);
                commands->UpdateBuffer(*constBufferMatrices, 0, &matrices, sizeof(matrices));
                Gs::RotateFree(matrices.modelView, Gs::Vector3f(0, 0, 1), Gs::pi * -0.05f);

                commands->BeginRenderPass(*context);
                {
                    // Draw scene again
                    commands->Draw(4, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            queue->Submit(*commands);

            // Present result on screen
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
