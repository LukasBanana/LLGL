/*
 * main.cpp (Tutorial12_MultiRenderer)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef _WIN32

#include "../tutorial.h"

#include <LLGL/Platform/NativeHandle.h>


static void CompileShader(LLGL::Shader* shader, const LLGL::ShaderSource& code)
{
    // Compile shader
    shader->Compile(code);

    // Print info log (warnings and errors)
    std::string log = shader->QueryInfoLog();
    if (!log.empty())
        std::cerr << log << std::endl;
};

int main(int argc, char* argv[])
{
    try
    {
        // Create main window
        LLGL::WindowDescriptor mainWindowDesc;
        {
            mainWindowDesc.title    = L"LLGL Tutorial 12: Multi Renderer";
            mainWindowDesc.size     = { 800, 600 };
            mainWindowDesc.centered = true;
        }
        auto mainWindow = LLGL::Window::Create(mainWindowDesc);

        // Get native handle (HWND for Win32) from main window
        LLGL::NativeHandle mainWindowHandle;
        mainWindow->GetNativeHandle(&mainWindowHandle);

        // Copy native handle from main window into context handle as parent window
        LLGL::NativeContextHandle mainWindowContextHandle;
        mainWindowContextHandle.parentWindow = mainWindowHandle.window;

        // Create sub window for 1st renderer
        LLGL::WindowDescriptor subWindow0Desc;
        {
            subWindow0Desc.position         = {   0,   0 };
            subWindow0Desc.size             = { 400, 600 };
            subWindow0Desc.borderless       = true;
            subWindow0Desc.visible          = true;
            subWindow0Desc.windowContext    = (&mainWindowContextHandle);
        }
        std::shared_ptr<LLGL::Window> subWindow0 = LLGL::Window::Create(subWindow0Desc);

        // Load render system module
        Tutorial::Debugger deb;
        std::shared_ptr<LLGL::RenderSystem> rendererGL = LLGL::RenderSystem::Load("OpenGL", nullptr, &deb);

        // Create render context
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution    = { 400, 600 };
            contextDesc.multiSampling           = LLGL::MultiSamplingDescriptor(8);
        }
        LLGL::RenderContext* contextGL = rendererGL->CreateRenderContext(contextDesc, subWindow0);

        // Show main window
        mainWindow->Show();

        // Vertex data (3 vertices for our triangle)
        auto cubeVertices = Tutorial::GenerateTexturedCubeVertices();
        auto cubeIndices = Tutorial::GenerateTexturedCubeTriangleIndices();

        // Vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::VectorType::Float3 });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::VectorType::Float2 });

        // Create vertex buffer
        const auto vertexBufferDesc = LLGL::VertexBufferDesc(sizeof(Tutorial::VertexPositionTexCoord) * cubeVertices.size(), vertexFormat);
        auto vertexBufferGL = rendererGL->CreateBuffer(vertexBufferDesc, cubeVertices.data());

        // Create index buffer
        const auto indexBufferDesc = LLGL::IndexBufferDesc(sizeof(unsigned int) * cubeIndices.size(), LLGL::DataType::UInt32);
        auto indexBufferGL = rendererGL->CreateBuffer(indexBufferDesc, cubeIndices.data());

        // Create constant buffer
        struct Matrices
        {
            Gs::Matrix4f wvpMatrix;
        }
        matrices;

        const auto constBufferDesc = LLGL::ConstantBufferDesc(sizeof(matrices));
        auto constBufferGL = rendererGL->CreateBuffer(constBufferDesc);

        auto CreateSceneShader = [&vertexFormat](LLGL::RenderSystem* renderer)
        {
            // Create shaders
            LLGL::Shader* vertShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
            LLGL::Shader* fragShader = renderer->CreateShader(LLGL::ShaderType::Fragment);

            if (renderer->GetRenderingCaps().shadingLanguage >= LLGL::ShadingLanguage::HLSL_2_0)
            {
                auto shaderCode = ReadFileContent("shader.hlsl");
                CompileShader(vertShader, LLGL::ShaderSource(shaderCode, "VS", "vs_4_0"));
                CompileShader(fragShader, LLGL::ShaderSource(shaderCode, "PS", "ps_4_0"));
            }
            else
            {
                CompileShader(vertShader, ReadFileContent("shader.VS.vert"));
                CompileShader(fragShader, ReadFileContent("shader.PS.frag"));
            }

            // Create shader program which is used as composite
            LLGL::ShaderProgram* shaderProgram = renderer->CreateShaderProgram();

            // Attach vertex- and fragment shader to the shader program
            shaderProgram->AttachShader(*vertShader);
            shaderProgram->AttachShader(*fragShader);

            // Bind vertex attribute layout (this is not required for a compute shader program)
            shaderProgram->BuildInputLayout(vertexFormat);
        
            // Link shader program and check for errors
            if (!shaderProgram->LinkShaders())
                throw std::runtime_error(shaderProgram->QueryInfoLog());

            return shaderProgram;
        };

        // Create texture
        auto textureGL = Tutorial::LoadTextureWithRenderer(*rendererGL, "../Media/Textures/Logo_OpenGL.png");

        // Create sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.maxAnisotropy = 8;
        }
        auto samplerGL = rendererGL->CreateSampler(samplerDesc);

        // Create shader program
        auto shaderProgramGL = CreateSceneShader(rendererGL.get());

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDescGL;
        {
            pipelineDescGL.shaderProgram            = shaderProgramGL;
            pipelineDescGL.depth.testEnabled        = true;
            pipelineDescGL.depth.writeEnabled       = true;
            pipelineDescGL.rasterizer.multiSampling = contextDesc.multiSampling;
        }
        LLGL::GraphicsPipeline* pipelineGL = rendererGL->CreateGraphicsPipeline(pipelineDescGL);

        // Create command buffer to submit subsequent graphics commands to the GPU
        LLGL::CommandBuffer* commandsGL = rendererGL->CreateCommandBuffer();
        
        // Set the render context as the initial render target
        commandsGL->SetRenderTarget(*contextGL);

        commandsGL->SetClearColor({ 0.1f, 0.1f, 0.4f });

        // Set viewport (this guaranteed to be a persistent state)
        commandsGL->SetViewport({ 0, 0, 800, 600 });

        // Initialize matrices
        Gs::Matrix4f projMatrix, viewMatrix, worldMatrix;
        projMatrix = Gs::ProjectionMatrix4f::Perspective(800.0f / 600.0f, 0.1f, 100.0f, Gs::Deg2Rad(45.0f), Gs::ProjectionFlags::Direct3DPreset).ToMatrix4();
        Gs::Translate(viewMatrix, Gs::Vector3f(0, 0, 5));

        // Enter main loop
        while (mainWindow->ProcessEvents())
        {
            // Update scene transformation
            Gs::RotateFree(worldMatrix, Gs::Vector3f(0, 1, 0), Gs::Deg2Rad(0.005f));
            matrices.wvpMatrix = projMatrix * viewMatrix * worldMatrix;

            // Clear color buffer
            commandsGL->Clear(LLGL::ClearFlags::ColorDepth);

            // Update constant buffer
            rendererGL->WriteBuffer(*constBufferGL, &matrices, sizeof(matrices), 0);

            // Set graphics pipeline and vertex buffer
            commandsGL->SetGraphicsPipeline(*pipelineGL);
            commandsGL->SetConstantBuffer(*constBufferGL, 0);
            commandsGL->SetVertexBuffer(*vertexBufferGL);
            commandsGL->SetIndexBuffer(*indexBufferGL);
            commandsGL->SetSampler(*samplerGL, 0);
            commandsGL->SetTexture(*textureGL, 0);

            // Draw triangulated cube
            commandsGL->DrawIndexed(36, 0);

            // Present the result on the screen
            contextGL->Present();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        system("pause");
    }
    return 0;
}

#else

#include <iostream>

int main()
{
    std::cerr << "this tutorial is only available for the Win32 platform" << std::endl;
    return 0;
}

#endif

