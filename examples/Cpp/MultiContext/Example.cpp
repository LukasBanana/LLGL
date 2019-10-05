/*
 * Example.cpp (Example_MultiContext)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
            contextDesc.videoMode.resolution    = { 640, 480 };
            contextDesc.vsync.enabled           = true;
            contextDesc.samples                 = 8;
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
        if (auto display = LLGL::Display::InstantiatePrimary())
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
        for (auto shader : { vertShader, geomShader, fragShader })
        {
            if (shader)
            {
                std::string log = shader->GetReport();
                if (!log.empty())
                    std::cerr << log << std::endl;
            }
        }

        // Create shader program which is used as composite
        LLGL::ShaderProgramDescriptor shaderProgramDesc;
        {
            shaderProgramDesc.vertexShader      = vertShader;
            shaderProgramDesc.geometryShader    = geomShader;
            shaderProgramDesc.fragmentShader    = fragShader;
        }
        auto shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

        if (shaderProgram->HasErrors())
            throw std::runtime_error(shaderProgram->GetReport());

        // Create graphics pipeline
        LLGL::PipelineState* pipeline[2] = {};
        const bool logicOpSupported = renderer->GetRenderingCaps().features.hasLogicOp;

        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.renderPass                     = context1->GetRenderPass();
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.rasterizer.multiSampleEnabled  = (contextDesc.samples > 1);
        }
        pipeline[0] = renderer->CreatePipelineState(pipelineDesc);

        {
            pipelineDesc.renderPass                     = context2->GetRenderPass();

            // Only enable logic operations if it's supported, otherwise an exception is thrown
            if (logicOpSupported)
                pipelineDesc.blend.logicOp = LLGL::LogicOp::CopyInverted;
        }
        pipeline[1] = renderer->CreatePipelineState(pipelineDesc);

        // Initialize viewport array
        LLGL::Viewport viewports[2] =
        {
            LLGL::Viewport {   0.0f, 0.0f, 320.0f, 480.0f },
            LLGL::Viewport { 320.0f, 0.0f, 320.0f, 480.0f },
        };

        bool enableLogicOp = false;

        if (logicOpSupported)
            std::cout << "Press SPACE to enabled/disable logic fragment operations" << std::endl;

        // Generate multiple-instances via geometry shader.
        // Otherwise, use instanced rendering if geometry shaders are not supported (for Metal shading language).
        std::uint32_t numInstances = (geomShader != nullptr ? 1 : 2);

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
                commands->SetPipelineState(*pipeline[enableLogicOp ? 1 : 0]);

                // Draw triangle with 3 vertices in 1st render context
                commands->BeginRenderPass(*context1);
                {
                    commands->DrawInstanced(3, 0, numInstances);
                }
                commands->EndRenderPass();

                // Draw quad with 4 vertices in 2nd render context
                commands->BeginRenderPass(*context2);
                {
                    commands->DrawInstanced(4, 3, numInstances);
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
