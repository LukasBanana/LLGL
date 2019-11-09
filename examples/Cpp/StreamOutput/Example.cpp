/*
 * Example.cpp (Example_StreamOutput)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <iomanip>


class Example_StreamOutput : public ExampleBase
{

    LLGL::ShaderProgram*    shaderProgram       = nullptr;

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
        ExampleBase { L"LLGL Example: StreamOutput" }
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
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.450core.vert.spv" },
                    { LLGL::ShaderType::Geometry, "Example.450core.geom.spv" },
                    { LLGL::ShaderType::Fragment, "Example.450core.frag.spv" }
                },
                { vertexFormat },
                streamOutputFormat
            );
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
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
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");
    }

    void CreatePipelines()
    {
        // Create graphics pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(Settings@2):vert")
        );

        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

    void CreateResourceHeaps()
    {
        LLGL::ResourceHeapDescriptor heapDesc;
        {
            heapDesc.pipelineLayout = pipelineLayout;
            heapDesc.resourceViews  = { constantBuffer };
        }
        resourceHeap = renderer->CreateResourceHeap(heapDesc);
    }

private:

    void PrintOutputVector(std::size_t index, const Gs::Vector4f* outputVectors)
    {
        const auto& v = outputVectors[index];
        std::cout << std::fixed << std::setfill('0') << std::setprecision(2);
        std::cout << "SV_Position[" << index << "] = " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "        \r";
    }

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

        // Start command recording
        commands->Begin();
        {
            // Update constant buffer
            commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

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
            std::flush(std::cout);

            // Unmap buffer
            renderer->UnmapBuffer(*streamOutputBuffer);
        }

        // Read stream-output buffer
        //commandQueue->WaitIdle();

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_StreamOutput);



