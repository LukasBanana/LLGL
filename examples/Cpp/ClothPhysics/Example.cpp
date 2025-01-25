/*
 * Example.cpp (Example_ClothPhysics)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>

// Enables storage textures instead of typed buffers for physics particles (i.e. RWTexture2D instead of RWBuffer in HLSL for instance).
// Currently only supported for D3D11 and D3D12
//#define ENABLE_STORAGE_TEXTURES

// Enable wireframe polygon mode
//#define ENABLE_WIREFRAME


#ifdef ENABLE_STORAGE_TEXTURES
const LLGL::ShaderMacro g_shaderMacros[] = { { "ENABLE_STORAGE_TEXTURES" }, {} };
#else
const LLGL::ShaderMacro g_shaderMacros[] = { {} };
#endif

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
    LLGL::Buffer*           indexBuffer                         = nullptr;

    #ifdef ENABLE_STORAGE_TEXTURES

    LLGL::Buffer*           vertexBufferNull                    = nullptr;
    LLGL::Texture*          particleBuffers[NumAttribs]         = {};

    #else

    LLGL::BufferArray*      vertexBufferArray                   = nullptr;
    LLGL::Buffer*           particleBuffers[NumAttribs]         = {};

    #endif // /ENABLE_STORAGE_TEXTURES

    LLGL::Texture*          colorMap                            = nullptr;
    LLGL::Sampler*          linearSampler                       = nullptr;

    LLGL::PipelineLayout*   computeLayout                       = nullptr;
    LLGL::ResourceHeap*     computeResourceHeap                 = nullptr; // Contains two descriptor sets for a swap-buffer fashion

    LLGL::Shader*           computeShaders[NumComputeShaders]   = {};
    LLGL::PipelineState*    computePipelines[NumComputeShaders] = {};

    ShaderPipeline          graphicsShaderPipeline;
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
        ExampleBase { "LLGL Example: Cloth Physics" }
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

        // Label objects
        particleBuffers[AttribBase    ]->SetDebugName("Particles.Base");
        particleBuffers[AttribCurrPos ]->SetDebugName("Particles.CurrentPosition");
        particleBuffers[AttribNextPos ]->SetDebugName("Particles.NextPosition");
        particleBuffers[AttribPrevPos ]->SetDebugName("Particles.PreviousPosition");
        particleBuffers[AttribVelocity]->SetDebugName("Particles.Velocity");
        particleBuffers[AttribNormal  ]->SetDebugName("Particles.Normal");

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
        #ifdef ENABLE_STORAGE_TEXTURES

        // Initialize binding flags
        long bindFlags = LLGL::BindFlags::Sampled;
        if (storageType == LLGL::StorageBufferType::RWTypedBuffer)
            bindFlags |= LLGL::BindFlags::Storage;

        // Create particle buffers as texture
        LLGL::TextureDescriptor texDesc;
        {
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

        #else

        // Initialize binding flags
        long bindFlags = 0;
        if (vertexAttrib != nullptr)
            bindFlags |= LLGL::BindFlags::VertexBuffer;

        if (storageType == LLGL::StorageBufferType::TypedBuffer)
            bindFlags |= LLGL::BindFlags::Sampled;
        else if (storageType == LLGL::StorageBufferType::RWTypedBuffer)
            bindFlags |= LLGL::BindFlags::Storage;

        // Create particle buffers
        LLGL::BufferDescriptor bufferDesc;
        {
            bufferDesc.size         = sizeof(Gs::Vector4f) * numClothVertices;
            bufferDesc.bindFlags    = bindFlags;
            bufferDesc.format       = LLGL::Format::RGBA32Float;
            if (vertexAttrib != nullptr)
                bufferDesc.vertexAttribs = { *vertexAttrib };
        }
        particleBuffers[attrib] = renderer->CreateBuffer(bufferDesc, initialData);

        #endif // /ENABLE_STORAGE_TEXTURES
    }

    void CreateBuffers()
    {
        // Initialize vertex format for rendering (not all vertex attributes are required for rendering)
        vertexFormat.attributes =
        {
            LLGL::VertexAttribute{ "pos",      LLGL::Format::RGBA32Float, /*location:*/ 0, /*offset:*/ 0, /*stride:*/ sizeof(Gs::Vector4f), /*slot:*/ 0 },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGBA32Float, /*location:*/ 1, /*offset:*/ 0, /*stride:*/ sizeof(Gs::Vector4f), /*slot:*/ 1 },
            LLGL::VertexAttribute{ "texCoord", LLGL::Format::RG32Float,   /*location:*/ 2, /*offset:*/ 0, /*stride:*/ sizeof(ParticleBase), /*slot:*/ 2 },
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
        CreateParticleBuffer(AttribBase,     LLGL::StorageBufferType::TypedBuffer,   verticesBase.data(), &(vertexFormat.attributes[2]));
        CreateParticleBuffer(AttribCurrPos,  LLGL::StorageBufferType::RWTypedBuffer, verticesPos.data());
        CreateParticleBuffer(AttribNextPos,  LLGL::StorageBufferType::RWTypedBuffer, verticesPos.data());
        CreateParticleBuffer(AttribPrevPos,  LLGL::StorageBufferType::RWTypedBuffer, verticesPos.data(), &(vertexFormat.attributes[0]));
        CreateParticleBuffer(AttribVelocity, LLGL::StorageBufferType::RWTypedBuffer, zeroVectors.data());
        CreateParticleBuffer(AttribNormal,   LLGL::StorageBufferType::RWTypedBuffer, zeroVectors.data(), &(vertexFormat.attributes[1]));

        #ifdef ENABLE_STORAGE_TEXTURES

        // Create dummy vertex buffer
        LLGL::BufferDescriptor vbNullDesc;
        {
            vbNullDesc.debugName    = "Buffer.Null";
            vbNullDesc.size         = 1;
            vbNullDesc.bindFlags    = LLGL::BindFlags::VertexBuffer;
        }
        vertexBufferNull = renderer->CreateBuffer(vbNullDesc);

        #else

        // Create vertex buffer array for rendering
        LLGL::Buffer* const buffers[3] =
        {
            particleBuffers[AttribPrevPos], // Read "pos" from last written position (i.e. "prevPos") from last compute shader invocation
            particleBuffers[AttribNormal],  // Read "normal"
            particleBuffers[AttribBase]     // Read "texCoord" from .xy
        };
        vertexBufferArray = renderer->CreateBufferArray(3, buffers);
        vertexBufferArray->SetDebugName("BufferArray.Vertices");

        #endif

        // Create index buffer
        LLGL::BufferDescriptor indexBufferDesc;
        {
            indexBufferDesc.debugName   = "Buffer.Indices";
            indexBufferDesc.size        = sizeof(std::uint32_t) * indices.size();
            indexBufferDesc.bindFlags   = LLGL::BindFlags::IndexBuffer;
            indexBufferDesc.format      = LLGL::Format::R32UInt;
        }
        indexBuffer = renderer->CreateBuffer(indexBufferDesc, indices.data());
    }

    void CreateTexture()
    {
        // Load color map from file
        colorMap = LoadTexture("Logo_LLGL.png");
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
            computeShaders[0] = LoadShader({ LLGL::ShaderType::Compute, "Example.hlsl", "CSForces",             "cs_5_0" }, {}, {}, g_shaderMacros);
            computeShaders[1] = LoadShader({ LLGL::ShaderType::Compute, "Example.hlsl", "CSStretchConstraints", "cs_5_0" }, {}, {}, g_shaderMacros);
            computeShaders[2] = LoadShader({ LLGL::ShaderType::Compute, "Example.hlsl", "CSRelaxation",         "cs_5_0" }, {}, {}, g_shaderMacros);
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            computeShaders[0] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSForces.comp"             });
            computeShaders[1] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSStretchConstraints.comp" });
            computeShaders[2] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSRelaxation.comp"         });
        }
        else if (Supported(LLGL::ShadingLanguage::ESSL))
        {
            computeShaders[0] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSForces.comp",             "", "310 es" });
            computeShaders[1] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSStretchConstraints.comp", "", "310 es" });
            computeShaders[2] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSRelaxation.comp",         "", "310 es" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            computeShaders[0] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSForces.450core.comp.spv"             });
            computeShaders[1] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSStretchConstraints.450core.comp.spv" });
            computeShaders[2] = LoadShader({ LLGL::ShaderType::Compute, "Example.CSRelaxation.450core.comp.spv"         });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            computeShaders[0] = LoadShader({ LLGL::ShaderType::Compute, "Example.metal", "CSForces",             "2.0" });
            computeShaders[1] = LoadShader({ LLGL::ShaderType::Compute, "Example.metal", "CSStretchConstraints", "2.0" });
            computeShaders[2] = LoadShader({ LLGL::ShaderType::Compute, "Example.metal", "CSRelaxation",         "2.0" });
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create compute pipeline layout
        computeLayout = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "heap{"
                "cbuffer(SceneState@0):comp,"
                #ifdef ENABLE_STORAGE_TEXTURES
                "texture(parBase@1):comp,"
                "rwtexture(parCurrPos@2, parNextPos@3, parPrevPos@4, parVelocity@5, parNormal@6):comp,"
                #else
                "buffer(parBase@1):comp,"
                "rwbuffer(parCurrPos@2, parNextPos@3, parPrevPos@4, parVelocity@5, parNormal@6):comp,"
                "},"
                #endif // /ENABLE_STORAGE_TEXTURES
                "barriers{rwbuffer},"
            )
        );

        // Create resource heaps for compute pipeline
        const LLGL::ResourceViewDescriptor resourceViewsCompute[] =
        {
            constantBuffer,
            particleBuffers[AttribBase],
            particleBuffers[AttribCurrPos],
            particleBuffers[AttribNextPos],
            particleBuffers[AttribPrevPos],
            particleBuffers[AttribVelocity],
            particleBuffers[AttribNormal],

            constantBuffer,
            particleBuffers[AttribBase],
            particleBuffers[AttribNextPos], // Swap pos with next-pos
            particleBuffers[AttribCurrPos], // Swap next-pos with pos
            particleBuffers[AttribPrevPos],
            particleBuffers[AttribVelocity],
            particleBuffers[AttribNormal],
        };
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout     = computeLayout;
            resourceHeapDesc.numResourceViews   = sizeof(resourceViewsCompute) / sizeof(resourceViewsCompute[0]);
        }
        computeResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc, resourceViewsCompute);

        // Create compute pipeline
        const char* psoDebugNames[3] = { "CSForces.PSO", "CSStretchConstraints.PSO", "CSRelaxation.PSO" };
        for (int i = 0; i < 3; ++i)
        {
            LLGL::ComputePipelineDescriptor pipelineDesc;
            {
                pipelineDesc.debugName      = psoDebugNames[i];
                pipelineDesc.pipelineLayout = computeLayout;
                pipelineDesc.computeShader  = computeShaders[i];
            }
            computePipelines[i] = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(computePipelines[i]);
        }
    }

    void CreateGraphicsPipeline()
    {
        // Create graphics shader
        std::vector<LLGL::VertexFormat> usedVertexFormats;
        #ifndef ENABLE_STORAGE_TEXTURES
        usedVertexFormats = { vertexFormat };
        #endif
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            graphicsShaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" }, usedVertexFormats, {}, g_shaderMacros);
            graphicsShaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" }, {}, g_shaderMacros);
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL) || Supported(LLGL::ShadingLanguage::ESSL))
        {
            graphicsShaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.VS.vert" }, usedVertexFormats, {}, g_shaderMacros);
            graphicsShaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.PS.frag" }, {}, g_shaderMacros);
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            graphicsShaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.VS.450core.vert.spv" }, usedVertexFormats, {}, g_shaderMacros);
            graphicsShaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.PS.450core.frag.spv" }, {}, g_shaderMacros);
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            graphicsShaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VS", "2.0" }, usedVertexFormats, {}, g_shaderMacros);
            graphicsShaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", "PS", "2.0" }, {}, g_shaderMacros);
        }
        else
            throw std::runtime_error("shaders not available for selected renderer in this example");

        // Create graphics pipeline layout
        #ifdef ENABLE_STORAGE_TEXTURES

        graphicsLayout = renderer->CreatePipelineLayout(
            IsMetal() || IsVulkan()
                ? LLGL::Parse("heap{cbuffer(SceneState@3):vert:frag, texture(colorMap@4):frag, sampler(linearSampler@5):frag, texture(1,2,6):vert}, barriers{rwtexture}")
                : LLGL::Parse("heap{cbuffer(SceneState@0):vert:frag, texture(colorMap@0):frag, sampler(linearSampler@0):frag, texture(1,2,3):vert}, barriers{rwtexture}")
        );

        #else

        graphicsLayout = renderer->CreatePipelineLayout(
            IsMetal() || IsVulkan()
                ? LLGL::Parse("heap{cbuffer(SceneState@3):vert:frag, texture(colorMap@4):frag, sampler(linearSampler@5):frag},")
                : LLGL::Parse("heap{cbuffer(SceneState@0):vert:frag, texture(colorMap@0):frag, sampler(linearSampler@0):frag},")
        );

        #endif // /ENABLE_STORAGE_TEXTURES

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.debugName                      = "Scene.PSO";
            pipelineDesc.pipelineLayout                 = graphicsLayout;
            pipelineDesc.vertexShader                   = graphicsShaderPipeline.vs;
            pipelineDesc.fragmentShader                 = graphicsShaderPipeline.ps;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            #ifdef ENABLE_WIREFRAME
            pipelineDesc.rasterizer.polygonMode         = LLGL::PolygonMode::Wireframe;
            #endif
        }
        graphicsPipeline = renderer->CreatePipelineState(pipelineDesc);
        ReportPSOErrors(graphicsPipeline);

        // Create resource heaps for graphics pipeline
        const LLGL::ResourceViewDescriptor resourceViewsGraphics[] =
        {
            constantBuffer,
            colorMap,
            linearSampler,
            #ifdef ENABLE_STORAGE_TEXTURES
            particleBuffers[AttribBase],
            particleBuffers[AttribCurrPos],
            particleBuffers[AttribNormal],
            #endif
        };
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout     = graphicsLayout;
            resourceHeapDesc.numResourceViews   = sizeof(resourceViewsGraphics) / sizeof(resourceViewsGraphics[0]);
        }
        graphicsResourceHeap = renderer->CreateResourceHeap(resourceHeapDesc, resourceViewsGraphics);
    }

