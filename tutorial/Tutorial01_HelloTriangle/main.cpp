/*
 * main.cpp (Tutorial01_HelloTriangle)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


//#define ENABLE_MULTISAMPLING

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
            contextDesc.videoMode.resolution    = { 800, 600 };
            #ifdef ENABLE_MULTISAMPLING
            contextDesc.multiSampling           = LLGL::MultiSamplingDescriptor(8);
            #endif
        }
        LLGL::RenderContext* context = renderer->CreateRenderContext(contextDesc);

        // Set window title and show window
        auto& window = static_cast<LLGL::Window&>(context->GetSurface());

        window.SetTitle(L"LLGL Tutorial 01: Hello Triangle");
        window.Show();

        // Vertex data structure
        struct Vertex
        {
            Gs::Vector2f    position;
            LLGL::ColorRGBf color;
        };

        // Vertex data (3 vertices for our triangle)
        const float s = 0.5f;

        Vertex vertices[] =
        {
            { {  0,  s }, { 1, 0, 0 } }, // 1st vertex: center-top, red
            { {  s, -s }, { 0, 1, 0 } }, // 2nd vertex: right-bottom, green
            { { -s, -s }, { 0, 0, 1 } }, // 3rd vertex: left-bottom, blue
        };

        // Vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::VectorType::Float2 }); // position has 2 float components
        vertexFormat.AppendAttribute({ "color",    LLGL::VectorType::Float3 }); // color has 3 float components

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
        auto ReadFileContent = [](const std::string& filename)
        {
            // Read file and check for failure
            std::ifstream file(filename);

            if (!file.good())
                throw std::runtime_error("failed to read file: \"" + filename + "\"");

            return std::string(
                ( std::istreambuf_iterator<char>(file) ),
                ( std::istreambuf_iterator<char>() )
            );
        };

        // Load vertex - and fragment shader code from file
        auto CompileShader = [](LLGL::Shader* shader, const std::string& source, const LLGL::ShaderDescriptor& shaderDesc = {})
        {
            // Compile shader
            shader->Compile(source, shaderDesc);

            // Print info log (warnings and errors)
            std::string log = shader->QueryInfoLog();
            if (!log.empty())
                std::cerr << log << std::endl;
        };

        if (renderer->GetRenderingCaps().shadingLanguage >= LLGL::ShadingLanguage::HLSL_2_0)
        {
            auto shaderCode = ReadFileContent("shader.hlsl");
            CompileShader(vertexShader, shaderCode, LLGL::ShaderDescriptor("VS", "vs_4_0"));
            CompileShader(fragmentShader, shaderCode, LLGL::ShaderDescriptor("PS", "ps_4_0"));
        }
        else
        {
            #ifdef __APPLE__
            CompileShader(vertexShader, ReadFileContent("vertex.140core.glsl"));
            CompileShader(fragmentShader, ReadFileContent("fragment.140core.glsl"));
            #else
            CompileShader(vertexShader, ReadFileContent("vertex.glsl"));
            CompileShader(fragmentShader, ReadFileContent("fragment.glsl"));
            #endif
        }

        // Create shader program which is used as composite
        LLGL::ShaderProgram* shaderProgram = renderer->CreateShaderProgram();

        // Attach vertex- and fragment shader to the shader program
        shaderProgram->AttachShader(*vertexShader);
        shaderProgram->AttachShader(*fragmentShader);

        // Bind vertex attribute layout (this is not required for a compute shader program)
        shaderProgram->BuildInputLayout(vertexFormat);
        
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
        
        // Set viewport (this guaranteed to be a persistent state)
        commands->SetViewport({ 0, 0, 800, 600 });
        commands->SetScissor({ 0, 0, 800, 600 });

        // Enter main loop
        while (window.ProcessEvents())
        {
            // Set the render context as the initial render target
            commands->SetRenderTarget(*context);

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
