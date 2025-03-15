/*
 * Example.cpp (Example_IndirectDraw)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#define _USE_MATH_DEFINES
#include <cmath>

#include <ExampleBase.h>


class Example_IndirectDraw : public ExampleBase
{

    static const std::uint32_t maxNumSceneObjects = 64;

    LLGL::VertexFormat      vertexFormat[2];

    LLGL::Buffer*           perVertexDataBuf        = nullptr;
    LLGL::Buffer*           perInstanceDataBuf      = nullptr;
    LLGL::BufferArray*      vertexBufferArray       = nullptr;

    LLGL::Buffer*           inputBuffer             = nullptr;
    LLGL::Buffer*           indirectArgBuffer       = nullptr;

    LLGL::Shader*           computeShader           = nullptr;
    LLGL::PipelineLayout*   computeLayout           = nullptr;
    LLGL::PipelineState*    computePipeline         = nullptr;

    LLGL::Shader*           graphicsVertexShader    = nullptr;
    LLGL::Shader*           graphicsFragmentShader  = nullptr;
    LLGL::PipelineLayout*   graphicsLayout          = nullptr;
    LLGL::PipelineState*    graphicsPipeline        = nullptr;

    struct SceneState
    {
        float           time            = 0.0f;
        std::uint32_t   numSceneObjects = maxNumSceneObjects;
        float           aspectRatio     = 1.0f;
        float           _pad0[1];
    }
    sceneState;

    struct SceneObject
    {
        float rotation[2][2];
        float position[2];
        float _pad0[2];
    };

public:

    Example_IndirectDraw() :
        ExampleBase { "LLGL Example: Indirect Draw" }
    {
        // Check if samplers are supported
        const auto& renderCaps = renderer->GetRenderingCaps();

        LLGL_VERIFY(renderCaps.features.hasComputeShaders);

        // Create all graphics objects
        CreateBuffers();
        CreateComputePipeline();
        CreateGraphicsPipeline();
    }

    void CreateBuffers()
    {
        // Specify vertex format
        struct Vertex
        {
            float           coord[2];
            std::uint8_t    color[4];
        };

        vertexFormat[0].attributes =
        {
            LLGL::VertexAttribute{ "coord", LLGL::Format::RG32Float,  /*location:*/ 0, /*offset:*/ 0, /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },
            LLGL::VertexAttribute{ "color", LLGL::Format::RGBA8UNorm, /*location:*/ 1, /*offset:*/ 8, /*stride:*/ sizeof(Vertex), /*slot:*/ 0 },
        };

        vertexFormat[1].attributes =
        {
            LLGL::VertexAttribute{ "rotation", /*semanticIndex:*/ 0, LLGL::Format::RG32Float, /*location:*/ 2, /*offset:*/  0, /*stride:*/ sizeof(SceneObject), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "rotation", /*semanticIndex:*/ 1, LLGL::Format::RG32Float, /*location:*/ 3, /*offset:*/  8, /*stride:*/ sizeof(SceneObject), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
            LLGL::VertexAttribute{ "position",                       LLGL::Format::RG32Float, /*location:*/ 4, /*offset:*/ 16, /*stride:*/ sizeof(SceneObject), /*slot:*/ 1, /*instanceDivisor:*/ 1 },
        };

        // Define vertex buffer data
        auto CircleX = [](float a) { return std::sin(a*Gs::pi/180.0f); };
        auto CircleY = [](float a) { return std::cos(a*Gs::pi/180.0f); };

        Vertex vertices[] =
        {
            // Triangle
            { { CircleX(  0.0f), CircleY(  0.0f) }, { 255,   0,   0, 255 } },
            { { CircleX(120.0f), CircleY(120.0f) }, {   0, 255,   0, 255 } },
            { { CircleX(240.0f), CircleY(240.0f) }, {   0,   0, 255, 255 } },

            // Quad
            { { -1.0f, +1.0f }, {   0, 255,   0, 255 } },
            { { -1.0f, -1.0f }, { 255,   0,   0, 255 } },
            { { +1.0f, +1.0f }, {   0,   0, 255, 255 } },
            { { +1.0f, -1.0f }, { 255,   0, 255, 255 } },
        };

        // Create vertex buffer
        LLGL::BufferDescriptor perVertexDataDesc;
        {
            perVertexDataDesc.debugName     = "Vertices";
            perVertexDataDesc.size          = sizeof(vertices);
            perVertexDataDesc.bindFlags     = LLGL::BindFlags::VertexBuffer;
            perVertexDataDesc.vertexAttribs = vertexFormat[0].attributes;
        }
        perVertexDataBuf = renderer->CreateBuffer(perVertexDataDesc, vertices);

        // Create instance buffer
        LLGL::BufferDescriptor perInstanceDataDesc;
        {
            perInstanceDataDesc.debugName       = "Instances";
            perInstanceDataDesc.size            = sizeof(SceneObject) * maxNumSceneObjects;
            perInstanceDataDesc.bindFlags       = LLGL::BindFlags::VertexBuffer | LLGL::BindFlags::Storage;
            perInstanceDataDesc.vertexAttribs   = vertexFormat[1].attributes;
            perInstanceDataDesc.format          = LLGL::Format::RGBA32Float;
        }
        perInstanceDataBuf = renderer->CreateBuffer(perInstanceDataDesc);

        // Create vertex array buffer
        LLGL::Buffer* buffers[2] = { perVertexDataBuf, perInstanceDataBuf };
        vertexBufferArray = renderer->CreateBufferArray(2, buffers);

        // Create scene state buffer
        LLGL::BufferDescriptor inBufferDesc;
        {
            inBufferDesc.debugName  = "Input";
            inBufferDesc.size       = sizeof(SceneState);
            inBufferDesc.bindFlags  = LLGL::BindFlags::ConstantBuffer;
        }
        inputBuffer = renderer->CreateBuffer(inBufferDesc, &sceneState);

        // Create indirect argument buffer
        LLGL::BufferDescriptor argBufferDesc;
        {
            argBufferDesc.debugName = "IndirectArguments";
            argBufferDesc.size      = sizeof(LLGL::DrawIndirectArguments) * 2;
            argBufferDesc.bindFlags = LLGL::BindFlags::IndirectBuffer | LLGL::BindFlags::Storage;
            argBufferDesc.format    = LLGL::Format::RGBA32UInt;
        }
        indirectArgBuffer = renderer->CreateBuffer(argBufferDesc);
    }

    void CreateComputePipeline()
    {
        // Create compute shader
        if (Supported(LLGL::ShadingLanguage::GLSL))
            computeShader = LoadShader({ LLGL::ShaderType::Compute, "Example.comp" });
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
            computeShader = LoadShader({ LLGL::ShaderType::Compute, "Example.comp.spv" });
        else if (Supported(LLGL::ShadingLanguage::HLSL))
            computeShader = LoadShader({ LLGL::ShaderType::Compute, "Example.hlsl", "CS", "cs_5_0" });
        else if (Supported(LLGL::ShadingLanguage::Metal))
            computeShader = LoadShader({ LLGL::ShaderType::Compute, "Example.metal", "CS", "1.1" });
        else
            LLGL_THROW_RUNTIME_ERROR("shaders not available for selected renderer in this example");

        // Create compute pipeline layout
        computeLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneState@2):comp,"
                "rwbuffer(sceneObjects@3):comp,"
                "rwbuffer(drawArgs@4):comp,"
            )
        );

        // Create compute pipeline
        LLGL::ComputePipelineDescriptor pipelineDesc;
        {
            pipelineDesc.debugName      = "ComputePSO";
            pipelineDesc.pipelineLayout = computeLayout;
            pipelineDesc.computeShader  = computeShader;
        }
        computePipeline = renderer->CreatePipelineState(pipelineDesc);

        // Report PSO compilation errors
        if (const LLGL::Report* report = computePipeline->GetReport())
        {
            if (report->HasErrors())
                LLGL::Log::Errorf(report->GetText());
        }
    }

    void CreateGraphicsPipeline()
    {
        // Create graphics shader
        if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            graphicsVertexShader    = LoadShader({ LLGL::ShaderType::Vertex,   "Example.vert" }, { vertexFormat[0], vertexFormat[1] });
            graphicsFragmentShader  = LoadShader({ LLGL::ShaderType::Fragment, "Example.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            graphicsVertexShader    = LoadShader({ LLGL::ShaderType::Vertex,   "Example.vert.spv" }, { vertexFormat[0], vertexFormat[1] });
            graphicsFragmentShader  = LoadShader({ LLGL::ShaderType::Fragment, "Example.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            graphicsVertexShader    = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" }, { vertexFormat[0], vertexFormat[1] });
            graphicsFragmentShader  = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            graphicsVertexShader    = LoadShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VS", "1.1" }, { vertexFormat[0], vertexFormat[1] });
            graphicsFragmentShader  = LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", "PS", "1.1" });
        }
        else
            LLGL_THROW_RUNTIME_ERROR("shaders not available for selected renderer in this example");

        // Create compute pipeline layout
        graphicsLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneState@2):vert,"
            )
        );

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.debugName                      = "GraphicsPSO";
            pipelineDesc.pipelineLayout                 = graphicsLayout;
            pipelineDesc.vertexShader                   = graphicsVertexShader;
            pipelineDesc.fragmentShader                 = graphicsFragmentShader;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        graphicsPipeline = renderer->CreatePipelineState(pipelineDesc);

        // Report PSO compilation errors
        if (const LLGL::Report* report = graphicsPipeline->GetReport())
        {
            if (report->HasErrors())
                LLGL::Log::Errorf(report->GetText());
        }
    }

