/*
 * Test5_Vulkan.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Helper.h"


//#define TEST_RENDER_TARGET
//#define TEST_QUERY
//#define TEST_STORAGE_BUFFER


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("Vulkan");

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;

        contextDesc.videoMode.resolution        = { 800, 600 };
        //contextDesc.videoMode.fullscreen        = true;

        contextDesc.multiSampling.enabled       = true;
        contextDesc.multiSampling.samples       = 8;

        contextDesc.vsync.enabled               = true;

        LLGL::WindowDescriptor windowDesc;
        {
            windowDesc.size     = contextDesc.videoMode.resolution;
            windowDesc.centered = true;
            windowDesc.visible  = true;
        }
        auto window = std::shared_ptr<LLGL::Window>(std::move(LLGL::Window::Create(windowDesc)));

        window->SetTitle(L"LLGL Vulkan Test");

        auto context = renderer->CreateRenderContext(contextDesc, window);

        // create command buffer
        auto commands = renderer->CreateCommandBuffer();

        // load shaders
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

        // create shader program
        auto shaderProgram = renderer->CreateShaderProgram();

        shaderProgram->AttachShader(*shaderVert);
        shaderProgram->AttachShader(*shaderFrag);

        shaderProgram->LinkShaders();

        // create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;

        const auto resolution = contextDesc.videoMode.resolution;
        const auto viewportSize = resolution.Cast<float>();

        pipelineDesc.shaderProgram = shaderProgram;
        pipelineDesc.viewports.push_back(LLGL::Viewport(0.0f, 0.0f, viewportSize.x, viewportSize.y));
        pipelineDesc.scissors.push_back(LLGL::Scissor(0, 0, resolution.x, resolution.y));
        pipelineDesc.blend.targets.push_back({});

        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Print renderer information
        const auto& info = renderer->GetRendererInfo();
        const auto& caps = renderer->GetRenderingCaps();

        std::cout << "Renderer: " << info.rendererName << std::endl;
        std::cout << "Device: " << info.deviceName << std::endl;
        std::cout << "Vendor: " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        auto input = std::make_shared<LLGL::Input>();
        window->AddEventListener(input);

        while (window->ProcessEvents() && !input->KeyDown(LLGL::Key::Escape))
        {
            commands->SetClearColor({ 0.2f, 0.2f, 0.4f, 1.0f });

            commands->SetRenderTarget(*context);

            commands->Clear(LLGL::ClearFlags::ColorDepth);

            commands->SetGraphicsPipeline(*pipeline);

            commands->Draw(3, 0);

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
