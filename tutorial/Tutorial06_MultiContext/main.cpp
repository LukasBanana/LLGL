/*
 * main.cpp (Tutorial06_MultiContext)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/LLGL.h>
#include <Gauss/Gauss.h>
#include <iostream>
#include <fstream>


int main()
{
    try
    {
        // Load render system module
        auto renderer = LLGL::RenderSystem::Load("OpenGL");

        std::cout << "LLGL Renderer: " << renderer->GetName() << std::endl;

        // Create two render contexts
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution    = { 640, 480 };
            contextDesc.sampling.enabled        = true;
            contextDesc.sampling.samples        = 8;
        }
        auto context1 = renderer->CreateRenderContext(contextDesc);
        auto context2 = renderer->CreateRenderContext(contextDesc);

        // Create input handler
        auto input = std::make_shared<LLGL::Input>();

        context1->GetWindow().AddEventListener(input);
        context2->GetWindow().AddEventListener(input);

        // Set window titles
        context1->GetWindow().SetTitle(L"LLGL Tutorial 06: Multi Context (1)");
        context2->GetWindow().SetTitle(L"LLGL Tutorial 06: Multi Context (2)");

        // Set window positions
        auto desktopResolution = LLGL::Desktop::GetResolution();
        context1->GetWindow().SetPosition({ desktopResolution.x/2 - 700, desktopResolution.y/2 - 480/2 });
        context2->GetWindow().SetPosition({ desktopResolution.x/2 + 700 - 640, desktopResolution.y/2 - 480/2 });

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
        auto vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create shaders
        auto vertexShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
        auto geometryShader = renderer->CreateShader(LLGL::ShaderType::Geometry);
        auto fragmentShader = renderer->CreateShader(LLGL::ShaderType::Fragment);

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
            CompileShader(geometryShader, LLGL::ShaderSource(shaderCode, "GS", "gs_4_0"));
            CompileShader(fragmentShader, LLGL::ShaderSource(shaderCode, "PS", "ps_4_0"));
        }
        else
        {
            CompileShader(vertexShader, ReadFileContent("vertex.glsl"));
            CompileShader(geometryShader, ReadFileContent("geometry.glsl"));
            CompileShader(fragmentShader, ReadFileContent("fragment.glsl"));
        }

        // Create shader program which is used as composite
        auto shaderProgram = renderer->CreateShaderProgram();

        // Attach vertex- and fragment shader to the shader program
        shaderProgram->AttachShader(*vertexShader);
        shaderProgram->AttachShader(*geometryShader);
        shaderProgram->AttachShader(*fragmentShader);

        // Bind vertex attribute layout (this is not required for a compute shader program)
        shaderProgram->BuildInputLayout(vertexFormat);
        
        // Link shader program and check for errors
        if (!shaderProgram->LinkShaders())
            throw std::runtime_error(shaderProgram->QueryInfoLog());

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.rasterizer.sampling.enabled    = true;
            pipelineDesc.rasterizer.sampling.samples    = 8;
        }
        auto pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Initialize viewport array
        LLGL::Viewport viewports[2] =
        {
            LLGL::Viewport(0, 0, 320, 480),
            LLGL::Viewport(320, 0, 320, 480),
        };
        
        // Enter main loop
        while ( ( context1->GetWindow().ProcessEvents() || context2->GetWindow().ProcessEvents() ) && !input->KeyPressed(LLGL::Key::Escape) )
        {
            // Draw content in 1st render context
            {
                // Make 1st render context to the current one
                renderer->MakeCurrent(context1);

                // Set viewport array
                context1->SetViewportArray(2, viewports);

                // Set graphics pipeline
                context1->SetGraphicsPipeline(*pipeline);

                // Set vertex buffer
                context1->SetVertexBuffer(*vertexBuffer);

                // Clear color buffer
                context1->ClearBuffers(LLGL::ClearBuffersFlags::Color);

                // Draw triangle with 3 vertices
                context1->Draw(3, 0);

                // Present the result on the screen
                context1->Present();
            }

            // Draw content in 2nd render context
            {
                // Make 2nd render context to the current one
                renderer->MakeCurrent(context2);

                // Set viewport array
                context2->SetViewportArray(2, viewports);

                // Set graphics pipeline
                context2->SetGraphicsPipeline(*pipeline);

                // Set vertex buffer
                context2->SetVertexBuffer(*vertexBuffer);

                // Clear color buffer
                context2->ClearBuffers(LLGL::ClearBuffersFlags::Color);

                // Draw quad with 4 vertices
                context2->Draw(4, 3);

                // Present the result on the screen
                context2->Present();
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
