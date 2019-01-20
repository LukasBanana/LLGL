/*
 * Example.cpp (Example_MultiContext)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


int main(int argc, char* argv[])
{
    try
    {
        // Set report callback to standard output
        LLGL::Log::SetReportCallbackStd();

        // Load render system module
        LLGL::RenderingDebugger debugger;
        auto renderer = LLGL::RenderSystem::Load(GetSelectedRendererModule(argc, argv), nullptr, &debugger);

        std::cout << "LLGL Renderer: " << renderer->GetName() << std::endl;

        // Create two render contexts
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution            = { 640, 480 };
            contextDesc.vsync.enabled                   = true;
            contextDesc.multiSampling                   = LLGL::MultiSamplingDescriptor(8);
            contextDesc.profileOpenGL.contextProfile    = LLGL::OpenGLContextProfile::CoreProfile;
        }
        auto context1 = renderer->CreateRenderContext(contextDesc);
        auto context2 = renderer->CreateRenderContext(contextDesc);

        // Get command queue and create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Create input handler
        auto input = std::make_shared<LLGL::Input>();

        auto& window1 = static_cast<LLGL::Window&>(context1->GetSurface());
        auto& window2 = static_cast<LLGL::Window&>(context2->GetSurface());

        window1.AddEventListener(input);
        window2.AddEventListener(input);

        // Set window titles
        window1.SetTitle(L"LLGL Example: Multi Context (1)");
        window2.SetTitle(L"LLGL Example: Multi Context (2)");

        // Set window positions
        LLGL::Extent2D desktopResolution;
        if (auto display = LLGL::Display::QueryPrimary())
            desktopResolution = display->GetDisplayMode().resolution;

        const LLGL::Offset2D desktopCenter
        {
            static_cast<int>(desktopResolution.width)/2,
            static_cast<int>(desktopResolution.height)/2
        };

        window1.SetPosition({ desktopCenter.x - 700, desktopCenter.y - 480/2 });
        window2.SetPosition({ desktopCenter.x + 700 - 640, desktopCenter.y - 480/2 });

        // Show windows
        window1.Show();
        window2.Show();

        // Vertex data structure
        struct Vertex
        {
            Gs::Vector2f    position;
            LLGL::ColorRGBf color;
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
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float }); // position has 2 float components
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGB32Float }); // color has 3 float components

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size                   = sizeof(vertices);             // Size (in bytes) of the vertex buffer
            vertexBufferDesc.bindFlags              = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexBuffer.format    = vertexFormat;                 // Vertex format layout
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

        if (HasLanguage(LLGL::ShadingLanguage::GLSL))
        {
            vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "Example.vert" });
            geomShader = renderer->CreateShader({ LLGL::ShaderType::Geometry, "Example.geom" });
            fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "Example.frag" });
        }
        else if (HasLanguage(LLGL::ShadingLanguage::SPIRV))
        {
            vertShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.450core.vert.spv"));
            geomShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Geometry, "Example.450core.geom.spv"));
            fragShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv"));
        }
        else if (HasLanguage(LLGL::ShadingLanguage::HLSL))
        {
            vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_4_0" });
            geomShader = renderer->CreateShader({ LLGL::ShaderType::Geometry, "Example.hlsl", "GS", "gs_4_0" });
            fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_4_0" });
        }
        else if (HasLanguage(LLGL::ShadingLanguage::Metal))
        {
            vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VS", "2.0" });
            //geomShader = N/A
            fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "Example.metal", "PS", "2.0" });
        }

        // Print info log (warnings and errors)
        for (auto shader : { vertShader, geomShader, fragShader })
        {
            if (shader)
            {
                std::string log = shader->QueryInfoLog();
                if (!log.empty())
                    std::cerr << log << std::endl;
            }
        }

        // Create shader program which is used as composite
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexFormats     = { vertexFormat };
            shaderProgramDesc.vertexShader      = vertShader;
            shaderProgramDesc.geometryShader    = geomShader;
            shaderProgramDesc.fragmentShader    = fragShader;
        }
        auto shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

        if (shaderProgram->HasErrors())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        // Create graphics pipeline
        LLGL::GraphicsPipeline* pipeline[2] = {};
        const bool logicOpSupported = renderer->GetRenderingCaps().features.hasLogicOp;

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.renderPass                 = context1->GetRenderPass();
            pipelineDesc.primitiveTopology          = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.rasterizer.multiSampling   = contextDesc.multiSampling;
        }
        pipeline[0] = renderer->CreateGraphicsPipeline(pipelineDesc);

        {
            // Only enable logic operations if it's supported, otherwise an exception is thrown
            if (logicOpSupported)
                pipelineDesc.blend.logicOp = LLGL::LogicOp::CopyInverted;
        }
        pipeline[1] = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Initialize viewport array
        LLGL::Viewport viewports[2] =
        {
            LLGL::Viewport {   0.0f, 0.0f, 320.0f, 480.0f },
            LLGL::Viewport { 320.0f, 0.0f, 320.0f, 480.0f },
        };

        bool enableLogicOp = false;

        if (logicOpSupported)
            std::cout << "Press SPACE to enabled/disable logic fragment operations" << std::endl;

        // Enter main loop
        while ( ( window1.ProcessEvents() || window2.ProcessEvents() ) && !input->KeyPressed(LLGL::Key::Escape) )
        {
            // Switch between pipeline states
            if (input->KeyDown(LLGL::Key::Space))
            {
                if (logicOpSupported)
                {
                    enableLogicOp = !enableLogicOp;
                    if (enableLogicOp)
                        std::cout << "Logic Fragment Operation Enabled" << std::endl;
                    else
                        std::cout << "Logic Fragment Operation Disabled" << std::endl;
                }
                else
                    std::cout << "Logic Fragment Operation Not Supported" << std::endl;
            }

            // Start encoding commands
            commands->Begin();
            {
                // Set global render states: viewports, vertex buffer, and graphics pipeline
                commands->SetViewports(2, viewports);
                commands->SetVertexBuffer(*vertexBuffer);
                commands->SetGraphicsPipeline(*pipeline[enableLogicOp ? 1 : 0]);

                // Draw triangle with 3 vertices in 1st render context
                commands->BeginRenderPass(*context1);
                {
                    commands->Draw(3, 0);
                }
                commands->EndRenderPass();

                // Draw quad with 4 vertices in 2nd render context
                commands->BeginRenderPass(*context2);
                {
                    commands->Draw(4, 3);
                }
                commands->EndRenderPass();
            }
            commands->End();
            commandQueue->Submit(*commands);

            // Present the results on the screen
            context1->Present();
            context2->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
