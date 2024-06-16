/*
 * Example.cpp (Example_StreamOutput)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <stdio.h>


class Example_StreamOutput : public ExampleBase
{

    LLGL::Shader*           vsScene             = nullptr;
    LLGL::Shader*           gsScene             = nullptr;
    LLGL::Shader*           fsScene             = nullptr;

    LLGL::PipelineLayout*   pipelineLayout      = nullptr;
    LLGL::PipelineState*    pipeline            = nullptr;

    LLGL::Buffer*           vertexBuffer        = nullptr;
    LLGL::Buffer*           indexBuffer         = nullptr;
    LLGL::Buffer*           constantBuffer      = nullptr;
    LLGL::Buffer*           streamOutputBuffer  = nullptr;

    LLGL::ResourceHeap*     resourceHeap        = nullptr;

    struct Settings
    {
        Gs::Matrix4f wvpMatrix;
    }
    settings;

public:

    Example_StreamOutput() :
        ExampleBase { "LLGL Example: StreamOutput" }
    {
        // Create all graphics objects
        LLGL::VertexFormat vertexFormat;
        LLGL::VertexFormat streamOutputFormat;

        CreateBuffers(vertexFormat, streamOutputFormat);
        CreateShaders(vertexFormat, streamOutputFormat);
        CreatePipelines();
        CreateResourceHeaps();
    }

    void CreateBuffers(LLGL::VertexFormat& vertexFormat, LLGL::VertexFormat& streamOutputFormat)
    {
        // Specify vertex format
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

        // Create vertex, index, and constant buffers
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeTriangleIndices(), LLGL::Format::R32UInt);
        constantBuffer = CreateConstantBuffer(settings);

        // Specify stream-output format
        LLGL::VertexAttribute soAttrib;
        {
            const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
            if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
                soAttrib.name = "SV_Position";
            else
                soAttrib.name = "gl_Position";
        }
        streamOutputFormat.AppendAttribute(soAttrib);

        // Create stream-output buffer
        LLGL::BufferDescriptor soBufferDesc;
        {
            soBufferDesc.size           = sizeof(Gs::Vector4f) * 36 * 3;
            soBufferDesc.bindFlags      = LLGL::BindFlags::StreamOutputBuffer;
            soBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Read;
        }
        streamOutputBuffer = renderer->CreateBuffer(soBufferDesc);
    }

    void CreateShaders(const LLGL::VertexFormat& vertexFormat, const LLGL::VertexFormat& streamOutputFormat)
    {
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" }, { vertexFormat });
            gsScene = LoadShader({ LLGL::ShaderType::Geometry, "Example.hlsl", "GS", "gs_5_0" }, {}, streamOutputFormat);
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Example.vert" }, { vertexFormat });
            gsScene = LoadShader({ LLGL::ShaderType::Geometry, "Example.geom" }, {}, streamOutputFormat);
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Example.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            // Note: OpenGL might support SPIR-V but transform-feedback doesn't work properly with the GL_NV_transform_feedback extension with SPIR-V shaders.
            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Example.450core.vert.spv" }, { vertexFormat });
            gsScene = LoadShader({ LLGL::ShaderType::Geometry, "Example.450core.geom.spv" }, {}, streamOutputFormat);
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Example.450core.frag.spv" });
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");
    }

    void CreatePipelines()
    {
        // Create graphics pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(LLGL::Parse("heap{ cbuffer(Settings@2):vert }"));

        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = vsScene;
            pipelineDesc.geometryShader                 = gsScene;
            pipelineDesc.fragmentShader                 = fsScene;
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

    void CreateResourceHeaps()
    {
        resourceHeap = renderer->CreateResourceHeap(pipelineLayout, { constantBuffer });
    }

private:

    void PrintOutputVector(std::size_t index, const Gs::Vector4f* outputVectors)
    {
        const auto& v = outputVectors[index];
        ::printf(
            "SV_Position[%d] = %.02f, %.02f, %.02f, %.02f        \r",
            static_cast<int>(index), v.x, v.y, v.z, v.w
        );
    }

    void OnDrawFrame() override
    {
        timer.MeasureTime();

        // Update constant buffer
        static float rotation;
        rotation += static_cast<float>(timer.GetDeltaTime()) * 0.5f;

        settings.wvpMatrix = projection;
        Gs::Translate(settings.wvpMatrix, { 0, 0, 7 });
        Gs::Scale(settings.wvpMatrix, Gs::Vector3f(0.5f));
        Gs::RotateFree(settings.wvpMatrix, Gs::Vector3f(1).Normalized(), rotation);

        // Start command recording
        commands->Begin();
        {
            // Update constant buffer
            commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

            // Set vertex and index buffers
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetIndexBuffer(*indexBuffer);

            // Begin render pass for swap-chain
            commands->BeginRenderPass(*swapChain);
            {
                // Clear color and depth buffers
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);

                // Set viewport to swap-chain resolution
                commands->SetViewport(swapChain->GetResolution());

                // Set graphics pipeline state
                commands->SetPipelineState(*pipeline);

                // Set buffers
                commands->SetResourceHeap(*resourceHeap);

                // Draw scene with stream output
                commands->BeginStreamOutput(1, &streamOutputBuffer);
                {
                    commands->DrawIndexed(36, 0);
                }
                commands->EndStreamOutput();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Read stream-output buffer
        //commandQueue->WaitIdle();

        if (auto outputBuffer = renderer->MapBuffer(*streamOutputBuffer, LLGL::CPUAccess::ReadOnly))
        {
            // Print output data
            PrintOutputVector(1, reinterpret_cast<const Gs::Vector4f*>(outputBuffer));
            ::fflush(stdout);

            // Unmap buffer
            renderer->UnmapBuffer(*streamOutputBuffer);
        }

        // Read stream-output buffer
        //commandQueue->WaitIdle();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_StreamOutput);



