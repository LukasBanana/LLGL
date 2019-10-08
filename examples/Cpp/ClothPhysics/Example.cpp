/*
 * Example.cpp (Example_ClothPhysics)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#define _USE_MATH_DEFINES
#include <cmath>

#include <ExampleBase.h>


class Example_ClothPhysics : public ExampleBase
{

    const std::uint32_t     clothSegmentsU          = 16;
    const std::uint32_t     clothSegmentsV          = 16;
    const float             clothParticleMass       = 1.0f;
    const float             dampingFactor           = 3.8f;
    const Gs::Vector3f      viewPos                 = { 0, -0.75f, -5 };

    LLGL::VertexFormat      vertexFormat;

    LLGL::Buffer*           constantBuffer          = nullptr;
    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           indexBuffer             = nullptr;

    LLGL::PipelineLayout*   computeLayout           = nullptr;
    LLGL::ResourceHeap*     computeResourceHeap     = nullptr;

    LLGL::ShaderProgram*    computeShaders[3]       = {};
    LLGL::PipelineState*    computePipelines[3]     = {};

    LLGL::ShaderProgram*    graphicsShader          = nullptr;
    LLGL::PipelineLayout*   graphicsLayout          = nullptr;
    LLGL::PipelineState*    graphicsPipeline        = nullptr;
    LLGL::ResourceHeap*     graphicsResourceHeap    = nullptr;

    std::uint32_t           numClothIndices         = 0;
    Gs::Vector2f            viewRotation;

    struct SceneState
    {
        Gs::Matrix4f    wvpMatrix;
        Gs::Matrix4f    wMatrix;
        Gs::Vector4f    gravity         = { 0.0f, -9.81f * 0.1f, 0.0f, 1.0f };
        std::uint32_t   gridSize[2];
        std::uint32_t   numIterations   = 4;
        std::uint32_t   pad0;
        float           damping;
        float           stiffness       = 1.0f;
        float           dt;
        float           pad1;
        Gs::Vector4f    lightVec        = { 0.0f, 0.0f, 1.0f, 0.0f };
    }
    sceneState;

    struct Vertex
    {
        Gs::Vector4f    origPos;
        Gs::Vector4f    prevPos;
        Gs::Vector4f    nextPos;
        Gs::Vector4f    velocity;
        Gs::Vector4f    pos;
        Gs::Vector4f    normal;
        Gs::Vector2f    texCoord;
        float           invMass;
        float           pad0;
    };

public:

    Example_ClothPhysics() :
        ExampleBase { L"LLGL Example: Cloth Physics" }
    {
        // Check if samplers are supported
        const auto& renderCaps = renderer->GetRenderingCaps();

        if (!renderCaps.features.hasComputeShaders)
            throw std::runtime_error("compute shaders are not supported by this renderer");

        // Create all graphics objects
        CreateBuffers();
        CreateComputePipeline();
        CreateGraphicsPipeline();
    }

    void GenerateClothGeometry(std::vector<Vertex>& vertices, std::vector<std::uint32_t>& indices)
    {
        const auto invSegsU = 1.0f / static_cast<float>(clothSegmentsU);
        const auto invSegsV = 1.0f / static_cast<float>(clothSegmentsV);

        // Generate vertices from top to bottom, left to right
        vertices.resize((clothSegmentsU + 1)*(clothSegmentsV + 1));

        for (std::uint32_t v = 0; v <= clothSegmentsV; ++v)
        {
            for (std::uint32_t u = 0; u <= clothSegmentsU; ++u)
            {
                // Set mass for left and righ top particles to infinity to create suspension points
                bool isSuspensionPoint = (v == 0 && (u == 0 || u == clothSegmentsU));// || (v == clothSegmentsV && u == 0);

                auto& vert = vertices[v * (clothSegmentsU + 1) + u];
                {
                    // Initialize vertex attributes to generate 2D grid
                    vert.texCoord.x = static_cast<float>(u) * invSegsU;
                    vert.texCoord.y = static_cast<float>(v) * invSegsV;
                    vert.origPos.x  = vert.texCoord.x * 2.0f - 1.0f;
                    vert.origPos.y  = 0.0f;
                    vert.origPos.z  = vert.texCoord.y * -2.0f;
                    vert.prevPos    = vert.origPos;
                    vert.pos        = vert.origPos;

                    // Store mass as inverse value to simplify physics integration (zero means infinite mass)
                    vert.invMass    = (isSuspensionPoint ? 0.0f : 1.0f / clothParticleMass);
                }
            }
        }

        // Generate indices for triangle strips
        for (std::uint32_t v = 0; v < clothSegmentsV; ++v)
        {
            // Generate indices for row of triangle strip
            for (std::uint32_t u = 0; u <= clothSegmentsU; ++u)
            {
                indices.push_back((v + 1)*(clothSegmentsU + 1) + u);
                indices.push_back((v    )*(clothSegmentsU + 1) + u);
            }

            // Append special index value to restart the triangle strip
            if (v + 1 != clothSegmentsV)
                indices.push_back(0xFFFFFFFF);
        }

        // Store segments count for compute shader input
        sceneState.gridSize[0] = (clothSegmentsU + 1);
        sceneState.gridSize[1] = (clothSegmentsV + 1);
    }

    void CreateBuffers()
    {
        // Initialize vertex format for rendering (not all vertex attributes are required for rendering)
        vertexFormat.AppendAttribute({ "pos",      LLGL::Format::RGBA32Float }, false, sizeof(Gs::Vector4f)*4);
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGBA32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float   });
        vertexFormat.SetStride(sizeof(Vertex));
        vertexFormat.SetSlot(0);

        // Generate vertex and index data
        std::vector<Vertex> vertices;
        std::vector<std::uint32_t> indices;
        GenerateClothGeometry(vertices, indices);

        // Create constant buffer
        constantBuffer = CreateConstantBuffer(sceneState);

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size                       = sizeof(Vertex) * vertices.size();
            vertexBufferDesc.bindFlags                  = LLGL::BindFlags::VertexBuffer | LLGL::BindFlags::Storage;
            vertexBufferDesc.vertexAttribs              = vertexFormat.attributes;
            vertexBufferDesc.storageBuffer.storageType  = LLGL::StorageBufferType::RWBuffer;
            vertexBufferDesc.storageBuffer.format       = LLGL::Format::RGBA32Float;
            //vertexBufferDesc.storageBuffer.stride       = sizeof(Gs::Vector4f);//sizeof(Vertex);
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices.data());

        // Create index buffer
        LLGL::BufferDescriptor indexBufferDesc;
        {
            indexBufferDesc.size        = sizeof(std::uint32_t) * indices.size();
            indexBufferDesc.bindFlags   = LLGL::BindFlags::IndexBuffer;
            indexBufferDesc.indexFormat = LLGL::Format::R32UInt;
        }
        indexBuffer = renderer->CreateBuffer(indexBufferDesc, indices.data());

        // Store number of indices for draw commands
        numClothIndices = static_cast<std::uint32_t>(indices.size());
    }

    void CreateComputePipeline()
    {
        // Create compute shader
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            computeShaders[0] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.hlsl", "CSForces", "cs_5_0" } }
            );
            computeShaders[1] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.hlsl", "CSStretchConstraints", "cs_5_0" } }
            );
            computeShaders[2] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.hlsl", "CSRelaxation", "cs_5_0" } }
            );
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create compute pipeline layout
        computeLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(1):comp, rwbuffer(2):comp")
        );

        // Create resource heaps for compute pipeline
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = computeLayout;
            resourceHeapDesc.resourceViews  = { constantBuffer, vertexBuffer };
        }
        computeResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);

        // Create compute pipeline
        for (int i = 0; i < 3; ++i)
        {
            LLGL::ComputePipelineDescriptor pipelineDesc;
            {
                pipelineDesc.pipelineLayout = computeLayout;
                pipelineDesc.shaderProgram  = computeShaders[i];
            }
            computePipelines[i] = renderer->CreatePipelineState(pipelineDesc);
        }
    }

    void CreateGraphicsPipeline()
    {
        // Create graphics shader
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat }
            );
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create graphics pipeline layout
        graphicsLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(1):vert:frag")
        );

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.pipelineLayout                 = graphicsLayout;
            pipelineDesc.shaderProgram                  = graphicsShader;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            //pipelineDesc.rasterizer.polygonMode         = LLGL::PolygonMode::Wireframe;
        }
        graphicsPipeline = renderer->CreatePipelineState(pipelineDesc);

        // Create resource heaps for graphics pipeline
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = graphicsLayout;
            resourceHeapDesc.resourceViews  = { constantBuffer };
        }
        graphicsResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

private:

    void UpdateScene()
    {
        // Update user input
        if (input->KeyPressed(LLGL::Key::LButton))
        {
            auto motion = input->GetMouseMotion();
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -90.0f, 90.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }

        // Update timer
        timer->MeasureTime();
        sceneState.dt       = std::max(0.0001f, static_cast<float>(timer->GetDeltaTime()));
        sceneState.damping  = (1.0f - std::pow(10.0f, -dampingFactor));

        // Update world matrix
        sceneState.wMatrix.LoadIdentity();

        // Update view matrix
        Gs::Matrix4f vMatrix;
        Gs::RotateFree(vMatrix, { 0, 1, 0 }, Gs::Deg2Rad(viewRotation.y));
        Gs::RotateFree(vMatrix, { 1, 0, 0 }, Gs::Deg2Rad(viewRotation.x));
        Gs::Translate(vMatrix, viewPos);
        vMatrix.MakeInverse();

        // Update world-view-projection matrix
        sceneState.wvpMatrix = projection * vMatrix * sceneState.wMatrix;
    }

    void OnDrawFrame() override
    {
        UpdateScene();

        // Record and submit compute commands
        commands->Begin();
        {
            // Update scene state constant buffer
            commands->UpdateBuffer(*constantBuffer, 0, &sceneState, sizeof(sceneState));

            // Run compute shader
            for (int i = 0; i < 3; ++i)
            {
                commands->SetPipelineState(*computePipelines[i]);
                commands->SetComputeResourceHeap(*computeResourceHeap);
                commands->Dispatch(clothSegmentsU + 1, clothSegmentsV + 1, 1);
            }

            commands->ResetResourceSlots(LLGL::ResourceType::Buffer, 2, 1, LLGL::BindFlags::Storage, LLGL::StageFlags::ComputeStage);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Record and submit graphics commands
        commands->Begin();
        {
            // Draw scene
            commands->BeginRenderPass(*context);
            {
                // Clear color buffer and set viewport
                commands->Clear(LLGL::ClearFlags::ColorDepth);
                commands->SetViewport(context->GetVideoMode().resolution);

                // Set vertex and index buffers
                commands->SetVertexBuffer(*vertexBuffer);
                commands->SetIndexBuffer(*indexBuffer);

                // Draw scene with indirect argument buffer
                commands->SetPipelineState(*graphicsPipeline);
                commands->SetGraphicsResourceHeap(*graphicsResourceHeap);
                commands->DrawIndexed(numClothIndices, 0);

                commands->ResetResourceSlots(LLGL::ResourceType::Buffer, 0, 1, LLGL::BindFlags::VertexBuffer, LLGL::StageFlags::VertexStage);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_ClothPhysics);



