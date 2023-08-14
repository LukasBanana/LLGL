/*
 * Example.cpp (Example_ComputeShader)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#define _USE_MATH_DEFINES
#include <cmath>

#include <ExampleBase.h>


class Example_ComputeShader : public ExampleBase
{

    static const std::uint32_t maxNumSceneObjects = 64;

    LLGL::VertexFormat      vertexFormat[2];

    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           instanceBuffer          = nullptr;
    LLGL::BufferArray*      vertexBufferArray       = nullptr;

    LLGL::Buffer*           inputBuffer             = nullptr;
    LLGL::Buffer*           indirectArgBuffer       = nullptr;

    LLGL::Shader*           computeShader           = nullptr;
    LLGL::PipelineLayout*   computeLayout           = nullptr;
    LLGL::PipelineState*    computePipeline         = nullptr;
    LLGL::ResourceHeap*     computeResourceHeap     = nullptr;

    LLGL::Shader*           graphicsVertexShader    = nullptr;
    LLGL::Shader*           graphicsFragmentShader  = nullptr;
    LLGL::PipelineState*    graphicsPipeline        = nullptr;

    struct SceneState
    {
        float           time            = 0.0f;
        std::uint32_t   numSceneObjects = maxNumSceneObjects;
        float           _pad0[2];
    }
    sceneState;

    struct SceneObject
    {
        Gs::Matrix2f    rotation;
        Gs::Vector2f    position;
        float           _pad0[2];
    };

public:

    Example_ComputeShader() :
        ExampleBase { L"LLGL Example: Compute Shader", { 800, 800 } }
    {
        // Check if samplers are supported
        const auto& renderCaps = renderer->GetRenderingCaps();

        if (!renderCaps.features.hasComputeShaders)
            throw std::runtime_error("compute shaders are not supported by this renderer");

        // Create all graphics objects
        CreateBuffers();
        CreateComputePipeline();
        CreateGraphicsPipeline();

        // Add debugging names
        commands->SetName("Commands");
        vertexBuffer->SetName("Vertices");
        instanceBuffer->SetName("Instances");
        inputBuffer->SetName("Input");
        indirectArgBuffer->SetName("IndirectArguments");
        computeShader->SetName("Compute.Shader");
        computeLayout->SetName("Compute.Layout");
        computePipeline->SetName("Compute.Pipeline");
        computeResourceHeap->SetName("Compute.ResourceHeap");
        graphicsVertexShader->SetName("Graphics.VertexShader");
        graphicsFragmentShader->SetName("Graphics.FragmentShader");
        graphicsPipeline->SetName("Graphics.Pipeline");
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
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = sizeof(vertices);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat[0].attributes;
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create instance buffer
        LLGL::BufferDescriptor instanceBufferDesc;
        {
            instanceBufferDesc.size             = sizeof(SceneObject) * maxNumSceneObjects;
            instanceBufferDesc.bindFlags        = LLGL::BindFlags::VertexBuffer | LLGL::BindFlags::Storage;
            instanceBufferDesc.vertexAttribs    = vertexFormat[1].attributes;
            instanceBufferDesc.format           = LLGL::Format::RGBA32Float;
        }
        instanceBuffer = renderer->CreateBuffer(instanceBufferDesc);

        // Create vertex array buffer
        LLGL::Buffer* buffers[] = { vertexBuffer, instanceBuffer };
        vertexBufferArray = renderer->CreateBufferArray(2, buffers);

        // Create scene state buffer
        LLGL::BufferDescriptor inBufferDesc;
        {
            inBufferDesc.size       = sizeof(SceneState);
            inBufferDesc.bindFlags  = LLGL::BindFlags::ConstantBuffer;
        }
        inputBuffer = renderer->CreateBuffer(inBufferDesc, &sceneState);

        // Create indirect argument buffer
        LLGL::BufferDescriptor argBufferDesc;
        {
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
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create compute pipeline layout
        computeLayout = renderer->CreatePipelineLayout(
            LLGL::Parse("heap{cbuffer(2):comp, rwbuffer(3):comp, rwbuffer(4):comp}")
        );

        // Create compute pipeline
        LLGL::ComputePipelineDescriptor pipelineDesc;
        {
            pipelineDesc.computeShader  = computeShader;
            pipelineDesc.pipelineLayout = computeLayout;
        }
        computePipeline = renderer->CreatePipelineState(pipelineDesc);

        // Create resource heap for compute pipeline
        computeResourceHeap = renderer->CreateResourceHeap(computeLayout, { inputBuffer, instanceBuffer, indirectArgBuffer });
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
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = graphicsVertexShader;
            pipelineDesc.fragmentShader                 = graphicsFragmentShader;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        graphicsPipeline = renderer->CreatePipelineState(pipelineDesc);
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
            commands->UpdateBuffer(*inputBuffer, 0, &sceneState, sizeof(sceneState));

            // Run compute shader
            commands->SetPipelineState(*computePipeline);
            commands->SetResourceHeap(*computeResourceHeap);
            commands->Dispatch(sceneState.numSceneObjects, 1, 1);

            commands->ResetResourceSlots(LLGL::ResourceType::Buffer, 3, 1, LLGL::BindFlags::Storage, LLGL::StageFlags::ComputeStage);
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
                commands->DrawIndirect(*indirectArgBuffer, 0, 2, sizeof(LLGL::DrawIndirectArguments));

                commands->ResetResourceSlots(LLGL::ResourceType::Buffer, 1, 1, LLGL::BindFlags::VertexBuffer, LLGL::StageFlags::VertexStage);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        swapChain->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_ComputeShader);



