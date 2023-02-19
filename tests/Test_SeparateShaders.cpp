/*
 * Test_SeparateShaders.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <LLGL/Misc/Utility.h>
#include <LLGL/Misc/VertexFormat.h>
#include <iostream>


int main(int argc, char* argv[])
{
    try
    {
        // Render system with debugger
        LLGL::RenderingDebugger debugger;
        LLGL::Log::SetReportCallbackStd(&(std::cerr));

        auto renderer = LLGL::RenderSystem::Load("OpenGL", nullptr, &debugger);

        // Swap chain and surface
        LLGL::SwapChainDescriptor swapChainDesc;
        {
            swapChainDesc.resolution = { 800, 600 };
        }
        auto swapChain = renderer->CreateSwapChain(swapChainDesc);

        auto& window = LLGL::CastTo<LLGL::Window>(swapChain->GetSurface());
        window.Show();
        window.SetTitle("LLGL Test Separate Shaders - " + LLGL::UTF8String{ renderer->GetName() });

        const auto& limits = renderer->GetRenderingCaps().limits;

        // Vertex buffer
        const struct Vertex
        {
            float   position[2];
            uint8_t color[4];
        }
        vertices[]
        {
            { +0.0f, +0.5f, 255,   0,   0, 255 },
            { +0.5f, -0.5f,   0, 255,   0, 255 },
            { -0.5f, -0.5f,   0,   0, 255, 255 },
        };

        LLGL::VertexFormat vertexFormat;
        {
            vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float  });
            vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });
        }
        auto vertexBuffer = renderer->CreateBuffer(LLGL::VertexBufferDesc(sizeof(vertices), vertexFormat), vertices);

        // Constant buffer
        struct Settings
        {
            float offset[4];
            float albedo[4];
            float pad[56]; // Padding to 256 ((4+4+56)*sizeof float) alignment per buffer view
        }
        settings[2] = {};

        settings[0].albedo[0] = 1.0f;
        settings[0].albedo[1] = 1.0f;
        settings[0].albedo[2] = 1.0f;
        settings[0].albedo[3] = 1.0f;

        settings[1].offset[0] = 0.2f;
        settings[1].albedo[0] = 1.0f;
        settings[1].albedo[1] = 0.0f;
        settings[1].albedo[2] = 0.0f;
        settings[1].albedo[3] = 1.0f;

        auto constantBuffer = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(settings)), settings);

        // Pipeline layout
        auto layout = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(Settings@0):vert:frag"));

        // Shaders
        auto CreateSeparateShader = [&renderer, &vertexFormat](LLGL::ShaderType type, std::string filename)
        {
            filename = "Shaders/" + filename;
            auto shaderDesc = LLGL::ShaderDescFromFile(type, filename.c_str(), nullptr, nullptr, LLGL::ShaderCompileFlags::SeparateShader);
            if (type == LLGL::ShaderType::Vertex)
                shaderDesc.vertex.inputAttribs = vertexFormat.attributes;
            return renderer->CreateShader(shaderDesc);
        };

        auto vShader = CreateSeparateShader(LLGL::ShaderType::Vertex,   "SeparateShaderTest.vert");
        auto fShader = CreateSeparateShader(LLGL::ShaderType::Fragment, "SeparateShaderTest.frag");

        #if 0
        // Shader reflection
        LLGL::ShaderReflection reflection;
        shaderProgram->Reflect(reflection);

        auto colorMapALoc = shaderProgram->FindUniformLocation("colorMapA");
        auto colorMapBLoc = shaderProgram->FindUniformLocation("colorMapB");
        #endif

        // PSO
        LLGL::GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout  = layout;
            psoDesc.vertexShader    = vShader;
            psoDesc.fragmentShader  = fShader;
            psoDesc.renderPass      = swapChain->GetRenderPass();
        }
        auto pso = renderer->CreatePipelineState(psoDesc);

        if (auto report = pso->GetReport())
        {
            std::cerr << report->GetText() << std::endl;
            if (report->HasErrors())
            {
                #ifdef _WIN32
                system("pause");
                #endif
                return 0;
            }
        }

        // Resource heap
        LLGL::ResourceViewDescriptor resourceViews[] =
        {
            LLGL::ResourceViewDescriptor{ constantBuffer, LLGL::BufferViewDescriptor{ LLGL::Format::Undefined,                   0, sizeof(settings[0]) } },
            LLGL::ResourceViewDescriptor{ constantBuffer, LLGL::BufferViewDescriptor{ LLGL::Format::Undefined, sizeof(settings[0]), sizeof(settings[1]) } },
        };
        auto resources = renderer->CreateResourceHeap(layout, resourceViews);

        // Main loop
        const LLGL::ColorRGBAf backgroundColor{ 0.2f, 0.2f, 0.4f };

        auto queue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer(LLGL::CommandBufferFlags::ImmediateSubmit);

        LLGL::Input input{ window };
        std::uint32_t resSet = 0;

        while (window.ProcessEvents() && !input.KeyPressed(LLGL::Key::Escape))
        {
            if (input.KeyDown(LLGL::Key::Tab))
            {
                resSet = 1 - resSet;
                std::cout << "Switched to resource heap: " << resSet << std::endl;
            }

            commands->Begin();
            {
                commands->BeginRenderPass(*swapChain);
                {
                    // Set viewport and clear
                    commands->SetViewport(swapChain->GetResolution());
                    commands->Clear(LLGL::ClearFlags::Color, backgroundColor);

                    // Bind resources
                    commands->SetPipelineState(*pso);
                    commands->SetVertexBuffer(*vertexBuffer);
                    commands->SetResourceHeap(*resources, resSet);

                    // Draw
                    commands->Draw(3, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            queue->Submit(*commands);
            swapChain->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
