/*
 * main.cpp (Tutorial01_HelloTriangle)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
        std::shared_ptr<LLGL::RenderSystem> renderer = LLGL::RenderSystem::Load(rendererModule);

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution    = { 640, 480 };
            #ifdef ENABLE_MULTISAMPLING
            contextDesc.sampling.enabled        = true;
            contextDesc.sampling.samples        = 8;
            #endif
        }
        LLGL::RenderContext* context = renderer->CreateRenderContext(contextDesc);

        // Set window title and show window
        context->GetWindow().SetTitle(L"LLGL Tutorial 01: Hello Triangle");
        context->GetWindow().Show();

        // Create command buffer to submit subsequent graphics commands to the GPU
        LLGL::CommandBuffer* commands = renderer->CreateCommandBuffer();

        // Vertex data structure
        struct Vertex
        {
            Gs::Vector2f    position;
            LLGL::ColorRGBf color;
        };

        // Vertex data (3 vertices for our triangle)
        Vertex vertices[] =
        {
            { {  0,  1 }, { 1, 0, 0 } }, // 1st vertex: center-top, red
            { {  1, -1 }, { 0, 1, 0 } }, // 2nd vertex: right-bottom, green
            { { -1, -1 }, { 0, 0, 1 } }, // 3rd vertex: left-bottom, blue
        };

        // Vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::DataType::Float, 2 }); // position has 2 float components
        vertexFormat.AppendAttribute({ "color",    LLGL::DataType::Float, 3 }); // color has 3 float components

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.type                           = LLGL::BufferType::Vertex;
            vertexBufferDesc.size                           = sizeof(vertices);             // Size (in bytes) of the vertex buffer
            vertexBufferDesc.usage                          = LLGL::BufferUsage::Static;    // Buffer usage is static since we won't change it frequently
            vertexBufferDesc.vertexBuffer.vertexFormat  = vertexFormat;                 // Vertex format layout
        }
        LLGL::Buffer* vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create shaders
        LLGL::Shader* vertexShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
        LLGL::Shader* fragmentShader = renderer->CreateShader(LLGL::ShaderType::Fragment);

        // Define the lambda function to read an entire text file
        auto ReadFileContent = [](const std::string& filename)
        {
            std::ifstream file(filename);
            return std::string(
                ( std::istreambuf_iterator<char>(file) ),
                ( std::istreambuf_iterator<char>() )
            );
        };

        // Load vertex - and fragment shader code from file
        auto CompileShader = [](LLGL::Shader* shader, const LLGL::ShaderSource& code)
        {
            // Compile shader
            shader->Compile(code);

            // Print info log (warnings and errors)
            std::string log = shader->QueryInfoLog();
            if (!log.empty())
                std::cerr << log << std::endl;
        };

        if (renderer->GetRenderingCaps().shadingLanguage >= LLGL::ShadingLanguage::HLSL_2_0)
        {
            auto shaderCode = ReadFileContent("shader.hlsl");
            CompileShader(vertexShader, LLGL::ShaderSource(shaderCode, "VS", "vs_4_0"));
            CompileShader(fragmentShader, LLGL::ShaderSource(shaderCode, "PS", "ps_4_0"));
        }
        else
        {
            CompileShader(vertexShader, ReadFileContent("vertex.glsl"));
            CompileShader(fragmentShader, ReadFileContent("fragment.glsl"));
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
            pipelineDesc.shaderProgram                  = shaderProgram;
            #ifdef ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampleEnabled  = true;
            pipelineDesc.rasterizer.samples             = 8;
            #endif
        }
        LLGL::GraphicsPipeline* pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
        
        // Enter main loop
        while (context->GetWindow().ProcessEvents())
        {
            // Clear color buffer
            commands->ClearBuffers(LLGL::ClearBuffersFlags::Color);

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
    }
    return 0;
}
