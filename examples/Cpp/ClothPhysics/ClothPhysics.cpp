/*
 * ClothPhysics.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>

// Enable wireframe polygon mode
#define ENABLE_WIREFRAME 0


class Example_ClothPhysics : public ExampleBase
{

    enum class ComputeMode
    {
        ComputeRWTextures,      // Run physics simulation in compute shaders with RW textures for better cache utilization (Preferred pipeline for performance).
        ComputeRWBuffers,       // If the RW texture pipeline is not supported, use RW buffers in compute shaders instead.
        StreamOutputBuffers,    // If compute shaders are not supported, use stream-output buffers to run physics simulation on the GPU (WebGL, GLES 3.0).
    };

    enum PhysicsShader
    {
        Shader_Forces = 0,
        Shader_StretchConstraints,
        Shader_Relaxation,

        NumPhysicsShaders
    };

    enum ParticleAttribute
    {
        Attrib_Base = 0, // Original position, UV coordinates, inverse mass: float4[2] per particle
        Attrib_CurrPos,  // Current position: float4 per particle
        Attrib_NextPos,  // Next position: float4 per particle
        Attrib_PrevPos,  // Previous position: float4 per particle
        Attrib_Velocity, // Velocity vector: float4 per particle
        Attrib_Normal,   // Surface normal: float4 per particle

        NumAttribsCS,

        Attrib_VelocitySwap = NumAttribsCS,
        Attrib_NormalSwap,

        NumAttribsSO,
    };

    const std::uint32_t     numSolverIterations                 = 8;    // Number of integration steps to resolve stretching constraints between particles, good values are in [1, 10]
    const std::uint32_t     clothSegmentsU                      = 16;   // Number of segments in horizontal direction for cloth geometry
    const std::uint32_t     clothSegmentsV                      = 16;   // Number of segments in vertical direction for cloth geometry
    const float             clothParticleMass                   = 1.0f;
    const Gs::Vector3f      gravityVector                       = { 0, -9.81f * 0.2f, 0 };
    const float             dampingFactor                       = 3.8f;
    float                   stiffnessFactor                     = 1.0f; // Should be in [0, 1]
    const Gs::Vector3f      viewPos                             = { 0, -0.75f, -5 };

    const char*             physicsPSODebugNames[3]             = { "Forces.PSO", "StretchConstraints.PSO", "Relaxation.PSO" };

    ComputeMode             computeMode                         = ComputeMode::ComputeRWTextures;

    LLGL::Buffer*           constantBuffer                      = nullptr;
    LLGL::Buffer*           indexBuffer                         = nullptr;

    LLGL::Buffer*           vertexBufferNull                    = nullptr;

    LLGL::BufferArray*      vertexBufferArray                   = nullptr;
    LLGL::Resource*         particleBuffers[NumAttribsSO]       = {};
    LLGL::Texture*          particleTextures[NumAttribsSO]      = {};

    LLGL::Texture*          colorMap                            = nullptr;
    LLGL::Sampler*          linearSampler                       = nullptr;

    LLGL::PipelineLayout*   computeLayout                       = nullptr;
    LLGL::ResourceHeap*     computeResourceHeap                 = nullptr; // Contains two descriptor sets for a swap-buffer fashion

    LLGL::Shader*           physicsShaders[NumPhysicsShaders]   = {};
    LLGL::PipelineLayout*   physicsLayout[NumPhysicsShaders]    = {};
    LLGL::PipelineState*    physicsPipelines[NumPhysicsShaders] = {};

    ShaderPipeline          graphicsShaderPipeline;
    LLGL::PipelineLayout*   graphicsLayout                      = nullptr;
    LLGL::PipelineState*    graphicsPipeline                    = nullptr;
    LLGL::ResourceHeap*     graphicsResourceHeap                = nullptr;

    std::uint32_t           numClothVertices                    = 0;
    std::uint32_t           numClothIndices                     = 0;
    std::uint32_t           swapBufferIndex                     = 0; // Index to swap particle buffer heaps
    bool                    swapVelocityBuffer                  = false;
    bool                    swapNormalBuffer                    = false;

    struct alignas(16) SceneState
    {
        Gs::Matrix4f    wvpMatrix;
        Gs::Matrix4f    wMatrix;
        Gs::Vector4f    gravity;
        std::uint32_t   gridSize[2] = {};
        std::uint32_t   pad0[2]     = {};
        float           damping     = {};
        float           dTime       = {};
        float           dStiffness  = {}; // Reciprocal of number of solver iterations: 1/n
        float           pad1        = {};
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
        ExampleBase { "LLGL Example: Cloth Physics" }
    {
        // Check if compute pipeline is supported
        const auto& renderCaps = renderer->GetRenderingCaps();
        if (renderCaps.features.hasComputeShaders)
            computeMode = (IsDirect3D() ? ComputeMode::ComputeRWTextures : ComputeMode::ComputeRWBuffers);
        else
            computeMode = ComputeMode::StreamOutputBuffers;

        if (computeMode != ComputeMode::StreamOutputBuffers)
        {
            if (!MinimumShaderModel(/*hlsl:*/ "5.0", /*glsl:*/ "430", /*essl:*/ "310", /*metal:*/ "2.0"))
                computeMode = ComputeMode::StreamOutputBuffers;
        }

        if (computeMode == ComputeMode::StreamOutputBuffers)
        {
            if (!MinimumShaderModel(/*hlsl:*/ "4.0", /*glsl:*/ "330", /*essl:*/ "300", /*metal:*/ "1.0"))
                return;
        }

        const char* computeModeLabel[] =
        {
            "ComputeRWTextures",
            "ComputeRWBuffers",
            "StreamOutputBuffers",
        };
        LLGL::Log::Printf("-> COMPUTE MODE: %s\n\n", computeModeLabel[static_cast<int>(computeMode)]);

        // Create all graphics objects
        CreateResources();
        if (computeMode == ComputeMode::StreamOutputBuffers)
            CreateStreamOutputPipeline();
        else
            CreateComputePipeline();
        CreateGraphicsPipeline();

        // Update vectors for projection
        sceneState.lightVec.z *= GetProjectionZAxis();

        // Show some information
        LLGL::Log::Printf(
            "press LEFT MOUSE BUTTON and move the mouse to rotate the camera\n"
            "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to change the cloth stiffness\n"
        );
    }

    // Generates the grid geometry for the cloth with triangle strip topology
    void GenerateClothGeometry(
        std::vector<ParticleBase>&  verticesBase,
        std::vector<Gs::Vector4f>&  verticesPos,
        std::vector<std::uint32_t>& indices)
    {
        const float projZAxis = GetProjectionZAxis();

        const float invSegsU = 1.0f / static_cast<float>(clothSegmentsU);
        const float invSegsV = 1.0f / static_cast<float>(clothSegmentsV);

        // Generate vertices from top to bottom, left to right
        const std::uint32_t numVertices = (clothSegmentsU + 1)*(clothSegmentsV + 1);
        verticesBase.resize(numVertices);
        verticesPos.resize(numVertices);

        for (std::uint32_t v = 0; v <= clothSegmentsV; ++v)
        {
            for (std::uint32_t u = 0; u <= clothSegmentsU; ++u)
            {
                const auto idx = v * (clothSegmentsU + 1) + u;

                // Set mass for left and right top particles to infinity to create suspension points
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
                    vertPos.z = vertBase.uv[1] * -2.0f * projZAxis;
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
        ParticleAttribute       attrib,
        const char*             debugName,
        LLGL::StorageBufferType storageType,
        const void*             initialData     = nullptr,
        std::uint32_t           vertexStride    = 0)
    {
        if (computeMode == ComputeMode::ComputeRWTextures)
        {
            // Initialize binding flags
            long bindFlags = LLGL::BindFlags::Sampled;
            if (storageType == LLGL::StorageBufferType::RWTypedBuffer)
                bindFlags |= LLGL::BindFlags::Storage;

            // Create particle buffers as texture
            LLGL::TextureDescriptor texDesc;
            {
                texDesc.debugName       = debugName;
                texDesc.bindFlags       = bindFlags;
                texDesc.format          = LLGL::Format::RGBA32Float;
                texDesc.extent.width    = clothSegmentsU + 1;
                texDesc.extent.height   = clothSegmentsV + 1;
                texDesc.mipLevels       = 1;
            }
            LLGL::ImageView imageView;
            {
                imageView.format    = LLGL::ImageFormat::RGBA;
                imageView.dataType  = LLGL::DataType::Float32;
                imageView.dataSize  = sizeof(Gs::Vector4f) * numClothVertices;
                imageView.data      = initialData;
            }
            particleBuffers[attrib] = renderer->CreateTexture(texDesc, &imageView);
        }
        else
        {
            // Initialize binding flags
            long bindFlags = 0;
            if (vertexStride != 0)
                bindFlags |= LLGL::BindFlags::VertexBuffer;

            if (storageType == LLGL::StorageBufferType::TypedBuffer)
            {
                if (computeMode != ComputeMode::StreamOutputBuffers)
                    bindFlags |= LLGL::BindFlags::Sampled;
            }
            else if (storageType == LLGL::StorageBufferType::RWTypedBuffer)
            {
                if (computeMode == ComputeMode::StreamOutputBuffers)
                    bindFlags |= (LLGL::BindFlags::StreamOutputBuffer | LLGL::BindFlags::CopyDst | LLGL::BindFlags::CopySrc);
                else
                    bindFlags |= LLGL::BindFlags::Storage;
            }

            // Create particle buffers
            LLGL::BufferDescriptor bufferDesc;
            {
                bufferDesc.debugName    = debugName;
                bufferDesc.size         = sizeof(Gs::Vector4f) * numClothVertices;
                bufferDesc.stride       = vertexStride;
                bufferDesc.bindFlags    = bindFlags;
                bufferDesc.format       = LLGL::Format::RGBA32Float;
            }
            particleBuffers[attrib] = renderer->CreateBuffer(bufferDesc, initialData);

            if (computeMode == ComputeMode::StreamOutputBuffers)
            {
                LLGL::TextureDescriptor texDesc;
                {
                    texDesc.debugName       = debugName;
                    texDesc.bindFlags       = LLGL::BindFlags::Sampled | LLGL::BindFlags::CopyDst;
                    texDesc.format          = LLGL::Format::RGBA32Float;
                    texDesc.extent.width    = clothSegmentsU + 1;
                    texDesc.extent.height   = clothSegmentsV + 1;
                    texDesc.mipLevels       = 1;
                }
                LLGL::ImageView imageView;
                {
                    imageView.format    = LLGL::ImageFormat::RGBA;
                    imageView.dataType  = LLGL::DataType::Float32;
                    imageView.dataSize  = sizeof(Gs::Vector4f) * numClothVertices;
                    imageView.data      = initialData;
                }
                particleTextures[attrib] = renderer->CreateTexture(texDesc, &imageView);
            }
        }
    }

    void CreateResources()
    {
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
        CreateParticleBuffer(Attrib_Base,     "Particles.Base",             LLGL::StorageBufferType::TypedBuffer,   verticesBase.data(), sizeof(ParticleBase));
        CreateParticleBuffer(Attrib_CurrPos,  "Particles.CurrentPosition",  LLGL::StorageBufferType::RWTypedBuffer, verticesPos.data());
        CreateParticleBuffer(Attrib_NextPos,  "Particles.NextPosition",     LLGL::StorageBufferType::RWTypedBuffer, verticesPos.data());
        CreateParticleBuffer(Attrib_PrevPos,  "Particles.PreviousPosition", LLGL::StorageBufferType::RWTypedBuffer, verticesPos.data(), sizeof(Gs::Vector4f));
        CreateParticleBuffer(Attrib_Velocity, "Particles.Velocity",         LLGL::StorageBufferType::RWTypedBuffer, zeroVectors.data());
        CreateParticleBuffer(Attrib_Normal,   "Particles.Normal",           LLGL::StorageBufferType::RWTypedBuffer, zeroVectors.data(), sizeof(Gs::Vector4f));

        if (computeMode == ComputeMode::StreamOutputBuffers)
        {
            CreateParticleBuffer(Attrib_VelocitySwap, "Particles.VelocitySwap", LLGL::StorageBufferType::RWTypedBuffer, zeroVectors.data());
            CreateParticleBuffer(Attrib_NormalSwap,   "Particles.NormalSwap",   LLGL::StorageBufferType::RWTypedBuffer, zeroVectors.data(), sizeof(Gs::Vector4f));
        }

        // Create dummy vertex buffer
        LLGL::BufferDescriptor vbNullDesc;
        {
            vbNullDesc.debugName    = "Buffer.Null";
            vbNullDesc.size         = 1;
            vbNullDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
        }
        vertexBufferNull = renderer->CreateBuffer(vbNullDesc);

        if (computeMode != ComputeMode::ComputeRWTextures)
        {
            // Create vertex buffer array for rendering
            const LLGL::VertexBufferView bufferViews[3] =
            {
                LLGL::CastTo<LLGL::Buffer>(particleBuffers[Attrib_PrevPos]), // Read "pos" from last written position (i.e. "prevPos") from last compute shader invocation
                LLGL::CastTo<LLGL::Buffer>(particleBuffers[Attrib_Normal]),  // Read "normal"
                LLGL::CastTo<LLGL::Buffer>(particleBuffers[Attrib_Base])     // Read "texCoord" from .xy
            };
            vertexBufferArray = renderer->CreateBufferArray(bufferViews);
            vertexBufferArray->SetDebugName("BufferArray.Vertices");
        }

        // Create index buffer
        LLGL::BufferDescriptor indexBufferDesc;
        {
            indexBufferDesc.debugName   = "Buffer.Indices";
            indexBufferDesc.size        = sizeof(std::uint32_t) * indices.size();
            indexBufferDesc.bindFlags   = LLGL::BindFlags::IndexBuffer;
            indexBufferDesc.format      = LLGL::Format::R32UInt;
        }
        indexBuffer = renderer->CreateBuffer(indexBufferDesc, indices.data());

        // Load color map from file
        colorMap = LoadTexture("Logos/Logo_LLGL.png");

        // Create sampler state with linear interpolation (default configuration)
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.addressModeU = LLGL::SamplerAddressMode::Clamp;
            samplerDesc.addressModeV = LLGL::SamplerAddressMode::Clamp;
        }
        linearSampler = renderer->CreateSampler(samplerDesc);
    }

    const LLGL::ShaderMacro* GetShaderMacros() const
    {
        static const LLGL::ShaderMacro k_shaderMacrosTextures[] = { { "ENABLE_STORAGE_TEXTURES", "1" }, {} };
        static const LLGL::ShaderMacro k_shaderMacrosBuffers[] = { {} };
        return (computeMode == ComputeMode::ComputeRWTextures ? k_shaderMacrosTextures : k_shaderMacrosBuffers);
    }

    void CreateComputePipeline()
    {
        // Create compute shader
        if (computeMode == ComputeMode::ComputeRWTextures && !Supported(LLGL::ShadingLanguage::HLSL))
        {
            physicsShaders[0] = LoadComputeShader("ClothPhysics.RWTextures", "CSForces",             GetShaderMacros());
            physicsShaders[1] = LoadComputeShader("ClothPhysics.RWTextures", "CSStretchConstraints", GetShaderMacros());
            physicsShaders[2] = LoadComputeShader("ClothPhysics.RWTextures", "CSRelaxation",         GetShaderMacros());
        }
        else
        {
            physicsShaders[0] = LoadComputeShader("ClothPhysics", "CSForces",             GetShaderMacros());
            physicsShaders[1] = LoadComputeShader("ClothPhysics", "CSStretchConstraints", GetShaderMacros());
            physicsShaders[2] = LoadComputeShader("ClothPhysics", "CSRelaxation",         GetShaderMacros());
        }

        // Create compute pipeline layout
        if (computeMode == ComputeMode::ComputeRWTextures)
        {
            computeLayout = renderer->CreatePipelineLayout(
                LLGL::Parse(
                    "heap{"
                    "cbuffer(SceneState@7):comp,"
                    "texture(parBase@1):comp,"
                    "rwtexture(parCurrPos@2, parNextPos@3, parPrevPos@4, parVelocity@5, parNormal@6):comp,"
                    "},"
                    "barriers{rwtexture},"
                )
            );
        }
        else
        {
            computeLayout = renderer->CreatePipelineLayout(
                LLGL::Parse(
                    "heap{"
                    "cbuffer(SceneState@7):comp,"
                    "tbuffer(parBase@1):comp,"
                    "rwtbuffer(parCurrPos@2, parNextPos@3, parPrevPos@4, parVelocity@5, parNormal@6):comp,"
                    "},"
                    "barriers{rwbuffer},"
                )
            );
        }

        // Create compute pipeline
        for (int i = 0; i < 3; ++i)
        {
            LLGL::ComputePipelineDescriptor pipelineDesc;
            {
                pipelineDesc.debugName      = physicsPSODebugNames[i];
                pipelineDesc.pipelineLayout = computeLayout;
                pipelineDesc.computeShader  = physicsShaders[i];
            }
            physicsPipelines[i] = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(physicsPipelines[i]);
        }

        // Create resource heaps for compute pipeline
        const LLGL::ResourceViewDescriptor resourceViews[] =
        {
            constantBuffer,
            particleBuffers[Attrib_Base],
            particleBuffers[Attrib_CurrPos],
            particleBuffers[Attrib_NextPos],
            particleBuffers[Attrib_PrevPos],
            particleBuffers[Attrib_Velocity],
            particleBuffers[Attrib_Normal],

            constantBuffer,
            particleBuffers[Attrib_Base],
            particleBuffers[Attrib_NextPos], // Swap pos with next-pos
            particleBuffers[Attrib_CurrPos], // Swap next-pos with pos
            particleBuffers[Attrib_PrevPos],
            particleBuffers[Attrib_Velocity],
            particleBuffers[Attrib_Normal],
        };
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout     = computeLayout;
            resourceHeapDesc.numResourceViews   = sizeof(resourceViews) / sizeof(resourceViews[0]);
        }
        computeResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc, resourceViews);
    }

    void CreateStreamOutputPipeline()
    {
        // Create vertex shaders with stream-output
        physicsShaders[0] = LoadVertexShader("ClothPhysics.StreamOutput", "VSForces"            );
        physicsShaders[1] = LoadVertexShader("ClothPhysics.StreamOutput", "VSStretchConstraints");
        physicsShaders[2] = LoadVertexShader("ClothPhysics.StreamOutput", "VSRelaxation"        );

        // Create physics pipeline layouts
        physicsLayout[Shader_Forces] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneState@7):vert,"
                "texture(parBase@1, parCurrPos@2, parVelocity@5):vert,"
            )
        );

        physicsLayout[Shader_StretchConstraints] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneState@7):vert,"
                "texture(parBase@1, parCurrPos@2, parNormal@6):vert,"
            )
        );

        physicsLayout[Shader_Relaxation] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "cbuffer(SceneState@7):vert,"
                "texture(parNextPos@3, parPrevPos@4):vert,"
            )
        );

        // Create vertex pipelines with stream-outputs
        std::vector<std::vector<LLGL::VertexAttribute>> vertexAttribs;

        if (IsDirect3D())
        {
            vertexAttribs =
            {
                // VSForces
                std::vector<LLGL::VertexAttribute>
                {
                    LLGL::VertexAttribute{ "currPos",  LLGL::Format::RGBA32Float, 0, 0, sizeof(Gs::Vector4f), 0 },
                },

                // VSStretchConstraints
                std::vector<LLGL::VertexAttribute>
                {
                    LLGL::VertexAttribute{ "nextPos",  LLGL::Format::RGBA32Float, 0, 0, sizeof(Gs::Vector4f), 0 },
                    LLGL::VertexAttribute{ "normal",   LLGL::Format::RGBA32Float, 1, 0, sizeof(Gs::Vector4f), 1 },
                },

                // VSRelaxation
                std::vector<LLGL::VertexAttribute>
                {
                    LLGL::VertexAttribute{ "currPos",  LLGL::Format::RGBA32Float, 0, 0, sizeof(Gs::Vector4f), 0 },
                    LLGL::VertexAttribute{ "velocity", LLGL::Format::RGBA32Float, 1, 0, sizeof(Gs::Vector4f), 1 },
                },
            };
        }
        else
        {
            vertexAttribs =
            {
                // VSForces
                std::vector<LLGL::VertexAttribute>
                {
                    LLGL::VertexAttribute{ "v_CURRPOS",  LLGL::Format::RGBA32Float, 0, 0, sizeof(Gs::Vector4f), 0 },
                },

                // VSStretchConstraints
                std::vector<LLGL::VertexAttribute>
                {
                    LLGL::VertexAttribute{ "v_NEXTPOS",  LLGL::Format::RGBA32Float, 0, 0, sizeof(Gs::Vector4f), 0 },
                    LLGL::VertexAttribute{ "v_NORMAL",   LLGL::Format::RGBA32Float, 1, 0, sizeof(Gs::Vector4f), 1 },
                },

                // VSRelaxation
                std::vector<LLGL::VertexAttribute>
                {
                    LLGL::VertexAttribute{ "v_CURRPOS",  LLGL::Format::RGBA32Float, 0, 0, sizeof(Gs::Vector4f), 0 },
                    LLGL::VertexAttribute{ "v_VELOCITY", LLGL::Format::RGBA32Float, 1, 0, sizeof(Gs::Vector4f), 1 },
                },
            };
        }

        for (int i = 0; i < 3; ++i)
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.debugName                  = physicsPSODebugNames[i];
                pipelineDesc.pipelineLayout             = physicsLayout[i];
                pipelineDesc.outputVertexAttribs        = vertexAttribs[i];
                pipelineDesc.vertexShader               = physicsShaders[i];
                pipelineDesc.primitiveTopology          = LLGL::PrimitiveTopology::PointList;
                pipelineDesc.rasterizer.discardEnabled  = true; // Don't rasterize anything with this PSO, only write to stream-outputs
                pipelineDesc.blend.targets[0].colorMask = 0x00;
            }
            physicsPipelines[i] = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(physicsPipelines[i]);
        }
    }

    void CreateGraphicsPipeline()
    {
        // Create graphics shader
        if (computeMode == ComputeMode::ComputeRWTextures && !Supported(LLGL::ShadingLanguage::HLSL))
        {
            graphicsShaderPipeline.vs = LoadVertexShader  ("ClothPhysics.RWTextures", "VS", GetShaderMacros());
            graphicsShaderPipeline.ps = LoadFragmentShader("ClothPhysics",            "PS", GetShaderMacros());
        }
        else
        {
            graphicsShaderPipeline.vs = LoadVertexShader  ("ClothPhysics", "VS", GetShaderMacros());
            graphicsShaderPipeline.ps = LoadFragmentShader("ClothPhysics", "PS", GetShaderMacros());
        }

        // Create graphics pipeline layout
        std::vector<LLGL::VertexAttribute> vertexAttribs;

        if (computeMode == ComputeMode::ComputeRWTextures)
        {
            graphicsLayout = renderer->CreatePipelineLayout(
                LLGL::Parse(
                    "heap{"
                    "cbuffer(SceneState@7):vert:frag,"
                    "texture(colorMap@4):frag,"
                    "sampler(linearSampler@5):frag,"
                    "texture(vertexBase@1,vertexPos@2,vertexNormal@6):vert"
                    "},"
                    "sampler<colorMap, linearSampler>(s_colorMaplinearSampler@4),"
                    "barriers{rwtexture},"
                )
            );
        }
        else
        {
            // Initialize vertex format for rendering (not all vertex attributes are required for rendering)
            vertexAttribs =
            {
                LLGL::VertexAttribute{ "pos",      LLGL::Format::RGBA32Float, /*location:*/ 0, /*offset:*/ 0, /*stride:*/ sizeof(Gs::Vector4f), /*slot:*/ 0 },
                LLGL::VertexAttribute{ "normal",   LLGL::Format::RGBA32Float, /*location:*/ 1, /*offset:*/ 0, /*stride:*/ sizeof(Gs::Vector4f), /*slot:*/ 1 },
                LLGL::VertexAttribute{ "texCoord", LLGL::Format::RG32Float,   /*location:*/ 2, /*offset:*/ 0, /*stride:*/ sizeof(ParticleBase), /*slot:*/ 2 },
            };

            graphicsLayout = renderer->CreatePipelineLayout(
                LLGL::Parse(
                    "heap{"
                    "cbuffer(SceneState@7):vert:frag,"
                    "texture(colorMap@4):frag,"
                    "sampler(linearSampler@5):frag"
                    "},"
                    "sampler<colorMap, linearSampler>(s_colorMaplinearSampler@4),"
                )
            );
        }

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.debugName                      = "Scene.PSO";
            pipelineDesc.pipelineLayout                 = graphicsLayout;
            pipelineDesc.inputVertexAttribs             = vertexAttribs;
            pipelineDesc.vertexShader                   = graphicsShaderPipeline.vs;
            pipelineDesc.fragmentShader                 = graphicsShaderPipeline.ps;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            #if ENABLE_WIREFRAME
            pipelineDesc.rasterizer.polygonMode         = LLGL::PolygonMode::Wireframe;
            #endif

            // Flip triangle winding for right-handed projection, since we did not change index generation for our cloth geometry.
            pipelineDesc.rasterizer.frontCCW            = HasRightHandedProjection();
        }
        graphicsPipeline = renderer->CreatePipelineState(pipelineDesc);
        ReportPSOErrors(graphicsPipeline);

        // Create resource heaps for graphics pipeline
        const LLGL::ResourceViewDescriptor resourceViewsGraphics[] =
        {
            constantBuffer,
            colorMap,
            linearSampler,

            // Only for ComputeRWTextures pipeline
            particleBuffers[Attrib_Base],
            particleBuffers[Attrib_CurrPos],
            particleBuffers[Attrib_Normal],
        };
        const LLGL::ArrayView<LLGL::ResourceViewDescriptor> resourceViewsGraphicsArray
        {
            resourceViewsGraphics,
            (computeMode == ComputeMode::ComputeRWTextures ? 6u : 3u)
        };
        graphicsResourceHeap = renderer->CreateResourceHeap(graphicsLayout, resourceViewsGraphicsArray);
    }

