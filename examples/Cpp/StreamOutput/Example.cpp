/*
 * Example.cpp (Example_StreamOutput)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


class Example_StreamOutput : public ExampleBase
{

    LLGL::ShaderProgram*    shaderProgram       = nullptr;

    LLGL::GraphicsPipeline* pipeline            = nullptr;

    LLGL::Buffer*           vertexBuffer        = nullptr;
    LLGL::Buffer*           indexBuffer         = nullptr;
    LLGL::Buffer*           constantBuffer      = nullptr;
    LLGL::Buffer*           streamOutputBuffer  = nullptr;

    struct Settings
    {
        Gs::Matrix4f wvpMatrix;
    }
    settings;

public:

    Example_StreamOutput() :
        ExampleBase { L"LLGL Example: StreamOutput" }
    {
        // Create all graphics objects
        LLGL::VertexFormat vertexFormat;
        LLGL::StreamOutputFormat streamOutputFormat;

        CreateBuffers(vertexFormat, streamOutputFormat);
        CreateShaders(vertexFormat, streamOutputFormat);
        CreatePipelines();
    }

    void CreateBuffers(LLGL::VertexFormat& vertexFormat, LLGL::StreamOutputFormat& streamOutputFormat)
    {
        // Specify vertex format
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

        // Create vertex, index, and constant buffers
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeTriangleIndices(), LLGL::Format::R32UInt);
        constantBuffer = CreateConstantBuffer(settings);

        // Specify stream-output format
        LLGL::StreamOutputAttribute soAttrib;
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

    void CreateShaders(const LLGL::VertexFormat& vertexFormat, const LLGL::StreamOutputFormat& streamOutputFormat)
    {
        // Load shader program
        const auto& languages = renderer->GetRenderingCaps().shadingLanguages;
        if (std::find(languages.begin(), languages.end(), LLGL::ShadingLanguage::HLSL) != languages.end())
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Geometry, "Example.hlsl", "GS", "gs_5_0" },
                    { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat },
                streamOutputFormat
            );
        }
        else
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.vert" },
                    { LLGL::ShaderType::Geometry, "Example.geom" },
                    { LLGL::ShaderType::Fragment, "Example.frag" }
                },
                { vertexFormat },
                streamOutputFormat
            );
        }
    }

    void CreatePipelines()
    {
        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

private:

    void OnDrawFrame() override
    {
        timer->MeasureTime();

        // Update constant buffer
        static float rotation;
        rotation += static_cast<float>(timer->GetDeltaTime()) * 0.5f;

        settings.wvpMatrix = projection;
        Gs::Translate(settings.wvpMatrix, { 0, 0, 7 });
        Gs::Scale(settings.wvpMatrix, Gs::Vector3f(0.5f));
        Gs::RotateFree(settings.wvpMatrix, Gs::Vector3f(1).Normalized(), rotation);
        UpdateBuffer(constantBuffer, settings);

        // Start command recording
        commands->Begin();
        {
            // Set vertex and index buffers
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetIndexBuffer(*indexBuffer);

            // Begin render pass for context
            commands->BeginRenderPass(*context);
            {
                // Clear color and depth buffers
                commands->Clear(LLGL::ClearFlags::ColorDepth);

                // Set viewport to context resolution
                commands->SetViewport(context->GetResolution());

                // Set buffers
                commands->SetResource(*constantBuffer, 0, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage);
                commands->SetStreamOutputBuffer(*streamOutputBuffer);

                // Set graphics pipeline state
                commands->SetGraphicsPipeline(*pipeline);

                // Draw scene
                commands->BeginStreamOutput(LLGL::PrimitiveType::Triangles);
                {
                    commands->DrawIndexed(36, 0);
                }
                commands->EndStreamOutput();

                // Read stream-output buffer
                renderer->GetCommandQueue()->WaitIdle();

                if (auto outputBuffer = renderer->MapBuffer(*streamOutputBuffer, LLGL::CPUAccess::ReadOnly))
                {
                    std::vector<Gs::Vector4f> output(36*3);
                    ::memcpy(output.data(), outputBuffer, sizeof(Gs::Vector4f)*36*3);
                    renderer->UnmapBuffer(*streamOutputBuffer);

                    // print or debug data in "output" container ...
                }
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_StreamOutput);



