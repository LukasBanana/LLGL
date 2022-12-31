/*
 * Example.cpp (Example_HelloTriangle)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <LLGL/Misc/TypeNames.h>
#include <chrono>


// Enable multi-sampling
#define ENABLE_MULTISAMPLING

// Enable timer to show render times every second
//#define ENABLE_TIMER

// Enable caching of pipeline state objects (PSO)
#define ENABLE_CACHED_PSO

int main(int argc, char* argv[])
{
    try
    {
        // Let the user choose an available renderer
        std::string rendererModule = GetSelectedRendererModule(argc, argv);

        // Load render system module
        std::unique_ptr<LLGL::RenderSystem> renderer = LLGL::RenderSystem::Load(rendererModule);

        // Create swap-chain
        LLGL::SwapChainDescriptor swapChainDesc;
        {
            swapChainDesc.resolution    = { 800, 600 };
            swapChainDesc.depthBits     = 0; // We don't need a depth buffer for this example
            swapChainDesc.stencilBits   = 0; // We don't need a stencil buffer for this example
            #ifdef ENABLE_MULTISAMPLING
            swapChainDesc.samples       = 8; // check if LLGL adapts sample count that is too high
            #endif
        }
        LLGL::SwapChain* swapChain = renderer->CreateSwapChain(swapChainDesc);

        // Print renderer information
        const auto& info = renderer->GetRendererInfo();

        std::cout << "Renderer:             " << info.rendererName << std::endl;
        std::cout << "Device:               " << info.deviceName << std::endl;
        std::cout << "Vendor:               " << info.vendorName << std::endl;
        std::cout << "Shading Language:     " << info.shadingLanguageName << std::endl;
        std::cout << "Swap Chain Format:    " << LLGL::ToString(swapChain->GetColorFormat()) << std::endl;
        std::cout << "Depth/Stencil Format: " << LLGL::ToString(swapChain->GetDepthStencilFormat()) << std::endl;

        // Enable V-sync
        swapChain->SetVsyncInterval(1);

        // Set window title and show window
        auto& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());

        window.SetTitle(L"LLGL Example: Hello Triangle");
        window.Show();

        // Vertex data structure
        struct Vertex
        {
            float   position[2];
            uint8_t color[4];
        };

        // Vertex data (3 vertices for our triangle)
        const float s = 0.5f;

        Vertex vertices[] =
        {
            { {  0,  s }, { 255, 0, 0, 255 } }, // 1st vertex: center-top, red
            { {  s, -s }, { 0, 255, 0, 255 } }, // 2nd vertex: right-bottom, green
            { { -s, -s }, { 0, 0, 255, 255 } }, // 3rd vertex: left-bottom, blue
        };

        // Vertex format
        LLGL::VertexFormat vertexFormat;

        // Append 2D float vector for position attribute
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });

        // Append 3D unsigned byte vector for color
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });

        // Update stride in case out vertex structure is not 4-byte aligned
        vertexFormat.SetStride(sizeof(Vertex));

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);                 // Size (in bytes) of the vertex buffer
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;    // Enables the buffer to be bound to a vertex buffer slot
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;          // Vertex format layout
        }
        LLGL::Buffer* vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create shaders
        LLGL::Shader* vertShader = nullptr;
        LLGL::Shader* fragShader = nullptr;

        const auto& languages = renderer->GetRenderingCaps().shadingLanguages;

        LLGL::ShaderDescriptor vertShaderDesc, fragShaderDesc;

        if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
        {
            if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL_140) != languages.end())
            {
                #ifdef __APPLE__
                vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.140core.vert" };
                fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.140core.frag" };
                #else
                vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.vert" };
                fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.frag" };
                #endif
            }
            else
            {
                vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.120.vert" };
                fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.120.frag" };
            }
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
        {
            vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.450core.vert.spv");
            fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv");
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
        {
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_4_0" };
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_4_0" };
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::Metal) != languages.end())
        {
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.metal", "VS", "1.1" };
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.metal", "PS", "1.1" };
        }

        // Specify vertex attributes for vertex shader
        vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        vertShader = renderer->CreateShader(vertShaderDesc);
        fragShader = renderer->CreateShader(fragShaderDesc);

        for (auto shader : { vertShader, fragShader })
        {
            if (auto report = shader->GetReport())
                std::cerr << report->GetText() << std::endl;
        }

        // Create graphics pipeline
        LLGL::PipelineState* pipeline = nullptr;
        std::unique_ptr<LLGL::Blob> pipelineCache;

        #ifdef ENABLE_CACHED_PSO
        // Try to read PSO cache from file
        const std::string cacheFilename = "GraphicsPSO." + rendererModule + ".cache";
        if ((pipelineCache = LLGL::Blob::CreateFromFile(cacheFilename)) != nullptr)
        {
            // Create graphics PSO from cache
            pipeline = renderer->CreatePipelineState(*pipelineCache);
            std::cout << "Pipeline cache restored: " << pipelineCache->GetSize() << " bytes" << std::endl;
        }
        else
        #endif
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.vertexShader                   = vertShader;
                pipelineDesc.fragmentShader                 = fragShader;
                pipelineDesc.renderPass                     = swapChain->GetRenderPass();
                #ifdef ENABLE_MULTISAMPLING
                pipelineDesc.rasterizer.multiSampleEnabled  = (swapChainDesc.samples > 1);
                #endif
            }

            #ifdef ENABLE_CACHED_PSO

            // Create and cache graphics PSO
            pipeline = renderer->CreatePipelineState(pipelineDesc, &pipelineCache);
            if (pipelineCache)
            {
                std::cout << "Pipeline cache created: " << pipelineCache->GetSize() << " bytes" << std::endl;

                // Store PSO cache to file
                std::ofstream file{ cacheFilename, std::ios::out | std::ios::binary };
                file.write(
                    reinterpret_cast<const char*>(pipelineCache->GetData()),
                    static_cast<std::streamsize>(pipelineCache->GetSize())
                );
            }

            #else

            // Create graphics PSO
            pipeline = renderer->CreatePipelineState(pipelineDesc);

            #endif

            // Link shader program and check for errors
            if (auto report = pipeline->GetReport())
            {
                if (report->HasErrors())
                    throw std::runtime_error(report->GetText());
            }
        }

        // Create command buffer to submit subsequent graphics commands to the GPU
        LLGL::CommandBuffer* commands = renderer->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);

        #ifdef ENABLE_TIMER
        Stopwatch timer;
        auto start = std::chrono::system_clock::now();
        #endif

        // Enter main loop
        while (window.ProcessEvents())
        {
            #ifdef ENABLE_TIMER
            timer.MeasureTime();
            auto end = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() > 0)
            {
                std::cout << "Rendertime: " << timer.GetDeltaTime() << ", FPS: " << 1.0 / timer.GetDeltaTime() << '\n';
                start = end;
            }
            #endif

            // Begin recording commands
            commands->Begin();
            {
                // Set viewport and scissor rectangle
                commands->SetViewport(swapChain->GetResolution());

                // Set graphics pipeline
                commands->SetPipelineState(*pipeline);

                // Set vertex buffer
                commands->SetVertexBuffer(*vertexBuffer);

                // Set the swap-chain as the initial render target
                commands->BeginRenderPass(*swapChain);
                {
                    // Clear color buffer
                    commands->Clear(LLGL::ClearFlags::Color);

                    // Draw triangle with 3 vertices
                    commands->Draw(3, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();

            // Present the result on the screen
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
