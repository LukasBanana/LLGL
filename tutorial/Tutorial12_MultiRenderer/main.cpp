/*
 * main.cpp (Tutorial12_MultiRenderer)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifdef _WIN32

#include "../tutorial.h"

#include <LLGL/Platform/NativeHandle.h>


static void CompileShader(LLGL::Shader* shader, const std::string& source, const LLGL::ShaderDescriptor& shaderDesc = {})
{
    // Compile shader
    shader->Compile(source, shaderDesc);

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
            mainWindowDesc.title    = L"LLGL Tutorial 12: Multi Renderer ( OpenGL and Direct3D 11 )";
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

        // Create sub window for 2nd renderer
        LLGL::WindowDescriptor subWindow1Desc;
        {
            subWindow1Desc.position         = { 400,   0 };
            subWindow1Desc.size             = { 400, 600 };
            subWindow1Desc.borderless       = true;
            subWindow1Desc.visible          = true;
            subWindow1Desc.windowContext    = (&mainWindowContextHandle);
        }
        std::shared_ptr<LLGL::Window> subWindow1 = LLGL::Window::Create(subWindow1Desc);

        // Load render systems
        auto rendererGL = LLGL::RenderSystem::Load("OpenGL");
        auto rendererD3D = LLGL::RenderSystem::Load("Direct3D11");

        // Create render contexts
        LLGL::RenderContextDescriptor contextDesc;
        {
            contextDesc.videoMode.resolution    = { 400, 600 };
            contextDesc.multiSampling           = LLGL::MultiSamplingDescriptor(8);
        }
        auto contextGL = rendererGL->CreateRenderContext(contextDesc, subWindow0);
        auto contextD3D = rendererD3D->CreateRenderContext(contextDesc, subWindow1);

        // Show main window
        mainWindow->Show();

        // Vertex data (3 vertices for our triangle)
        auto cubeVertices = Tutorial::GenerateTexturedCubeVertices();
        auto cubeIndices = Tutorial::GenerateTexturedCubeTriangleIndices();

        // Vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::VectorType::Float3 });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::VectorType::Float2 });

        // Create vertex buffers
        const auto vertexBufferDesc = LLGL::VertexBufferDesc(sizeof(Tutorial::VertexPositionTexCoord) * cubeVertices.size(), vertexFormat);
        auto vertexBufferGL = rendererGL->CreateBuffer(vertexBufferDesc, cubeVertices.data());
        auto vertexBufferD3D = rendererD3D->CreateBuffer(vertexBufferDesc, cubeVertices.data());

        // Create index buffers
        const auto indexBufferDesc = LLGL::IndexBufferDesc(sizeof(unsigned int) * cubeIndices.size(), LLGL::DataType::UInt32);
        auto indexBufferGL = rendererGL->CreateBuffer(indexBufferDesc, cubeIndices.data());
        auto indexBufferD3D = rendererD3D->CreateBuffer(indexBufferDesc, cubeIndices.data());

        // Create constant buffers
        struct Matrices
        {
            Gs::Matrix4f wvpMatrix;
        }
        matrices;

        const auto constBufferDesc = LLGL::ConstantBufferDesc(sizeof(matrices));
        auto constBufferGL = rendererGL->CreateBuffer(constBufferDesc);
        auto constBufferD3D = rendererD3D->CreateBuffer(constBufferDesc);

        auto CreateSceneShader = [&vertexFormat](LLGL::RenderSystem* renderer)
        {
            // Create shaders
            auto vertShader = renderer->CreateShader(LLGL::ShaderType::Vertex);
            auto fragShader = renderer->CreateShader(LLGL::ShaderType::Fragment);

            if (renderer->GetRenderingCaps().shadingLanguage >= LLGL::ShadingLanguage::HLSL_2_0)
            {
                auto shaderCode = ReadFileContent("shader.hlsl");
                CompileShader(vertShader, shaderCode, { "VS", "vs_4_0" });
                CompileShader(fragShader, shaderCode, { "PS", "ps_4_0" });
            }
            else
            {
                CompileShader(vertShader, ReadFileContent("shader.VS.vert"));
                CompileShader(fragShader, ReadFileContent("shader.PS.frag"));
            }

            // Create shader program which is used as composite
            auto shaderProgram = renderer->CreateShaderProgram();

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

        // Create textures
        auto textureGL = Tutorial::LoadTextureWithRenderer(*rendererGL, "../Media/Textures/Logo_OpenGL.png");
        auto textureD3D = Tutorial::LoadTextureWithRenderer(*rendererD3D, "../Media/Textures/Logo_Direct3D.png");

        // Create samplers
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.maxAnisotropy = 8;
        }
        auto samplerGL = rendererGL->CreateSampler(samplerDesc);
        auto samplerD3D = rendererD3D->CreateSampler(samplerDesc);

        // Create shader programs
        auto shaderProgramGL = CreateSceneShader(rendererGL.get());
        auto shaderProgramD3D = CreateSceneShader(rendererD3D.get());

        // Create graphics pipelines
        LLGL::GraphicsPipelineDescriptor pipelineDescGL;
        {
            pipelineDescGL.shaderProgram            = shaderProgramGL;
            pipelineDescGL.depth.testEnabled        = true;
            pipelineDescGL.depth.writeEnabled       = true;
            pipelineDescGL.rasterizer.multiSampling = contextDesc.multiSampling;
        }
        auto pipelineGL = rendererGL->CreateGraphicsPipeline(pipelineDescGL);

        LLGL::GraphicsPipelineDescriptor pipelineDescD3D;
        {
            pipelineDescD3D.shaderProgram               = shaderProgramD3D;
            pipelineDescD3D.depth.testEnabled           = true;
            pipelineDescD3D.depth.writeEnabled          = true;
            pipelineDescD3D.rasterizer.multiSampling    = contextDesc.multiSampling;
        }
        auto pipelineD3D = rendererD3D->CreateGraphicsPipeline(pipelineDescD3D);

        // Create command buffers
        auto commandsGL = rendererGL->CreateCommandBuffer();
        auto commandsD3D = rendererD3D->CreateCommandBuffer();
        
        // Set the render context as the initial render target
        commandsGL->SetRenderTarget(*contextGL);
        commandsD3D->SetRenderTarget(*contextD3D);

        // Set background color
        commandsGL->SetClearColor({ 0.1f, 0.1f, 0.4f });
        commandsD3D->SetClearColor({ 0.1f, 0.1f, 0.4f });

        // Set viewports (this guaranteed to be a persistent state)
        commandsGL->SetViewport({ 0, 0, 800, 600 });
        commandsD3D->SetViewport({ -400, 0, 800, 600 });

        // Initialize matrices (OpenGL needs a unit-cube NDC-space)
        Gs::Matrix4f projMatrixGL, projMatrixD3D, viewMatrix, worldMatrix;
        
        const float aspectRatio = static_cast<float>(mainWindowDesc.size.x) / static_cast<float>(mainWindowDesc.size.y);
        const float nearPlane   = 0.1f;
        const float farPlane    = 100.0f;
        const float fieldOfView = 45.0f;

        projMatrixGL  = Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fieldOfView), Gs::ProjectionFlags::UnitCube).ToMatrix4();
        projMatrixD3D = Gs::ProjectionMatrix4f::Perspective(aspectRatio, nearPlane, farPlane, Gs::Deg2Rad(fieldOfView)).ToMatrix4();

        Gs::Translate(viewMatrix, Gs::Vector3f(0, 0, 5));

        // Enter main loop
        while (mainWindow->ProcessEvents())
        {
            // Update scene transformation
            Gs::RotateFree(worldMatrix, Gs::Vector3f(0, 1, 0), Gs::Deg2Rad(0.005f));

            // Draw scene for OpenGL
            {
                // Set transformation matrix for OpenGL
                matrices.wvpMatrix = projMatrixGL * viewMatrix * worldMatrix;

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

            // Draw scene for Direct3D
            {
                // Set transformation matrix for Direct3D
                matrices.wvpMatrix = projMatrixD3D * viewMatrix * worldMatrix;

                // Clear color buffer
                commandsD3D->Clear(LLGL::ClearFlags::ColorDepth);

                // Update constant buffer
                rendererD3D->WriteBuffer(*constBufferD3D, &matrices, sizeof(matrices), 0);

                // Set graphics pipeline and vertex buffer
                commandsD3D->SetGraphicsPipeline(*pipelineD3D);
                commandsD3D->SetConstantBuffer(*constBufferD3D, 0);
                commandsD3D->SetVertexBuffer(*vertexBufferD3D);
                commandsD3D->SetIndexBuffer(*indexBufferD3D);
                commandsD3D->SetSampler(*samplerD3D, 0);
                commandsD3D->SetTexture(*textureD3D, 0);

                // Draw triangulated cube
                commandsD3D->DrawIndexed(36, 0);

                // Present the result on the screen
                contextD3D->Present();
            }
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

