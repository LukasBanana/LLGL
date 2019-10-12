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

    enum ComputeShader
    {
        CSForces = 0,
        CSStretchConstraints,
        CSRelaxation,
        NumComputeShaders
    };

    enum ParticleAttribute
    {
        AttribBase = 0, // Original position, UV coordinates, inverse mass: float4[2] per particle
        AttribCurrPos,  // Current position: float4 per particle
        AttribNextPos,  // Next position: float4 per particle
        AttribPrevPos,  // Previous position: float4 per particle
        AttribVelocity, // Velocity vector: float4 per particle
        AttribNormal,   // Surface normal: float4 per particle
        NumAttribs
    };

    const std::uint32_t     numSolverIterations                 = 8;    // Number of integration steps to resolve stretching constraints between particles, good values are in [1, 10]
    const std::uint32_t     clothSegmentsU                      = 16;   // Number of segments in horizontal direction for cloth geometry
    const std::uint32_t     clothSegmentsV                      = 16;   // Number of segments in vertical direction for cloth geometry
    const float             clothParticleMass                   = 1.0f;
    const Gs::Vector3f      gravityVector                       = { 0, -9.81f * 0.2f, 0 };
    const float             dampingFactor                       = 3.8f;
    float                   stiffnessFactor                     = 1.0f; // Should be in [0, 1]
    const Gs::Vector3f      viewPos                             = { 0, -0.75f, -5 };

    LLGL::VertexFormat      vertexFormat;

    LLGL::Buffer*           constantBuffer                      = nullptr;
    LLGL::Buffer*           particleBuffers[NumAttribs]         = {};
    LLGL::BufferArray*      vertexBufferArray                   = nullptr;
    LLGL::Buffer*           indexBuffer                         = nullptr;

    LLGL::Texture*          colorMap                            = nullptr;
    LLGL::Sampler*          linearSampler                       = nullptr;

    LLGL::PipelineLayout*   computeLayout                       = nullptr;
    LLGL::ResourceHeap*     computeResourceHeaps[2]             = {}; // Swap-buffer fashion

    LLGL::ShaderProgram*    computeShaders[NumComputeShaders]   = {};
    LLGL::PipelineState*    computePipelines[NumComputeShaders] = {};

    LLGL::ShaderProgram*    graphicsShader                      = nullptr;
    LLGL::PipelineLayout*   graphicsLayout                      = nullptr;
    LLGL::PipelineState*    graphicsPipeline                    = nullptr;
    LLGL::ResourceHeap*     graphicsResourceHeap                = nullptr;

    std::uint32_t           numClothVertices                    = 0;
    std::uint32_t           numClothIndices                     = 0;
    std::uint32_t           swapBufferIndex                     = 0; // Index to swap particle buffer heaps
    Gs::Vector2f            viewRotation;

    struct SceneState
    {
        Gs::Matrix4f    wvpMatrix;
        Gs::Matrix4f    wMatrix;
        Gs::Vector4f    gravity;
        std::uint32_t   gridSize[2];
        std::uint32_t   pad0[2];
        float           damping;
        float           dTime;
        float           dStiffness; // Reciprocal of number of solver iterations: 1/n
        float           pad1;
        Gs::Vector4f    lightVec    = { 0.0f, 0.0f, 1.0f, 0.0f };
    }
    sceneState;

    struct ParticleBase
    {
        float uv[2];
        float invMass;
        float pad0;
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
        CreateTexture();
        CreateSampler();
        CreateComputePipeline();
        CreateGraphicsPipeline();

        // Show some information
        std::cout << "press LEFT MOUSE BUTTON and move the mouse to rotate the camera" << std::endl;
        std::cout << "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to change the cloth stiffness" << std::endl;
    }

    // Generates the grid geometry for the cloth with triangle strip topology
    void GenerateClothGeometry(
        std::vector<ParticleBase>&  verticesBase,
        std::vector<Gs::Vector4f>&  verticesPos,
        std::vector<std::uint32_t>& indices)
    {
        const auto invSegsU = 1.0f / static_cast<float>(clothSegmentsU);
        const auto invSegsV = 1.0f / static_cast<float>(clothSegmentsV);

        // Generate vertices from top to bottom, left to right
        const auto numVertices = (clothSegmentsU + 1)*(clothSegmentsV + 1);
        verticesBase.resize(numVertices);
        verticesPos.resize(numVertices);

        for (std::uint32_t v = 0; v <= clothSegmentsV; ++v)
        {
            for (std::uint32_t u = 0; u <= clothSegmentsU; ++u)
            {
                const auto idx = v * (clothSegmentsU + 1) + u;

                // Set mass for left and righ top particles to infinity to create suspension points
                bool isSuspensionPoint = (v == 0 && (u == 0 || u == clothSegmentsU));

                // Initialize base attributes
                auto& vertBase = verticesBase[idx];
                {
                    // Initialize vertex attributes to generate 2D grid
                    vertBase.uv[0] = static_cast<float>(u) * invSegsU;
                    vertBase.uv[1] = static_cast<float>(v) * invSegsV;

                    // Store mass as inverse value to simplify physics integration (zero means infinite mass)
                    vertBase.invMass = (isSuspensionPoint ? 0.0f : 1.0f / clothParticleMass);
                }

                // Initialize current and previous position for vertices
                auto& vertPos = verticesPos[idx];
                {
                    vertPos.x = vertBase.uv[0] * 2.0f - 1.0f;
                    vertPos.y = 0.0f;
                    vertPos.z = vertBase.uv[1] * -2.0f;
                }
            }
        }

        // Generate indices for triangle strips: one strip for each row, two indices for each column in the strips
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

    // Creates and initializes the particle buffer specified by <attrib>
    void CreateParticleBuffer(
        ParticleAttribute               attrib,
        LLGL::StorageBufferType         storageType,
        const void*                     initialData     = nullptr,
        const LLGL::VertexAttribute*    vertexAttrib    = nullptr)
    {
        // Initialize binding flags
        long bindFlags = 0;
        if (vertexAttrib != nullptr)
            bindFlags |= LLGL::BindFlags::VertexBuffer;

        if (storageType == LLGL::StorageBufferType::Buffer)
            bindFlags |= LLGL::BindFlags::Sampled;
        else if (storageType == LLGL::StorageBufferType::RWBuffer)
            bindFlags |= LLGL::BindFlags::Storage;

        // Create particle buffer
        LLGL::BufferDescriptor bufferDesc;
        {
            bufferDesc.size                         = sizeof(Gs::Vector4f) * numClothVertices;
            bufferDesc.storageBuffer.format         = LLGL::Format::RGBA32Float;
            bufferDesc.bindFlags                    = bindFlags;
            bufferDesc.storageBuffer.storageType    = storageType;
            if (vertexAttrib != nullptr)
                bufferDesc.vertexAttribs = { *vertexAttrib };
        }
        particleBuffers[attrib] = renderer->CreateBuffer(bufferDesc, initialData);
    }

    void CreateBuffers()
    {
        // Initialize vertex format for rendering (not all vertex attributes are required for rendering)
        vertexFormat.attributes =
        {
            LLGL::VertexAttribute{ "pos",      LLGL::Format::RGBA32Float, 0, 0, sizeof(Gs::Vector4f), 0 },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGBA32Float, 1, 0, sizeof(Gs::Vector4f), 1 },
            LLGL::VertexAttribute{ "texCoord", LLGL::Format::RG32Float,   2, 0, sizeof(Gs::Vector4f), 2 },
        };

        // Generate vertex and index data and store number of vertices and indices for draw commands
        std::vector<ParticleBase> verticesBase;
        std::vector<Gs::Vector4f> verticesPos;
        std::vector<Gs::Vector4f> zeroVectors;
        std::vector<std::uint32_t> indices;

        GenerateClothGeometry(verticesBase, verticesPos, indices);

        numClothVertices    = static_cast<std::uint32_t>(verticesPos.size());
        numClothIndices     = static_cast<std::uint32_t>(indices.size());

        zeroVectors.resize(verticesBase.size());

        // Create constant buffer
        constantBuffer = CreateConstantBuffer(sceneState);

        // Create particle buffers for each attribute
        CreateParticleBuffer(AttribBase,     LLGL::StorageBufferType::Buffer,   verticesBase.data(), &(vertexFormat.attributes[2]));
        CreateParticleBuffer(AttribCurrPos,  LLGL::StorageBufferType::RWBuffer, verticesPos.data());
        CreateParticleBuffer(AttribNextPos,  LLGL::StorageBufferType::RWBuffer, verticesPos.data());
        CreateParticleBuffer(AttribPrevPos,  LLGL::StorageBufferType::RWBuffer, verticesPos.data(), &(vertexFormat.attributes[0]));
        CreateParticleBuffer(AttribVelocity, LLGL::StorageBufferType::RWBuffer, zeroVectors.data());
        CreateParticleBuffer(AttribNormal,   LLGL::StorageBufferType::RWBuffer, zeroVectors.data(), &(vertexFormat.attributes[1]));

        // Create vertex buffer array for rendering
        LLGL::Buffer* const buffers[3] =
        {
            particleBuffers[AttribPrevPos], // Read "pos" from last written position (i.e. "prevPos") from last compute shader invocation
            particleBuffers[AttribNormal],  // Read "normal"
            particleBuffers[AttribBase]     // Read "texCoord" from .xy
        };
        vertexBufferArray = renderer->CreateBufferArray(3, buffers);

        // Create index buffer
        LLGL::BufferDescriptor indexBufferDesc;
        {
            indexBufferDesc.size        = sizeof(std::uint32_t) * indices.size();
            indexBufferDesc.bindFlags   = LLGL::BindFlags::IndexBuffer;
            indexBufferDesc.indexFormat = LLGL::Format::R32UInt;
        }
        indexBuffer = renderer->CreateBuffer(indexBufferDesc, indices.data());
    }

    void CreateTexture()
    {
        // Load color map from file
        colorMap = LoadTexture("../../Media/Textures/Logo_LLGL.png");
    }

    void CreateSampler()
    {
        // Create sampler state with linear interpolation (default configuration)
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.addressModeU = LLGL::SamplerAddressMode::Clamp;
            samplerDesc.addressModeV = LLGL::SamplerAddressMode::Clamp;
        }
        linearSampler = renderer->CreateSampler(samplerDesc);
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
        #if 0
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            computeShaders[0] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.CSForces.comp" } }
            );
            computeShaders[1] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.CSStretchConstraints.comp" } }
            );
            computeShaders[2] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.CSRelaxation.comp" } }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            computeShaders[0] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.CSForces.450core.comp.spv" } }
            );
            computeShaders[1] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.CSStretchConstraints.450core.comp.spv" } }
            );
            computeShaders[2] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.CSRelaxation.450core.comp.spv" } }
            );
        }
        #endif
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            computeShaders[0] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.metal", "CSForces", "2.0" } }
            );
            computeShaders[1] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.metal", "CSStretchConstraints", "2.0" } }
            );
            computeShaders[2] = LoadShaderProgram(
                { { LLGL::ShaderType::Compute, "Example.metal", "CSRelaxation", "2.0" } }
            );
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create compute pipeline layout
        computeLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc(
                "cbuffer(SceneState@0):comp,"
                "buffer(parBase@1),"
                "rwbuffer(parCurrPos@2, parNextPos@3, parPrevPos@4, parVelocity@5, parNormal@6):comp"
            )
        );

        // Create resource heaps for compute pipeline
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = computeLayout;
            resourceHeapDesc.resourceViews  =
            {
                constantBuffer,
                particleBuffers[AttribBase],
                particleBuffers[AttribCurrPos],
                particleBuffers[AttribNextPos],
                particleBuffers[AttribPrevPos],
                particleBuffers[AttribVelocity],
                particleBuffers[AttribNormal]
            };
        }
        computeResourceHeaps[0] = renderer->CreateResourceHeap(resourceHeapDesc);

        {
            resourceHeapDesc.resourceViews =
            {
                constantBuffer,
                particleBuffers[AttribBase],
                particleBuffers[AttribNextPos], // Swap pos with next-pos
                particleBuffers[AttribCurrPos], // Swap next-pos with pos
                particleBuffers[AttribPrevPos],
                particleBuffers[AttribVelocity],
                particleBuffers[AttribNormal]
            };
        }
        computeResourceHeaps[1] = renderer->CreateResourceHeap(resourceHeapDesc);

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
        #if 0
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.VS.vert" },
                    { LLGL::ShaderType::Fragment, "Example.PS.frag" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.VS.450core.vert.spv" },
                    { LLGL::ShaderType::Fragment, "Example.PS.450core.frag.spv" }
                },
                { vertexFormat }
            );
        }
        #endif
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            graphicsShader = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.metal", "VS", "2.0" },
                    { LLGL::ShaderType::Fragment, "Example.metal", "PS", "2.0" }
                },
                { vertexFormat }
            );
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create graphics pipeline layout
        graphicsLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(SceneState@3):vert:frag, texture(colorMap@4):frag, sampler(linearSampler@5):frag")
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
            resourceHeapDesc.resourceViews  = { constantBuffer, colorMap, linearSampler };
        }
        graphicsResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