private:

    void UpdateScene()
    {
        // Update user input
        auto motion = input.GetMouseMotion();

        if (input.KeyPressed(LLGL::Key::LButton))
        {
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -90.0f, 90.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }

        if (input.KeyPressed(LLGL::Key::RButton))
        {
            float delta = motion.x*0.01f;
            stiffnessFactor = std::max(0.5f, std::min(stiffnessFactor + delta, 1.0f));
            LLGL::Log::Printf("stiffness: %d%%    \r", static_cast<int>(stiffnessFactor * 100.0f));
            ::fflush(stdout);
        }

        // Update timer
        timer.MeasureTime();
        sceneState.damping      = (1.0f - std::pow(10.0f, -dampingFactor));
        sceneState.dTime        = std::max(0.0001f, std::min(static_cast<float>(timer.GetDeltaTime()), 1.0f));
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
                commands->SetResourceHeap(*computeResourceHeap, swapBufferIndex);
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
                    commands->SetResourceHeap(*computeResourceHeap, swapBufferIndex);
                    commands->Dispatch(clothSegmentsU + 1, clothSegmentsV + 1, 1);
                }
            }
            commands->PopDebugGroup();

            // Run compute shader to adjust velocity of particles
            commands->PushDebugGroup("CSRelaxation");
            {
                commands->SetPipelineState(*computePipelines[CSRelaxation]);
                commands->SetResourceHeap(*computeResourceHeap, swapBufferIndex);
                commands->Dispatch(clothSegmentsU + 1, clothSegmentsV + 1, 1);
            }
            commands->PopDebugGroup();

            // Draw scene
            commands->BeginRenderPass(*swapChain);
            {
                // Clear color buffer and set viewport
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
                commands->SetViewport(swapChain->GetResolution());

                // Set vertex and index buffers
                #ifdef ENABLE_STORAGE_TEXTURES
                commands->SetVertexBuffer(*vertexBufferNull);
                #else
                commands->SetVertexBufferArray(*vertexBufferArray);
                #endif

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



