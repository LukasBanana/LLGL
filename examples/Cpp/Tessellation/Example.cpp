/*
 * Example.cpp (Example_Tessellation)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>


class Example_Tessellation : public ExampleBase
{

    static constexpr float  maxHeightFactor     = 0.2f;

    enum class ViewModes
    {
        SolidOnly,
        MixedSolidAndWireframe,
        WireframeOnly,
    };

    ShaderPipeline          shaderPipeline;
    LLGL::PipelineState*    pipeline[2]         = { nullptr };

    LLGL::Buffer*           vertexBuffer        = nullptr;
    LLGL::Buffer*           indexBuffer         = nullptr;
    LLGL::Buffer*           sceneBuffer         = nullptr;

    LLGL::Sampler*          linearSampler       = nullptr;

    LLGL::Texture*          colorMap            = nullptr;
    LLGL::Texture*          normalMap           = nullptr;
    LLGL::Texture*          specularMap         = nullptr;
    LLGL::Texture*          heightMap           = nullptr;

    LLGL::PipelineLayout*   pipelineLayout      = nullptr;

    ViewModes               viewMode            = ViewModes::SolidOnly;
    float                   maxTessFactor       = 1.0f;

    TriangleMesh            model;

    struct Scene
    {
        Gs::Matrix4f    vpMatrix;       // View-projection matrix to transform coordinates from world-space into clipping-space
        Gs::Matrix4f    vMatrix;        // View matrix to transform coordinates from world-space into view-space
        Gs::Matrix4f    wMatrix;        // World matrix to transform coordinates from model-space into world-space
        Gs::Vector3f    lightVec        = { 0.0f, 0.0f, -1.0f };
        float           texScale        = 0.5f;
        float           tessLevelInner  = 64.0f;
        float           tessLevelOuter  = 64.0f;
        float           heightFactor    = 0.2f;
        float           shininessPower  = 90.0f;
    }
    scene;

public:

    Example_Tessellation() :
        ExampleBase( L"LLGL Example: Tessellation" )
    {
        // Check if constant buffers and tessellation shaders are supported
        const auto& renderCaps = renderer->GetRenderingCaps();

        LLGL_VERIFY(renderCaps.features.hasConstantBuffers);
        LLGL_VERIFY(renderCaps.features.hasTessellatorStage);

        // Limit initial tessellation factor
        maxTessFactor = static_cast<float>(renderCaps.limits.maxTessFactor);
        SetTessellationFactor(scene.tessLevelInner, scene.tessLevelOuter);

        // Create graphics object
        LLGL::VertexFormat vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreateTextures();
        CreatePipelines();

        // Print some information on the standard output
        LLGL::Log::Printf(
            "press LEFT MOUSE BUTTON to rotate the model\n"
            "press RIGHT MOUSE BUTTON and move mouse on X axis to change tessellation factors (max = %u)\n"
            "press TAB KEY to switch between wireframe modes\n",
            renderCaps.limits.maxTessFactor
        );
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position",  LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "normal",    LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "tangent",   LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "bitangent", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "texCoord",  LLGL::Format::RG32Float  });

        // Load cube model with minor pre-tessellation.
        // A cube with only 8 vertices would only allow a rough tessellation depending on the displacement map.
        std::vector<TexturedVertex> texuturedVertices;
        model = LoadObjModel(texuturedVertices, "UVCube2x.obj", 4);

        // Create buffers for a simple 3D cube model
        //auto texuturedVertices = GenerateTexturedCubeVertices();
        vertexBuffer = CreateVertexBuffer(GenerateTangentSpaceQuadVertices(texuturedVertices), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateTexturedCubeQuadIndices(model.numVertices, model.firstVertex), LLGL::Format::R32UInt);
        sceneBuffer = CreateConstantBuffer(scene);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            shaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,         "Example.vert" }, { vertexFormat });
            shaderPipeline.hs = LoadShader({ LLGL::ShaderType::TessControl,    "Example.tesc" });
            shaderPipeline.ds = LoadShader({ LLGL::ShaderType::TessEvaluation, "Example.tese" });
            shaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment,       "Example.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,         "Example.450core.vert.spv" }, { vertexFormat });
            shaderPipeline.hs = LoadShader({ LLGL::ShaderType::TessControl,    "Example.450core.tesc.spv" });
            shaderPipeline.ds = LoadShader({ LLGL::ShaderType::TessEvaluation, "Example.450core.tese.spv" });
            shaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment,       "Example.450core.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            shaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,         "Example.hlsl", "VS", "vs_5_0" }, { vertexFormat });
            shaderPipeline.hs = LoadShader({ LLGL::ShaderType::TessControl,    "Example.hlsl", "HS", "hs_5_0" });
            shaderPipeline.ds = LoadShader({ LLGL::ShaderType::TessEvaluation, "Example.hlsl", "DS", "ds_5_0" });
            shaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment,       "Example.hlsl", "PS", "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            shaderPipeline.hs = LoadShader({ LLGL::ShaderType::Compute,        "Example.metal", "HS", "2.0" });
            shaderPipeline.ds = LoadShader({ LLGL::ShaderType::Vertex,         "Example.metal", "DS", "2.0" }, { vertexFormat });
            shaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment,       "Example.metal", "PS", "2.0" });
        }
    }

    void CreateTextures()
    {
        // Create default sampler state
        linearSampler = renderer->CreateSampler({});

        // Load textures
        const long texBindFlags = (LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment);
        colorMap    = LoadTexture("PBR/rocks/gray_rocks_diff_1k.jpg",   texBindFlags, LLGL::Format::RGBA8UNorm);
        normalMap   = LoadTexture("PBR/rocks/gray_rocks_nor_gl_1k.png", texBindFlags, LLGL::Format::RGBA8UNorm);
        specularMap = LoadTexture("PBR/rocks/gray_rocks_spec_1k.jpg",   texBindFlags, LLGL::Format::R8UNorm   );
        heightMap   = LoadTexture("PBR/rocks/gray_rocks_disp_1k.png",   texBindFlags, LLGL::Format::R8UNorm   );
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        constexpr long vertStage    = LLGL::StageFlags::VertexStage;
        constexpr long fragStage    = LLGL::StageFlags::FragmentStage;
        const long tessEvalStage    = (IsMetal() ? LLGL::StageFlags::VertexStage : LLGL::StageFlags::TessEvaluationStage);
        const long allStages        = fragStage | vertStage | (IsMetal() ? LLGL::StageFlags::ComputeStage : LLGL::StageFlags::AllTessStages);

        LLGL::PipelineLayoutDescriptor plDesc;
        {
            plDesc.bindings =
            {
                LLGL::BindingDescriptor{ "Scene",         LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, allStages,                 1 },
                LLGL::BindingDescriptor{ "linearSampler", LLGL::ResourceType::Sampler, 0,                               fragStage | tessEvalStage, 2 },
                LLGL::BindingDescriptor{ "colorMap",      LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        fragStage,                 3 },
                LLGL::BindingDescriptor{ "normalMap",     LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        fragStage,                 4 },
                LLGL::BindingDescriptor{ "specularMap",   LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        fragStage,                 5 },
                LLGL::BindingDescriptor{ "heightMap",     LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        tessEvalStage,             6 },
            };
            plDesc.combinedTextureSamplers =
            {
                LLGL::CombinedTextureSamplerDescriptor{ "colorMap",    "colorMap",    "linearSampler", 3 },
                LLGL::CombinedTextureSamplerDescriptor{ "normalMap",   "normalMap",   "linearSampler", 4 },
                LLGL::CombinedTextureSamplerDescriptor{ "specularMap", "specularMap", "linearSampler", 5 },
                LLGL::CombinedTextureSamplerDescriptor{ "heightMap",   "heightMap",   "linearSampler", 6 },
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(plDesc);

        // Setup graphics pipeline descriptors
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            // Set references to shader program, render pass, and pipeline layout
            pipelineDesc.vertexShader                   = shaderPipeline.vs;
            pipelineDesc.tessControlShader              = shaderPipeline.hs;
            pipelineDesc.tessEvaluationShader           = shaderPipeline.ds;
            pipelineDesc.fragmentShader                 = shaderPipeline.ps;
            pipelineDesc.renderPass                     = swapChain->GetRenderPass();
            pipelineDesc.pipelineLayout                 = pipelineLayout;

            // Set input-assembler state (draw patches with 4 control points with 32-bit indices)
            pipelineDesc.indexFormat                    = LLGL::Format::R32UInt;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::Patches4;

            // Enable multi-sample anti-aliasing
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            // Enable back-face culling and scissor test to show multiple views in the same viewport
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            pipelineDesc.rasterizer.scissorTestEnabled  = true;

            // Specify tessellation state (only required for Metal)
            pipelineDesc.tessellation.partition         = LLGL::TessellationPartition::FractionalOdd;
            pipelineDesc.tessellation.outputWindingCCW  = true;
        }

        // Create graphics pipelines
        pipeline[0] = renderer->CreatePipelineState(pipelineDesc);

        pipelineDesc.rasterizer.polygonMode = LLGL::PolygonMode::Wireframe;
        pipeline[1] = renderer->CreatePipelineState(pipelineDesc);
    }

private:

    void SetTessellationFactor(float inner, float outer)
    {
        scene.tessLevelInner = Gs::Clamp(inner, 1.0f, maxTessFactor);
        scene.tessLevelOuter = Gs::Clamp(outer, 1.0f, maxTessFactor);
        scene.heightFactor = Gs::SmoothStep(scene.tessLevelInner/maxTessFactor) * maxHeightFactor;
    }

    void UpdateUserInput()
    {
        // Update tessellation levels by user input
        const LLGL::Offset2D motion = input.GetMouseMotion();
        const float deltaX = static_cast<float>(motion.x)*0.1f;
        const float deltaY = static_cast<float>(motion.y)*0.1f;

        static Gs::Quaternionf rotation = Rotation(Gs::Deg2Rad(-20.0f), 0.0f);
        if (input.KeyPressed(LLGL::Key::LButton))
            RotateModel(rotation, deltaX*0.05f, deltaY*0.05f);

        if (input.KeyPressed(LLGL::Key::RButton))
            SetTessellationFactor(scene.tessLevelInner + deltaX, scene.tessLevelOuter + deltaX);

        if (input.KeyDown(LLGL::Key::Tab))
            viewMode = static_cast<ViewModes>((static_cast<int>(viewMode) + 1) % 3);

        // Update matrices
        scene.vMatrix.LoadIdentity();
        Gs::Translate(scene.vMatrix, Gs::Vector3f(0, 0, -5));
        scene.vMatrix.MakeInverse();

        scene.vpMatrix = projection * scene.vMatrix;

        scene.wMatrix.LoadIdentity();
        Gs::QuaternionToMatrix(scene.wMatrix, rotation);
    }

    void DrawTessellatedModel(bool showWireframe)
    {
        commands->SetPipelineState(*pipeline[showWireframe ? 1 : 0]);

        // Bind constant buffer to graphics pipeline
        commands->SetResource(0, *sceneBuffer);
        commands->SetResource(1, *linearSampler);
        commands->SetResource(2, *colorMap);
        commands->SetResource(3, *normalMap);
        commands->SetResource(4, *specularMap);
        commands->SetResource(5, *heightMap);

        // Draw tessellated quads. A cube with 6 quads will have 6*4=24 vertices from patches of 4 control points
        commands->DrawIndexed(model.numVertices, 0);
    }

    void DrawScene()
    {
        commands->Begin();
        {
            // Update constant buffer
            commands->UpdateBuffer(*sceneBuffer, 0, &scene, sizeof(scene));

            // Set hardware buffers to draw the model
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetIndexBuffer(*indexBuffer);

            // Set the swap-chain as the initial render target
            commands->BeginRenderPass(*swapChain);
            {
                // Clear color- and depth buffers
                commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);

                // Set viewport and graphics PSO
                const LLGL::Extent2D resolution = swapChain->GetResolution();
                commands->SetViewport(resolution);

                switch (viewMode)
                {
                    case ViewModes::SolidOnly:
                    {
                        commands->SetScissor(LLGL::Scissor{ LLGL::Offset2D{}, resolution });
                        DrawTessellatedModel(false);
                    }
                    break;

                    case ViewModes::MixedSolidAndWireframe:
                    {
                        const LLGL::Extent2D halfResolution{ resolution.width/2, resolution.height };

                        commands->SetScissor(LLGL::Scissor{ LLGL::Offset2D{}, halfResolution });
                        DrawTessellatedModel(false);

                        commands->SetScissor(LLGL::Scissor{ LLGL::Offset2D{ static_cast<std::int32_t>(resolution.width/2), 0 }, halfResolution });
                        DrawTessellatedModel(true);
                    }
                    break;

                    case ViewModes::WireframeOnly:
                    {
                        commands->SetScissor(LLGL::Scissor{ LLGL::Offset2D{}, resolution });
                        DrawTessellatedModel(true);
                    }
                    break;
                }
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

    void OnDrawFrame() override
    {
        UpdateUserInput();
        DrawScene();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Tessellation);