private:

    void UpdateScene()
    {
        // Update user input
        auto motion = input->GetMouseMotion();

        if (input->KeyPressed(LLGL::Key::LButton))
        {
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -90.0f, 90.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }

        if (input->KeyPressed(LLGL::Key::RButton))
        {
            float delta = motion.x*0.01f;
            stiffnessFactor = std::max(0.5f, std::min(stiffnessFactor + delta, 1.0f));
            std::cout << "stiffness: " << static_cast<int>(stiffnessFactor * 100.0f) << "%    \r";
            std::flush(std::cout);
        }

        // Update timer
        timer->MeasureTime();
        sceneState.damping      = (1.0f - std::pow(10.0f, -dampingFactor));
        sceneState.dTime        = std::max(0.0001f, static_cast<float>(timer->GetDeltaTime()));
        sceneState.dStiffness   = 1.0f - std::pow(1.0f - stiffnessFactor, 1.0f / static_cast<float>(numSolverIterations));
        sceneState.gravity      = Gs::Vector4f{ gravityVector, 0.0f };

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

            // Run compute shader to apply particle forces
            commands->PushDebugGroup("CSForces");
            {
                commands->SetPipelineState(*computePipelines[CSForces]);
                commands->SetComputeResourceHeap(*computeResourceHeaps[swapBufferIndex]);
                commands->Dispatch(clothSegmentsU + 1, clothSegmentsV + 1, 1);
            }
            commands->PopDebugGroup();

            // Run compute shader to apply stretch constraints with number of integration steps
            commands->PushDebugGroup("CSStretchConstraints");
            {
                commands->SetPipelineState(*computePipelines[CSStretchConstraints]);

                for (std::uint32_t i = 0; i < numSolverIterations; ++i)
                {
                    if (i > 0)
                        swapBufferIndex = (swapBufferIndex + 1) % 2;
                    commands->SetComputeResourceHeap(*computeResourceHeaps[swapBufferIndex]);
                    commands->Dispatch(clothSegmentsU + 1, clothSegmentsV + 1, 1);
                }
            }
            commands->PopDebugGroup();

            // Run compute shader to adjust velocity of particles
            commands->PushDebugGroup("CSRelaxation");
            {
                commands->SetPipelineState(*computePipelines[CSRelaxation]);
                commands->SetComputeResourceHeap(*computeResourceHeaps[swapBufferIndex]);
                commands->Dispatch(clothSegmentsU + 1, clothSegmentsV + 1, 1);
            }
            commands->PopDebugGroup();

            commands->ResetResourceSlots(LLGL::ResourceType::Buffer, 4, 3, LLGL::BindFlags::Storage, LLGL::StageFlags::ComputeStage);
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
                commands->SetVertexBufferArray(*vertexBufferArray);
                commands->SetIndexBuffer(*indexBuffer);

                // Draw scene with indirect argument buffer
                commands->SetPipelineState(*graphicsPipeline);
                commands->SetGraphicsResourceHeap(*graphicsResourceHeap);
                commands->DrawIndexed(numClothIndices, 0);

                commands->ResetResourceSlots(LLGL::ResourceType::Buffer, 0, 2, LLGL::BindFlags::VertexBuffer, LLGL::StageFlags::VertexStage);
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