private:

    void UpdateScene(float dt)
    {
        const float projZAxis = GetProjectionZAxis();

        // Update user input
        auto motion = input.GetMouseMotion();

        if (input.KeyPressed(LLGL::Key::RButton))
        {
            float delta = motion.x*0.01f;
            stiffnessFactor = std::max(0.5f, std::min(stiffnessFactor + delta, 1.0f));
            LLGL::Log::Printf("stiffness: %d%%    \r", static_cast<int>(stiffnessFactor * 100.0f));
            ::fflush(stdout);
        }

        // Update timer
        sceneState.damping      = (1.0f - std::pow(10.0f, -dampingFactor));
        sceneState.dTime        = std::max(0.0001f, std::min(dt, 1.0f));
        sceneState.dStiffness   = 1.0f - std::pow(1.0f - stiffnessFactor, 1.0f / static_cast<float>(numSolverIterations));
        sceneState.gravity      = Gs::Vector4f{ gravityVector, 0.0f };

        static Gs::Quaternionf rotation;
        if (input.KeyPressed(LLGL::Key::LButton))
            TrackballRotation(rotation, input.KeyDown(LLGL::Key::LButton));

        // Update world matrix
        sceneState.wMatrix.LoadIdentity();
        Gs::QuaternionToMatrix(sceneState.wMatrix, rotation);

        // Update view matrix
        Gs::Matrix4f vMatrix;
        Gs::Translate(vMatrix, { viewPos.x, viewPos.y, viewPos.z * projZAxis });
        vMatrix.MakeInverse();

        // Update world-view-projection matrix
        sceneState.wvpMatrix = projection * vMatrix * sceneState.wMatrix;
    }

    void DispatchParticleWorkGroups()
    {
        const std::uint32_t numWorkGroupsX = clothSegmentsU + 1;
        const std::uint32_t numWorkGroupsY = clothSegmentsV + 1;

        if (computeMode != ComputeMode::StreamOutputBuffers)
        {
            // Run compute shader invocations for work groups
            commands->Dispatch(numWorkGroupsX, numWorkGroupsY, 1);
        }
        else
        {
            // Simulate work groups by writing to stream-output in vertex shader invocations
            commands->Draw(numWorkGroupsX*numWorkGroupsY, 0);
        }
    }

    void RunPhysicsSimulationWithComputePipeline()
    {
        // Run compute shader to apply particle forces
        commands->PushDebugGroup("Forces");
        {
            commands->SetPipelineState(*physicsPipelines[Shader_Forces]);
            commands->SetResourceHeap(*computeResourceHeap, swapBufferIndex);
            DispatchParticleWorkGroups();
        }
        commands->PopDebugGroup();

        // Run compute shader to apply stretch constraints with number of integration steps
        commands->PushDebugGroup("StretchConstraints");
        {
            commands->SetPipelineState(*physicsPipelines[Shader_StretchConstraints]);

            for (std::uint32_t i = 0; i < numSolverIterations; ++i)
            {
                if (i > 0)
                    swapBufferIndex = (swapBufferIndex + 1) % 2;
                commands->SetResourceHeap(*computeResourceHeap, swapBufferIndex);
                DispatchParticleWorkGroups();
            }
        }
        commands->PopDebugGroup();

        // Run compute shader to adjust velocity of particles
        commands->PushDebugGroup("Relaxation");
        {
            commands->SetPipelineState(*physicsPipelines[Shader_Relaxation]);
            commands->SetResourceHeap(*computeResourceHeap, swapBufferIndex);
            DispatchParticleWorkGroups();
        }
        commands->PopDebugGroup();
    }

    LLGL::Buffer* GetParticleBufferPositionIn() const
    {
        return LLGL::CastTo<LLGL::Buffer>(particleBuffers[swapBufferIndex == 0 ? Attrib_CurrPos : Attrib_NextPos]);
    }

    LLGL::Buffer* GetParticleBufferPositionOut() const
    {
        return LLGL::CastTo<LLGL::Buffer>(particleBuffers[swapBufferIndex == 0 ? Attrib_NextPos : Attrib_CurrPos]);
    }

    LLGL::Buffer* GetParticleBufferNormalIn() const
    {
        return LLGL::CastTo<LLGL::Buffer>(particleBuffers[!swapNormalBuffer ? Attrib_Normal : Attrib_NormalSwap]);
    }

    LLGL::Buffer* GetParticleBufferNormalOut() const
    {
        return LLGL::CastTo<LLGL::Buffer>(particleBuffers[!swapNormalBuffer ? Attrib_NormalSwap : Attrib_Normal]);
    }

    LLGL::Buffer* GetParticleBufferVelocityIn() const
    {
        return LLGL::CastTo<LLGL::Buffer>(particleBuffers[!swapVelocityBuffer ? Attrib_Velocity : Attrib_VelocitySwap]);
    }

    LLGL::Buffer* GetParticleBufferVelocityOut() const
    {
        return LLGL::CastTo<LLGL::Buffer>(particleBuffers[!swapVelocityBuffer ? Attrib_VelocitySwap : Attrib_Velocity]);
    }

    void CopyParticleBufferToTexture(ParticleAttribute attrib)
    {
        LLGL::TextureRegion region;
        region.extent = { clothSegmentsU + 1, clothSegmentsV + 1, 1 };
        commands->CopyTextureFromBuffer(*particleTextures[attrib], region, *LLGL::CastTo<LLGL::Buffer>(particleBuffers[attrib]), 0);
    }

    void SwapPositionBuffer()
    {
        CopyParticleBufferToTexture(swapBufferIndex == 0 ? Attrib_NextPos : Attrib_CurrPos);
        swapBufferIndex = (swapBufferIndex + 1) % 2;
    }

    void SwapNormalBuffer()
    {
        CopyParticleBufferToTexture(!swapNormalBuffer ? Attrib_NormalSwap : Attrib_Normal);
        swapNormalBuffer = !swapNormalBuffer;
    }

    void SwapVelocityBuffer()
    {
        CopyParticleBufferToTexture(!swapVelocityBuffer ? Attrib_VelocitySwap : Attrib_Velocity);
        swapVelocityBuffer = !swapVelocityBuffer;
    }

    void RunPhysicsSimulationWithStreamOutputs()
    {
        commands->SetVertexBuffer(*vertexBufferNull);

        // Run compute shader to apply particle forces
        commands->PushDebugGroup("VSForces");
        {
            commands->SetPipelineState(*physicsPipelines[Shader_Forces]);
            commands->SetResource(0, *constantBuffer);
            commands->SetResource(1, *particleTextures[Attrib_Base]);
            commands->SetResource(2, *particleTextures[Attrib_PrevPos]);
            commands->SetResource(3, *particleTextures[!swapVelocityBuffer ? Attrib_Velocity : Attrib_VelocitySwap]);

            LLGL::Buffer* soBuffers[] = { GetParticleBufferPositionOut() };
            commands->BeginStreamOutput(sizeof(soBuffers)/sizeof(soBuffers[0]), soBuffers);
            {
                DispatchParticleWorkGroups();
            }
            commands->EndStreamOutput();

            SwapPositionBuffer();
        }
        commands->PopDebugGroup();

        // Run compute shader to apply stretch constraints with number of integration steps
        commands->PushDebugGroup("VSStretchConstraints");
        {
            commands->SetPipelineState(*physicsPipelines[Shader_StretchConstraints]);
            commands->SetResource(0, *constantBuffer);
            commands->SetResource(1, *particleTextures[Attrib_Base]);

            for (std::uint32_t i = 0; i < numSolverIterations; ++i)
            {
                commands->SetResource(2, *particleTextures[swapBufferIndex == 0 ? Attrib_CurrPos : Attrib_NextPos]);
                commands->SetResource(3, *particleTextures[!swapNormalBuffer ? Attrib_Normal : Attrib_NormalSwap]);

                LLGL::Buffer* soBuffers[] = { GetParticleBufferPositionOut(), GetParticleBufferNormalOut() };
                commands->BeginStreamOutput(sizeof(soBuffers)/sizeof(soBuffers[0]), soBuffers);
                {
                    DispatchParticleWorkGroups();
                }
                commands->EndStreamOutput();

                SwapPositionBuffer();
                SwapNormalBuffer();
            }
        }
        commands->PopDebugGroup();

        // Run compute shader to adjust velocity of particles
        commands->PushDebugGroup("VSRelaxation");
        {
            commands->SetPipelineState(*physicsPipelines[Shader_Relaxation]);
            commands->SetResource(0, *constantBuffer);
            commands->SetResource(1, *particleTextures[swapBufferIndex == 0 ? Attrib_CurrPos : Attrib_NextPos]);
            commands->SetResource(2, *particleTextures[Attrib_PrevPos]);

            LLGL::Buffer* soBuffers[] = { GetParticleBufferPositionOut(), GetParticleBufferVelocityOut() };
            commands->BeginStreamOutput(sizeof(soBuffers)/sizeof(soBuffers[0]), soBuffers);
            {
                DispatchParticleWorkGroups();
            }
            commands->EndStreamOutput();

            commands->CopyBuffer(*LLGL::CastTo<LLGL::Buffer>(particleBuffers[Attrib_PrevPos]), 0, *soBuffers[0], 0, sizeof(Gs::Vector4f) * numClothVertices);
            CopyParticleBufferToTexture(Attrib_PrevPos);

            if (swapNormalBuffer)
                commands->CopyBuffer(*LLGL::CastTo<LLGL::Buffer>(particleBuffers[Attrib_Normal]), 0, *GetParticleBufferNormalIn(), 0, sizeof(Gs::Vector4f) * numClothVertices);

            SwapVelocityBuffer();
            SwapPositionBuffer();
        }
        commands->PopDebugGroup();
    }

    void OnDrawFrame(float dt) override
    {
        UpdateScene(dt);

        // Record and submit compute commands
        commands->Begin();
        {
            // Update scene state constant buffer
            commands->UpdateBuffer(*constantBuffer, 0, &sceneState, sizeof(sceneState));

            if (computeMode != ComputeMode::StreamOutputBuffers)
                RunPhysicsSimulationWithComputePipeline();
            else
                RunPhysicsSimulationWithStreamOutputs();

            // Draw scene
            commands->BeginRenderPass(*swapChain);
            {
                // Clear color buffer and set viewport
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                // Set vertex and index buffers
                if (computeMode == ComputeMode::ComputeRWTextures)
                    commands->SetVertexBuffer(*vertexBufferNull);
                else
                    commands->SetVertexBufferArray(*vertexBufferArray);

                commands->SetIndexBuffer(*indexBuffer);

                // Draw scene with indirect argument buffer
                commands->SetPipelineState(*graphicsPipeline);
                commands->SetResourceHeap(*graphicsResourceHeap);
                commands->DrawIndexed(numClothIndices, 0);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_ClothPhysics);



