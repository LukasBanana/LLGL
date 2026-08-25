/*
 * Example.cpp (Example_HelloTriangle)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Utils/TypeNames.h>


// Enable multi-sampling
#define NUM_MULTISAMPLES 8

#ifdef LLGL_OS_ANDROID
#define EXIT(VAL) return
void android_main(android_app* androidApp)
#else
#define EXIT(VAL) return (VAL)
int main(int argc, char* argv[])
#endif
{
    LLGL::Log::RegisterCallbackStd();

    // Let the user choose an available renderer
    LLGL::RenderSystemDescriptor rendererDesc;

    #ifdef LLGL_OS_ANDROID
    rendererDesc.moduleName = "OpenGLES3";
    rendererDesc.platformContext     = androidApp;
    rendererDesc.platformContextSize = sizeof(*androidApp);
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
        swapChainDesc.samples       = NUM_MULTISAMPLES; // LLGL adapts sample counts that are too high
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

    // Set window title on desktop platforms
    LLGL::Window* window = nullptr;

    if (LLGL::IsInstanceOf<LLGL::Window>(swapChain->GetSurface()))
    {
        window = LLGL::CastTo<LLGL::Window>(&swapChain->GetSurface());
        window->SetTitle(L"LLGL Example: Hello Triangle");
    }

    // Vertex data structure
    struct Vertex
    {
        float   position[2];
        uint8_t color[4];
    };

    // Vertex data (3 vertices for our triangle)
    const Vertex vertices[] =
    {
        { {  0.0f,  0.5f }, { 255, 0, 0, 255 } }, // 1st vertex: center-top, red
        { { +0.5f, -0.5f }, { 0, 255, 0, 255 } }, // 2nd vertex: right-bottom, green
        { { -0.5f, -0.5f }, { 0, 0, 255, 255 } }, // 3rd vertex: left-bottom, blue
    };

    // Create vertex buffer
    LLGL::BufferDescriptor vertexBufferDesc;
    {
        vertexBufferDesc.size       = sizeof(vertices);                 // Size (in bytes) of the vertex buffer
        vertexBufferDesc.stride     = sizeof(Vertex);                   // Stride (in bytes) between vertices; Can be 0 if we set it per `SetVertexBuffer()` call.
        vertexBufferDesc.bindFlags  = LLGL::BindFlags::VertexBuffer;    // Enables the buffer to be bound to a vertex buffer slot
    }
    LLGL::Buffer* vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

    // Create shaders from pre-compiled/pre-translated source
    LLGL::Shader* vertShader = nullptr;
    LLGL::Shader* fragShader = nullptr;

    auto IsShadingLanguageSupported = [&renderer](LLGL::ShadingLanguage lang) -> bool
    {
        const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
        return std::find(languages.begin(), languages.end(), lang) != languages.end();
    };

    LLGL::ShaderDescriptor vertShaderDesc, fragShaderDesc;

    if (IsShadingLanguageSupported(LLGL::ShadingLanguage::GLSL))
    {
        if (IsShadingLanguageSupported(LLGL::ShadingLanguage::GLSL_140))
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
    else if (IsShadingLanguageSupported(LLGL::ShadingLanguage::SPIRV))
    {
        // Load SPIR-V from binary files
        vertShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex,   "Example.450core.vert.spv");
        fragShaderDesc = LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "Example.450core.frag.spv");
    }
    else if (IsShadingLanguageSupported(LLGL::ShadingLanguage::HLSL))
    {
        vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_4_0" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_4_0" };
    }
    else if (IsShadingLanguageSupported(LLGL::ShadingLanguage::Metal))
    {
        vertShaderDesc = { LLGL::ShaderType::Vertex,   "Example.metal", "VS", "1.1" };
        fragShaderDesc = { LLGL::ShaderType::Fragment, "Example.metal", "PS", "1.1" };

        // Load Metal shaders from 'default.metallib' file that is bundled with the App folder
        vertShaderDesc.flags |= LLGL::ShaderCompileFlags::DefaultLibrary;
        fragShaderDesc.flags |= LLGL::ShaderCompileFlags::DefaultLibrary;
    }

    // Specify vertex attributes for vertex shader
    vertShader = renderer->CreateShader(vertShaderDesc);
    fragShader = renderer->CreateShader(fragShaderDesc);

    for (LLGL::Shader* shader : { vertShader, fragShader })
    {
        if (const LLGL::Report* report = shader->GetReport())
            LLGL::Log::Errorf("%s", report->GetText());
    }

    // Vertex format
    const LLGL::VertexAttribute vertexAttribs[] =
    {
        LLGL::VertexAttribute{ "position", LLGL::Format::RG32Float,  0, offsetof(Vertex, position), sizeof(Vertex) },
        LLGL::VertexAttribute{ "color",    LLGL::Format::RGBA8UNorm, 1, offsetof(Vertex, color   ), sizeof(Vertex) },
    };

    // Create graphics pipeline
    LLGL::GraphicsPipelineDescriptor pipelineDesc;
    {
        pipelineDesc.inputVertexAttribs             = vertexAttribs;
        pipelineDesc.vertexShader                   = vertShader;
        pipelineDesc.fragmentShader                 = fragShader;
        pipelineDesc.renderPass                     = swapChain->GetRenderPass();
        pipelineDesc.rasterizer.multiSampleEnabled  = (swapChainDesc.samples > 1);
    }
    LLGL::PipelineState* pipeline = renderer->CreatePipelineState(pipelineDesc);

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
    EXIT(0);
}
