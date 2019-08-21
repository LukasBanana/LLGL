/*
 * Example.cpp (Example_ComputeShader)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

    LLGL::ShaderProgram*    computeShader           = nullptr;
    LLGL::PipelineLayout*   computeLayout           = nullptr;
    LLGL::ComputePipeline*  computePipeline         = nullptr;
    LLGL::ResourceHeap*     computeResourceHeap     = nullptr;

    LLGL::ShaderProgram*    graphicsShader          = nullptr;
    LLGL::GraphicsPipeline* graphicsPipeline        = nullptr;

    //LLGL::Fence*            fence                   = nullptr;

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
        graphicsShader->SetName("Graphics.Shader");
        graphicsPipeline->SetName("Graphics.Pipeline");
    }

    void CreateBuffers()
    {
        // Specify vertex format
        vertexFormat[0].AppendAttribute({ "coord", LLGL::Format::RG32Float  });
        vertexFormat[0].AppendAttribute({ "color", LLGL::Format::RGBA8UNorm });

        vertexFormat[1].AppendAttribute({ "rotation", 0, LLGL::Format::RG32Float, 1 });
        vertexFormat[1].AppendAttribute({ "rotation", 1, LLGL::Format::RG32Float, 1 });
        vertexFormat[1].AppendAttribute({ "position",    LLGL::Format::RG32Float, 1 });
        vertexFormat[1].stride      = sizeof(SceneObject);
        vertexFormat[1].inputSlot   = 1;

        // Define vertex buffer data
        struct Vertex
        {
            float           x, y;
            std::uint8_t    r, g, b, a;
        };

        auto CircleX = [](float a) { return std::sin(a*Gs::pi/180.0f); };
        auto CircleY = [](float a) { return std::cos(a*Gs::pi/180.0f); };

        Vertex vertices[] =
        {
            // Triangle
            { CircleX(  0.0f), CircleY(  0.0f), 255,   0,   0, 255 },
            { CircleX(120.0f), CircleY(120.0f),   0, 255,   0, 255 },
            { CircleX(240.0f), CircleY(240.0f),   0,   0, 255, 255 },

            // Quad
            { -1.0f, +1.0f,   0, 255,   0, 255 },
            { -1.0f, -1.0f, 255,   0,   0, 255 },
            { +1.0f, +1.0f,   0,   0, 255, 255 },
            { +1.0f, -1.0f, 255,   0, 255, 255 },
        };

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size                   = sizeof(vertices);
            vertexBufferDesc.bindFlags              = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexBuffer.format    = vertexFormat[0];
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices);

        // Create instance buffer
        LLGL::BufferDescriptor instanceBufferDesc;
        {
            instanceBufferDesc.size                         = sizeof(SceneObject)*maxNumSceneObjects;
            instanceBufferDesc.bindFlags                    = LLGL::BindFlags::VertexBuffer | LLGL::BindFlags::Storage;
            instanceBufferDesc.vertexBuffer.format          = vertexFormat[1];
            instanceBufferDesc.storageBuffer.storageType    = LLGL::StorageBufferType::RWBuffer;
            instanceBufferDesc.storageBuffer.format         = LLGL::Format::R32Float;
            instanceBufferDesc.storageBuffer.stride         = sizeof(float);
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
            argBufferDesc.size                      = sizeof(LLGL::DrawIndirectArguments)*2;
            argBufferDesc.bindFlags                 = LLGL::BindFlags::IndirectBuffer | LLGL::BindFlags::Storage;
            argBufferDesc.storageBuffer.storageType = LLGL::StorageBufferType::RWBuffer;
            argBufferDesc.storageBuffer.format      = LLGL::Format::R32UInt;
            argBufferDesc.storageBuffer.stride      = sizeof(std::uint32_t);
        }
        indirectArgBuffer = renderer->CreateBuffer(argBufferDesc);

        // Create fence
        //fence = renderer->CreateFence();
    }

    void CreateComputePipeline()
    {
        // Create compute shader
        if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            computeShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Compute, "Example.comp" }
                }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            computeShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Compute, "Example.comp.spv" }
                }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            computeShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Compute, "Example.hlsl", "CS", "cs_5_0" },
                }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            computeShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Compute, "Example.metal", "CS", "1.1" },
                }
            );
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create compute pipeline layout
        computeLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(2):comp, rwbuffer(3):comp, rwbuffer(4):comp")
        );

        // Create compute pipeline
        LLGL::ComputePipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram  = computeShader;
            pipelineDesc.pipelineLayout = computeLayout;
        }
        computePipeline = renderer->CreateComputePipeline(pipelineDesc);

        // Create resource heap for compute pipeline
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = computeLayout;
            resourceHeapDesc.resourceViews  = { inputBuffer, instanceBuffer, indirectArgBuffer };
        }
        computeResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void CreateGraphicsPipeline()
    {
        // Create graphics shader
        if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.vert" },
                    { LLGL::ShaderType::Fragment, "Example.frag" }
                },
                { vertexFormat[0], vertexFormat[1] }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.vert.spv" },
                    { LLGL::ShaderType::Fragment, "Example.frag.spv" }
                },
                { vertexFormat[0], vertexFormat[1] }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat[0], vertexFormat[1] }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.metal", "VS", "1.1" },
                    { LLGL::ShaderType::Fragment, "Example.metal", "PS", "1.1" }
                },
                { vertexFormat[0], vertexFormat[1] }
            );
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = graphicsShader;
            pipelineDesc.primitiveTopology          = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.rasterizer.multiSampling   = GetMultiSampleDesc();
        }
        graphicsPipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

private:

    void OnDrawFrame() override
    {
        timer->MeasureTime();

        // Set render target
        commands->Begin();
        {
            // Update timer
            sceneState.time += static_cast<float>(timer->GetDeltaTime());
            commands->UpdateBuffer(*inputBuffer, 0, &sceneState, sizeof(sceneState));

            // Run compute shader
            commands->SetComputePipeline(*computePipeline);
            commands->SetComputeResourceHeap(*computeResourceHeap);
            commands->Dispatch(sceneState.numSceneObjects, 1, 1);

            if (commandsExt)
                commandsExt->ResetResourceSlots(LLGL::ResourceType::Buffer, 3, 1, LLGL::BindFlags::Storage, LLGL::StageFlags::ComputeStage);

            // Draw scene
            commands->BeginRenderPass(*context);
            {
                // Clear color buffer and set viewport
                commands->Clear(LLGL::ClearFlags::Color);
                commands->SetViewport(context->GetVideoMode().resolution);

                // Set vertex buffer
                commands->SetVertexBufferArray(*vertexBufferArray);

                // Draw scene with indirect argument buffer
                commands->SetGraphicsPipeline(*graphicsPipeline);
                commands->DrawIndirect(*indirectArgBuffer, 0, 2, sizeof(LLGL::DrawIndirectArguments));

                if (commandsExt)
                    commandsExt->ResetResourceSlots(LLGL::ResourceType::Buffer, 3, 1, LLGL::BindFlags::VertexBuffer, LLGL::StageFlags::VertexStage);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_ComputeShader);



