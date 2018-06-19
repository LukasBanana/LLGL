/*
 * main.cpp (Tutorial01_HelloTriangle)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <tutorial.h>
#include <chrono>


// Enable multi-sampling render context
//#define ENABLE_MULTISAMPLING

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
            contextDesc.multiSampling                   = LLGL::MultiSamplingDescriptor { 8 };
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
        LLGL::Shader* vertexShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
        LLGL::Shader* fragmentShader = renderer->CreateShader(LLGL::ShaderType::Fragment);

        // Define the lambda function to read an entire text file
        auto ReadTextFile = [](const std::string& filename)
        {
            // Read file and check for failure
            std::ifstream file { filename };

            if (!file.good())
                throw std::runtime_error("failed to read file: \"" + filename + "\"");

            return std::string(
                ( std::istreambuf_iterator<char>(file) ),
                ( std::istreambuf_iterator<char>() )
            );
        };

        auto ReadBinaryFile = [](const std::string& filename)
        {
            // Read file and for failure
            std::ifstream file { filename, std::ios_base::binary | std::ios_base::ate };

            if (!file.good())
                throw std::runtime_error("failed to read file: \"" + filename + "\"");

            const auto fileSize = static_cast<size_t>(file.tellg());
            std::vector<char> buffer(fileSize);

            file.seekg(0);
            file.read(buffer.data(), fileSize);

            return buffer;
        };

        // Load vertex - and fragment shader code from file
        auto CompileShader = [](LLGL::Shader* shader, const std::string& source, std::vector<char> byteCode = {}, const LLGL::ShaderDescriptor& shaderDesc = {})
        {
            // Compile shader
            if (byteCode.empty())
                shader->Compile(source, shaderDesc);
            else
                shader->LoadBinary(std::move(byteCode), shaderDesc);

            // Print info log (warnings and errors)
            std::string log = shader->QueryInfoLog();
            if (!log.empty())
                std::cerr << log << std::endl;
        };

        const auto& languages = renderer->GetRenderingCaps().shadingLanguages;

        if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::GLSL) != languages.end())
        {
            #ifdef __APPLE__
            CompileShader(vertexShader, ReadTextFile("vertex.140core.glsl"));
            CompileShader(fragmentShader, ReadTextFile("fragment.140core.glsl"));
            #else
            CompileShader(vertexShader, ReadTextFile("vertex.glsl"));
            CompileShader(fragmentShader, ReadTextFile("fragment.glsl"));
            #endif
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::SPIRV) != languages.end())
        {
            CompileShader(vertexShader, "", ReadBinaryFile("vertex.450core.spv"));
            CompileShader(fragmentShader, "", ReadBinaryFile("fragment.450core.spv"));
        }
        else if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
        {
            auto shaderCode = ReadTextFile("shader.hlsl");
            CompileShader(vertexShader, shaderCode, {}, LLGL::ShaderDescriptor("VS", "vs_4_0"));
            CompileShader(fragmentShader, shaderCode, {}, LLGL::ShaderDescriptor("PS", "ps_4_0"));
        }

        // Create shader program which is used as composite
        LLGL::ShaderProgram* shaderProgram = renderer->CreateShaderProgram();

        // Attach vertex- and fragment shader to the shader program
        shaderProgram->AttachShader(*vertexShader);
        shaderProgram->AttachShader(*fragmentShader);

        // Bind vertex attribute layout (this is not required for a compute shader program)
        shaderProgram->BuildInputLayout(1, &vertexFormat);

        // Link shader program and check for errors
        if (!shaderProgram->LinkShaders())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            #ifdef ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampling   = contextDesc.multiSampling;
            #endif
        }
        LLGL::GraphicsPipeline* pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

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

            // Set the render context as the initial render target
            commands->SetRenderTarget(*context);

            // Set viewport and scissor rectangle
            commands->SetViewport(LLGL::Viewport{ { 0, 0 }, resolution });

            // Clear color buffer
            commands->Clear(LLGL::ClearFlags::Color);

            // Set graphics pipeline
            commands->SetGraphicsPipeline(*pipeline);

            // Set vertex buffer
            commands->SetVertexBuffer(*vertexBuffer);

            // Draw triangle with 3 vertices
            commands->Draw(3, 0);

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
