/*
 * main.cpp (Tutorial01_HelloTriangle)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <tutorial.h>
#include <chrono>


// Enable multi-sampling render context
#define ENABLE_MULTISAMPLING

// Enable timer to show render times every second
//#define ENABLE_TIMER

int main(int argc, char* argv[])
{
    try
    {
        // Let the user choose an available renderer
        std::string rendererModule = GetSelectedRendererModule(argc, argv);

        // Load render system module
        std::unique_ptr<LLGL::RenderSystem> renderer = LLGL::RenderSystem::Load(rendererModule);

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution            = { 800, 600 };
            contextDesc.vsync.enabled                   = true;
            contextDesc.profileOpenGL.contextProfile    = LLGL::OpenGLContextProfile::CoreProfile;
            #ifdef ENABLE_MULTISAMPLING
            contextDesc.multiSampling                   = LLGL::MultiSamplingDescriptor { 20 };
            #endif
        }
        LLGL::RenderContext* context = renderer->CreateRenderContext(contextDesc);

        // Print renderer information
        const auto& info = renderer->GetRendererInfo();

        std::cout << "Renderer:         " << info.rendererName << std::endl;
        std::cout << "Device:           " << info.deviceName << std::endl;
        std::cout << "Vendor:           " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        // Set window title and show window
        auto& window = static_cast<LLGL::Window&>(context->GetSurface());

        window.SetTitle(L"LLGL Tutorial 01: Hello Triangle");
        window.Show();

        // Vertex data structure
        struct Vertex
        {
            Gs::Vector2f        position;
            LLGL::ColorRGBAub   color;
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
        vertexFormat.stride = sizeof(Vertex);

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.type                   = LLGL::BufferType::Vertex;
            vertexBufferDesc.size                   = sizeof(vertices);         // Size (in bytes) of the vertex buffer
            vertexBufferDesc.vertexBuffer.format    = vertexFormat;             // Vertex format layout
        }
        LLGL::Buffer* vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create shaders
        LLGL::Shader* vertShader = nullptr;
        LLGL::Shader* fragShader = nullptr;

        const auto& languages = renderer->GetRenderingCaps().shadingLanguages;

        if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
        {
            #ifdef __APPLE__
            vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "vertex.140core.glsl"   });
            fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "fragment.140core.glsl" });
            #else
            vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "vertex.glsl"   });
            fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "fragment.glsl" });
            #endif
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
        {
            vertShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Vertex, "vertex.450core.spv"));
            fragShader = renderer->CreateShader(LLGL::ShaderDescFromFile(LLGL::ShaderType::Fragment, "fragment.450core.spv"));
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
        {
            vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "shader.hlsl", "VS", "vs_4_0" });
            fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "shader.hlsl", "PS", "ps_4_0" });
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::Metal) != languages.end())
        {
            vertShader = renderer->CreateShader({ LLGL::ShaderType::Vertex,   "shader.metal", "VS", "1.1" });
            fragShader = renderer->CreateShader({ LLGL::ShaderType::Fragment, "shader.metal", "PS", "1.1" });
        }

        for (auto shader : { vertShader, fragShader })
        {
            if (shader != nullptr)
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
            shaderProgramDesc.fragmentShader    = fragShader;
        }
        LLGL::ShaderProgram* shaderProgram = renderer->CreateShaderProgram(shaderProgramDesc);

        // Link shader program and check for errors
        if (shaderProgram->HasErrors())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.renderPass                 = context->GetRenderPass();
            #ifdef ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampling   = contextDesc.multiSampling;
            #endif
        }
        LLGL::GraphicsPipeline* pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Get command queue to record and submit command buffers
        LLGL::CommandQueue* queue = renderer->GetCommandQueue();

        // Create command buffer to submit subsequent graphics commands to the GPU
        LLGL::CommandBuffer* commands = renderer->CreateCommandBuffer();

        // Get resolution to determine viewport size
        const auto resolution = contextDesc.videoMode.resolution;

        #ifdef ENABLE_TIMER
        auto timer = LLGL::Timer::Create();
        auto start = std::chrono::system_clock::now();
        #endif

        // Enter main loop
        while (window.ProcessEvents())
        {
            #ifdef ENABLE_TIMER
            timer->MeasureTime();
            auto end = std::chrono::system_clock::now();
            if (std::chrono::duration_cast<std::chrono::seconds>(end - start).count() > 0)
            {
                std::cout << "Rendertime: " << timer->GetDeltaTime() << ", FPS: " << 1.0 / timer->GetDeltaTime() << '\n';
                start = end;
            }
            #endif

            // Begin recording commands
            commands->Begin();
            {
                // Set viewport and scissor rectangle
                commands->SetViewport(LLGL::Viewport{ { 0, 0 }, resolution });

                // Set graphics pipeline
                commands->SetGraphicsPipeline(*pipeline);

                // Set vertex buffer
                commands->SetVertexBuffer(*vertexBuffer);

                // Set the render context as the initial render target
                commands->BeginRenderPass(*context);
                {
                    // Clear color buffer
                    commands->Clear(LLGL::ClearFlags::Color);

                    // Draw triangle with 3 vertices
                    commands->Draw(3, 0);
                }
                commands->EndRenderPass();
            }
            commands->End();
            queue->Submit(*commands);

            // Present the result on the screen
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
