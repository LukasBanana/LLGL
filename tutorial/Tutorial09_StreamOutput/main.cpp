/*
 * main.cpp (Tutorial09_StreamOutput)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


class Tutorial09 : public Tutorial
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

    Tutorial09() :
        Tutorial { L"LLGL Tutorial 09: StreamOutput" }
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
        vertexFormat.AppendAttribute({ "position", LLGL::VectorType::Float3 });

        // Create vertex, index, and constant buffers
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeTriangleIndices(), LLGL::DataType::UInt32);
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
            soBufferDesc.type   = LLGL::BufferType::StreamOutput;
            soBufferDesc.size   = sizeof(Gs::Vector4f) * 36 * 3;
            soBufferDesc.flags  = LLGL::BufferFlags::MapReadAccess;
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
                    { LLGL::ShaderType::Vertex, "shader.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Geometry, "shader.hlsl", "GS", "gs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat },
                streamOutputFormat
            );
        }
        else
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "vertex.glsl" },
                    { LLGL::ShaderType::Geometry, "geometry.glsl" },
                    { LLGL::ShaderType::Fragment, "fragment.glsl" }
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
        // Clear color and depth buffers
        commands->Clear(LLGL::ClearFlags::Color | LLGL::ClearFlags::Depth);

        // Update constant buffer
        static float rotation;
        rotation += 0.01f;
        settings.wvpMatrix = projection;
        Gs::Translate(settings.wvpMatrix, { 0, 0, 7 });
        Gs::Scale(settings.wvpMatrix, Gs::Vector3f(0.5f));
        Gs::RotateFree(settings.wvpMatrix, Gs::Vector3f(1).Normalized(), rotation);
        UpdateBuffer(constantBuffer, settings);

        // Set buffers
        commands->SetVertexBuffer(*vertexBuffer);
        commands->SetIndexBuffer(*indexBuffer);
        commands->SetConstantBuffer(*constantBuffer, 0, LLGL::ShaderStageFlags::VertexStage);
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
        renderer->GetCommandQueue()->WaitForFinish();

        if (auto outputBuffer = renderer->MapBuffer(*streamOutputBuffer, LLGL::BufferCPUAccess::ReadOnly))
        {
            std::vector<Gs::Vector4f> output(36*3);
            ::memcpy(output.data(), outputBuffer, sizeof(Gs::Vector4f)*36*3);
            renderer->UnmapBuffer(*streamOutputBuffer);

            // print or debug data in "output" container ...
        }

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial09);



