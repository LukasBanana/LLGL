/*
 * Example.cpp (Example_HelloTriangle)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Utils/TypeNames.h>
#include <chrono>


// Enable multi-sampling
#define ENABLE_MULTISAMPLING    1

// Enable caching of pipeline state objects (PSO)
#define ENABLE_CACHED_PSO       0

#ifdef LLGL_OS_ANDROID
#define EXIT(VAL) return
void android_main(android_app* androidApp)
#else
#define EXIT(VAL) return (VAL)
int main(int argc, char* argv[])
#endif
{
    try
    {
        LLGL::Log::RegisterCallbackStd();

        // Let the user choose an available renderer
        LLGL::RenderSystemDescriptor rendererDesc;

        #ifdef LLGL_OS_ANDROID
        rendererDesc.moduleName = "OpenGLES3";
        rendererDesc.androidApp = androidApp;
        #else
        const std::string rendererModule = GetSelectedRendererModule(argc, argv);
        rendererDesc.moduleName = rendererModule;
        #endif

        //rendererDesc.flags = LLGL::RenderSystemFlags::DebugDevice;
        LLGL::RenderingDebugger debugger;
        rendererDesc.debugger = &debugger;

        // Load render system module
        LLGL::Report report;
        LLGL::RenderSystemPtr renderer = LLGL::RenderSystem::Load(rendererDesc, &report);
        if (!renderer)
        {
            LLGL::Log::Errorf("%s", report.GetText());
            EXIT(1);
        }

        // Create swap-chain
        const LLGL::Display* display = LLGL::Display::GetPrimary();
        const std::uint32_t resScale = (display != nullptr ? static_cast<std::uint32_t>(display->GetScale()) : 1u);

        LLGL::SwapChainDescriptor swapChainDesc;
        {
            swapChainDesc.resolution    = { 800 * resScale, 600 * resScale };
            swapChainDesc.depthBits     = 0; // We don't need a depth buffer for this example
            swapChainDesc.stencilBits   = 0; // We don't need a stencil buffer for this example
            #if ENABLE_MULTISAMPLING
            swapChainDesc.samples       = 8; // check if LLGL adapts sample count that is too high
            #endif
        }
        LLGL::SwapChain* swapChain = renderer->CreateSwapChain(swapChainDesc);

        // Print renderer information
        const auto& info = renderer->GetRendererInfo();

        LLGL::Log::Printf(
            "Renderer:             %s\n"
            "Device:               %s\n"
            "Vendor:               %s\n"
            "Shading Language:     %s\n"
            "Swap Chain Format:    %s\n"
            "Depth/Stencil Format: %s\n"
            "Resolution:           %u x %u\n"
            "Samples:              %u\n",
            info.rendererName.c_str(),
            info.deviceName.c_str(),
            info.vendorName.c_str(),
            info.shadingLanguageName.c_str(),
            LLGL::ToString(swapChain->GetColorFormat()),
            LLGL::ToString(swapChain->GetDepthStencilFormat()),
            swapChain->GetResolution().width,
            swapChain->GetResolution().height,
            swapChain->GetSamples()
        );

        // Enable V-sync
        swapChain->SetVsyncInterval(1);

        // Set window title and show window
        LLGL::Window* window = nullptr;

        if (LLGL::IsInstanceOf<LLGL::Window>(swapChain->GetSurface()))
        {
            window = LLGL::CastTo<LLGL::Window>(&swapChain->GetSurface());
            window->SetTitle(L"LLGL Example: Hello Triangle");
            window->Show();
        }

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
            vertShaderDesc.flags |= LLGL::ShaderCompileFlags::DefaultLibrary;
            fragShaderDesc.flags |= LLGL::ShaderCompileFlags::DefaultLibrary;
        }

        // Specify vertex attributes for vertex shader
        vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;

        vertShader = renderer->CreateShader(vertShaderDesc);
        fragShader = renderer->CreateShader(fragShaderDesc);

        for (LLGL::Shader* shader : { vertShader, fragShader })
        {
            if (const LLGL::Report* report = shader->GetReport())
                LLGL::Log::Errorf("%s", report->GetText());
        }

        // Create graphics pipeline
        LLGL::PipelineState* pipeline = nullptr;
        LLGL::PipelineCache* pipelineCache = nullptr;

        #if ENABLE_CACHED_PSO

        // Try to read PSO cache from file
        const std::string cacheFilename = "GraphicsPSO." + rendererModule + ".cache";
        bool hasInitialCache = false;

        LLGL::Blob pipelineCacheBlob = LLGL::Blob::CreateFromFile(cacheFilename);
        if (pipelineCacheBlob)
        {
            LLGL::Log::Printf("Pipeline cache restored: %zu bytes\n", pipelineCacheBlob.GetSize());
            hasInitialCache = true;
        }

        pipelineCache = renderer->CreatePipelineCache(pipelineCacheBlob);

        #endif

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = vertShader;
            pipelineDesc.fragmentShader                 = fragShader;
            pipelineDesc.renderPass                     = swapChain->GetRenderPass();
            #if ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampleEnabled  = (swapChainDesc.samples > 1);
            #endif
        }

        // Create and cache graphics PSO
        std::uint64_t psoStartTime = LLGL::Timer::Tick();
        pipeline = renderer->CreatePipelineState(pipelineDesc, pipelineCache);
        std::uint64_t psoEndTime = LLGL::Timer::Tick();

        #if ENABLE_CACHED_PSO

        const double psoTime = static_cast<double>(psoEndTime - psoStartTime) / static_cast<double>(LLGL::Timer::Frequency()) * 1000.0;
        LLGL::Log::Printf("PSO creation time: %f ms\n", psoTime);

        if (!hasInitialCache)
        {
            if (LLGL::Blob psoCache = pipelineCache->GetBlob())
            {
                LLGL::Log::Printf("Pipeline cache created: %zu bytes", psoCache.GetSize());

                // Store PSO cache to file
                std::ofstream file{ cacheFilename, std::ios::out | std::ios::binary };
                file.write(
                    reinterpret_cast<const char*>(psoCache.GetData()),
                    static_cast<std::streamsize>(psoCache.GetSize())
                );
            }
        }

        #endif

        // Link shader program and check for errors
        if (const LLGL::Report* report = pipeline->GetReport())
        {
            if (report->HasErrors())
            {
                LLGL::Log::Errorf("%s\n", report->GetText());
                EXIT(1);
            }
        }

        // Create command buffer to submit subsequent graphics commands to the GPU
        LLGL::CommandBuffer* commands = renderer->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);

        // Enter main loop
        const float bgColor[4] = { 0.1f, 0.1f, 0.2f, 1.0f };

        while (LLGL::Surface::ProcessEvents() && (window == nullptr || !window->HasQuit()))
        {
            // Begin recording commands
            commands->Begin();
            {
                // Set viewport and scissor rectangle
                commands->SetViewport(swapChain->GetResolution());

                // Set vertex buffer
                commands->SetVertexBuffer(*vertexBuffer);

                // Set the swap-chain as the initial render target
                commands->BeginRenderPass(*swapChain);
                {
                    // Clear color buffer
                    commands->Clear(LLGL::ClearFlags::Color, bgColor);

                    // Set graphics pipeline
                    commands->SetPipelineState(*pipeline);

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
        LLGL::Log::Errorf("%s\n", e.what());
        #ifdef _WIN32
        system("pause");
        #endif
    }
    EXIT(0);
}
