/*
 * Example.cpp (Example_MultiContext)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>


int main(int argc, char* argv[])
{
    try
    {
        // Set report callback to standard output
        LLGL::Log::RegisterCallbackStd();

        // Load render system module
        LLGL::RenderingDebugger debugger;
        auto renderer = LLGL::RenderSystem::Load(GetSelectedRendererModule(argc, argv));//, nullptr, &debugger);

        LLGL::Log::Printf("LLGL Renderer: %s\n", renderer->GetName());

        // Create two swap-chains
        std::uint32_t resolutionScale = 1;
        if (const LLGL::Display* display = LLGL::Display::GetPrimary())
            resolutionScale = static_cast<int>(display->GetScale());

        const LLGL::Extent2D swapChainResolution{ 640 * resolutionScale, 480 * resolutionScale };

        LLGL::SwapChainDescriptor swapChainDesc[2];
        {
            swapChainDesc[0].resolution     = swapChainResolution;
            swapChainDesc[0].samples        = 8;
            swapChainDesc[0].depthBits      = 0;
            swapChainDesc[0].stencilBits    = 0;
        }
        auto swapChain1 = renderer->CreateSwapChain(swapChainDesc[0]);
        {
            swapChainDesc[1].resolution     = swapChainResolution;
            swapChainDesc[1].samples        = 8;
            swapChainDesc[1].depthBits      = 0;
            swapChainDesc[1].stencilBits    = 0;
        }
        auto swapChain2 = renderer->CreateSwapChain(swapChainDesc[1]);

        // Enable V-sync
        swapChain1->SetVsyncInterval(1);
        swapChain2->SetVsyncInterval(1);

        // Get command queue and create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Create input handler
        auto& window1 = static_cast<LLGL::Window&>(swapChain1->GetSurface());
        auto& window2 = static_cast<LLGL::Window&>(swapChain2->GetSurface());

        LLGL::Input inputs[2];
        inputs[0].Listen(window1);
        inputs[1].Listen(window2);

        // Set window titles
        window1.SetTitle(L"LLGL Example: Multi Context (1)");
        window2.SetTitle(L"LLGL Example: Multi Context (2)");

        // Set window positions
        if (const LLGL::Display* display = LLGL::Display::GetPrimary())
        {
            const LLGL::Extent2D desktopResolution = display->GetDisplayMode().resolution;
            const int scale = static_cast<int>(display->GetScale());
            const LLGL::Offset2D desktopCenter
            {
                static_cast<int>(desktopResolution.width)/scale/2,
                static_cast<int>(desktopResolution.height)/scale/2
            };

            window1.SetPosition({ desktopCenter.x - 700, desktopCenter.y - 480/2 });
            window2.SetPosition({ desktopCenter.x + 700 - 640, desktopCenter.y - 480/2 });
        }

        // Show windows
        window1.Show();
        window2.Show();

        // Vertex data structure
        struct Vertex
        {
            float position[2];
            float color[3];
        };

        // Vertex data
        float objSize = 0.5f;
        Vertex vertices[] =
        {
            // Triangle
            { {        0,  objSize }, { 1, 0, 0 } },
            { {  objSize, -objSize }, { 0, 1, 0 } },
            { { -objSize, -objSize }, { 0, 0, 1 } },

            // Quad
            { { -objSize, -objSize }, { 1, 0, 0 } },
            { { -objSize,  objSize }, { 1, 0, 0 } },
            { {  objSize, -objSize }, { 1, 1, 0 } },
            { {  objSize,  objSize }, { 1, 1, 0 } },
        };

        // Vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float  }); // position has 2 float components
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB32Float }); // color has 3 float components

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);             // Size (in bytes) of the vertex buffer
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;      // Vertex format layout
        }
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create shaders
        LLGL::Shader* vertShader = nullptr;
        LLGL::Shader* geomShader = nullptr;
        LLGL::Shader* fragShader = nullptr;

        // Load vertex, geometry, and fragment shaders from file
        auto HasLanguage = [&](const LLGL::ShadingLanguage lang)
        {
            const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
            return (std::find(languages.begin(), languages.end(), lang) != languages.end());
        };

        LLGL::ShaderDescriptor vertShaderDesc, geomShaderDesc, fragShaderDesc;

        if (HasLanguage(LLGL::ShadingLanguage::GLSL))
        {
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.vert" };
            geomShaderDesc = { LLGL::ShaderType::Geometry, "Example.geom" };
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.frag" };
        }
        else if (HasLanguage(LLGL::ShadingLanguage::SPIRV))
        {
            vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.450core.vert.spv");
            geomShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Geometry, "Example.450core.geom.spv");
            fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv");
        }
        else if (HasLanguage(LLGL::ShadingLanguage::HLSL))
        {
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_4_0" };
            geomShaderDesc = { LLGL::ShaderType::Geometry, "Example.hlsl", "GS", "gs_4_0" };
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_4_0" };
        }
        else if (HasLanguage(LLGL::ShadingLanguage::Metal))
        {
            vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.metal", "VS", "2.0" };
            //geomShaderDesc = N/A
            fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.metal", "PS", "2.0" };
        }

        // Set vertex input attributes and create vertex shader
        vertShaderDesc.vertex.inputAttribs = vertexFormat.attributes;
        vertShader = renderer->CreateShader(vertShaderDesc);

        // Create geometry shader (if supported)
        if (geomShaderDesc.source != nullptr)
            geomShader = renderer->CreateShader(geomShaderDesc);

        // Create fragment shader
        fragShader = renderer->CreateShader(fragShaderDesc);

        // Print info log (warnings and errors)
        for (LLGL::Shader* shader : { vertShader, geomShader, fragShader })
        {
            if (shader)
            {
                if (auto report = shader->GetReport())
                {
                    if (*report->GetText() != '\0')
                        LLGL::Log::Errorf("%s", report->GetText());
                }
            }
        }

        // Create graphics pipeline
        LLGL::PipelineState* pipeline[2] = {};
        const bool logicOpSupported = renderer->GetRenderingCaps().features.hasLogicOp;

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = vertShader;
            pipelineDesc.geometryShader                 = geomShader;
            pipelineDesc.fragmentShader                 = fragShader;
            pipelineDesc.renderPass                     = swapChain1->GetRenderPass();
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.rasterizer.multiSampleEnabled  = (swapChain1->GetSamples() > 1);
        }
        pipeline[0] = renderer->CreatePipelineState(pipelineDesc);

        {
            pipelineDesc.renderPass                     = swapChain2->GetRenderPass();
            pipelineDesc.rasterizer.multiSampleEnabled  = (swapChain2->GetSamples() > 1);

            // Only enable logic operations if it's supported, otherwise an exception is thrown
            if (logicOpSupported)
                pipelineDesc.blend.logicOp = LLGL::LogicOp::CopyInverted;
        }
        pipeline[1] = renderer->CreatePipelineState(pipelineDesc);

        for (LLGL::PipelineState* p : pipeline)
        {
            if (const LLGL::Report* report = p->GetReport())
            {
                if (report->HasErrors())
                    LLGL_THROW_RUNTIME_ERROR(report->GetText());
            }
        }

        // Initialize viewport array
        auto BuildViewports = [](const LLGL::SwapChain& swapChain, LLGL::Viewport (&outViewports)[2])
        {
            const LLGL::Extent2D swapChainRes = swapChain.GetResolution();
            const float w = static_cast<float>(swapChainRes.width);
            const float h = static_cast<float>(swapChainRes.height);
            outViewports[0] = LLGL::Viewport{ 0.0f, 0.0f, w/2, h };
            outViewports[1] = LLGL::Viewport{  w/2, 0.0f, w/2, h };
        };

        const float backgroundColor[2][4] =
        {
            { 0.2f, 0.2f, 0.5f, 1 },
            { 0.5f, 0.2f, 0.2f, 1 },
        };

        bool enableLogicOp[2] = { false, false };

        if (logicOpSupported)
            LLGL::Log::Printf("Press SPACE to enabled/disable logic fragment operations\n");

        // Generate multiple-instances via geometry shader.
        // Otherwise, use instanced rendering if geometry shaders are not supported (for Metal shading language).
        const std::uint32_t numInstances = (geomShader != nullptr ? 1 : 2);

        // Enter main loop
        while (!(inputs[0].KeyPressed(LLGL::Key::Escape) || inputs[1].KeyPressed(LLGL::Key::Escape)))
        {
            // Process events of both windows and quit when both windows are closed
            LLGL::Surface::ProcessEvents();

            if (window1.HasQuit() && window2.HasQuit())
                break;

            // Switch between pipeline states
            for (int i = 0; i < 2; ++i)
            {
                if (inputs[i].KeyDown(LLGL::Key::Space))
                {
                    if (logicOpSupported)
                    {
                        enableLogicOp[i] = !enableLogicOp[i];
                        LLGL::Log::Printf("Logic Fragment Operation %s (Window %d)\n", (enableLogicOp[i] ? "Enabled" : "Disabled"), (i + 1));
                    }
                    else
                        LLGL::Log::Printf("Logic Fragment Operation Not Supported\n");
                }
            }

            // Start encoding commands
            commands->Begin();
            {
                // Set global render states: viewports, vertex buffer, and graphics pipeline
                //commands->SetVertexBuffer(*vertexBuffer);

                LLGL::Viewport viewports[2];

                // Draw triangle with 3 vertices in 1st swap-chain
                if (window1.IsShown())
                {
                    commands->BeginRenderPass(*swapChain1);
                    {
                        BuildViewports(*swapChain1, viewports);
                        commands->Clear(LLGL::ClearFlags::Color, backgroundColor[0]);
                        commands->SetPipelineState(*pipeline[0]);//[enableLogicOp[0] ? 1 : 0]);
                        commands->SetViewports(2, viewports);
                        commands->SetVertexBuffer(*vertexBuffer);
                        commands->DrawInstanced(3, 0, numInstances);
                    }
                    commands->EndRenderPass();
                }

                // Draw quad with 4 vertices in 2nd swap-chain
                if (window2.IsShown())
                {
                    commands->BeginRenderPass(*swapChain2);
                    {
                        BuildViewports(*swapChain1, viewports);
                        commands->Clear(LLGL::ClearFlags::Color, backgroundColor[1]);
                        commands->SetPipelineState(*pipeline[1]);//[enableLogicOp[1] ? 1 : 0]);
                        commands->SetViewports(2, viewports);
                        commands->SetVertexBuffer(*vertexBuffer);
                        commands->DrawInstanced(4, 3, numInstances);
                    }
                    commands->EndRenderPass();
                }
            }
            commands->End();
            commandQueue->Submit(*commands);

            // Present the results on the screen
            if (window1.IsShown())
                swapChain1->Present();
            if (window2.IsShown())
                swapChain2->Present();

            inputs[0].Reset();
            inputs[1].Reset();
        }
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
    }
    return 0;
}