private:

    void OnDrawFrame() override
    {
        timer.MeasureTime();

        // Record and submit compute commands
        commands->Begin();
        {
            // Update timer
            sceneState.time += static_cast<float>(timer.GetDeltaTime());
            sceneState.aspectRatio = 1.0f / GetAspectRatio();
            commands->UpdateBuffer(*inputBuffer, 0, &sceneState, sizeof(sceneState));

            // Run compute shader
            commands->SetPipelineState(*computePipeline);
            commands->SetResource(0, *inputBuffer);
            commands->SetResource(1, *perInstanceDataBuf);
            commands->SetResource(2, *indirectArgBuffer);
            commands->Dispatch(sceneState.numSceneObjects, 1, 1);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Record and submit graphics commands
        commands->Begin();
        {
            // Draw scene
            commands->BeginRenderPass(*swapChain);
            {
                // Clear color buffer and set viewport
                commands->Clear(LLGL::ClearFlags::Color, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                // Set vertex buffer
                commands->SetVertexBufferArray(*vertexBufferArray);

                // Draw scene with indirect argument buffer
                commands->SetPipelineState(*graphicsPipeline);
                commands->SetResource(0, *inputBuffer);
                commands->DrawIndirect(*indirectArgBuffer, 0, 2, sizeof(LLGL::DrawIndirectArguments));
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_IndirectDraw);



