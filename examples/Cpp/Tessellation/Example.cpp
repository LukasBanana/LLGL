/*
 * Example.cpp (Example_Tessellation)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>


// Automatically rotate the model
//#define AUTO_ROTATE

// Use render pass to optimize attachment clearing
//#define ENABLE_RENDER_PASS


class Example_Tessellation : public ExampleBase
{

    ShaderPipeline          shaderPipeline;
    LLGL::PipelineState*    pipeline[2]         = { nullptr };

    LLGL::Buffer*           vertexBuffer        = nullptr;
    LLGL::Buffer*           indexBuffer         = nullptr;
    LLGL::Buffer*           constantBuffer      = nullptr;

    LLGL::PipelineLayout*   pipelineLayout      = nullptr;

    #ifdef ENABLE_RENDER_PASS
    LLGL::RenderPass*       renderPass          = nullptr;
    #endif

    std::uint32_t           constantBufferIndex = 0;

    bool                    showWireframe       = true;

    struct Settings
    {
        Gs::Matrix4f    wvpMatrix;
        float           tessLevelInner  = 5.0f;
        float           tessLevelOuter  = 5.0f;
        float           twist           = 0.0f;
        float           _pad0;                  // <-- padding for 16 byte pack alignment of constant buffers
    }
    settings;

public:

    Example_Tessellation() :
        ExampleBase( L"LLGL Example: Tessellation" )
    {
        // Check if constant buffers and tessellation shaders are supported
        const auto& renderCaps = renderer->GetRenderingCaps();

        if (!renderCaps.features.hasConstantBuffers)
            throw std::runtime_error("constant buffers are not supported by this renderer");
        if (!renderCaps.features.hasTessellatorStage)
            throw std::runtime_error("tessellation is not supported by this renderer");

        // Create graphics object
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        #ifdef ENABLE_RENDER_PASS
        CreateRenderPass();
        #endif
        CreatePipelines();

        // Print some information on the standard output
        LLGL::Log::Printf(
            "press LEFT MOUSE BUTTON and move mouse on X axis to increase/decrease inner tessellation\n"
            "press RIGHT MOUSE BUTTON and move mouse on X axis to increase/decrease outer tessellation\n"
            "press MIDDLE MOUSE BUTTON and move mouse on X axis to increase/decrease twist\n"
            "press TAB KEY to switch between wireframe modes\n"
        );
        ShowTessLevel();
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

        UpdateUserInput();

        // Create buffers for a simple 3D cube model
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeQuadIndices(), LLGL::Format::R32UInt);
        constantBuffer = CreateConstantBuffer(settings);

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
            constantBufferIndex = 1;//TODO: unify
        }
    }

    #ifdef ENABLE_RENDER_PASS
    void CreateRenderPass()
    {
        LLGL::RenderPassDescriptor renderPassDesc;
        {
            renderPassDesc.colorAttachments[0]  = LLGL::AttachmentFormatDescriptor{ swapChain->GetColorFormat(), LLGL::AttachmentLoadOp::Clear };
            renderPassDesc.depthAttachment      = LLGL::AttachmentFormatDescriptor{ swapChain->GetDepthStencilFormat(), LLGL::AttachmentLoadOp::Clear };
            renderPassDesc.samples              = GetMultiSampleDesc().SampleCount();
        }
        renderPass = renderer->CreateRenderPass(renderPassDesc);
    }
    #endif

    void CreatePipelines()
    {
        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor plDesc;
        {
            plDesc.bindings =
            {
                LLGL::BindingDescriptor
                {
                    "Settings",
                    LLGL::ResourceType::Buffer,
                    LLGL::BindFlags::ConstantBuffer,
                    (IsMetal() ? LLGL::StageFlags::ComputeStage | LLGL::StageFlags::VertexStage : LLGL::StageFlags::AllTessStages),
                    constantBufferIndex
                }
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
            #ifdef ENABLE_RENDER_PASS
            pipelineDesc.renderPass                     = renderPass;
            #else
            pipelineDesc.renderPass                     = swapChain->GetRenderPass();
            #endif
            pipelineDesc.pipelineLayout                 = pipelineLayout;

            // Set input-assembler state (draw patches with 4 control points with 32-bit indices)
            pipelineDesc.indexFormat                    = LLGL::Format::R32UInt;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::Patches4;

            // Enable multi-sample anti-aliasing
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            // Enable back-face culling
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;

            // Specify tessellation state (only required for Metal)
            pipelineDesc.tessellation.partition         = LLGL::TessellationPartition::FractionalOdd;
            pipelineDesc.tessellation.outputWindingCCW  = true;
        }

        // Create graphics pipelines
        pipeline[0] = renderer->CreatePipelineState(pipelineDesc);

        pipelineDesc.rasterizer.polygonMode = LLGL::PolygonMode::Wireframe;
        pipeline[1] = renderer->CreatePipelineState(pipelineDesc);
    }

    void ShowTessLevel()
    {
        /*LLGL::Log::Printf(
            "tessellation level (inner = %f, outer = %f)      \r",
            static_cast<double>(settings.tessLevelInner),
            static_cast<double>(settings.tessLevelOuter)
        );
        ::fflush(stdout);*/
    }

private:

    void UpdateUserInput()
    {
        // Tessellation level-of-detail limits
        static const float tessLevelMin = 1.0f, tessLevelMax = 64.0f;

        // Update tessellation levels by user input
        auto motion = input.GetMouseMotion().x;
        auto motionScaled = static_cast<float>(motion)*0.1f;

        if (input.KeyPressed(LLGL::Key::LButton))
        {
            settings.tessLevelInner += motionScaled;
            settings.tessLevelInner = Gs::Clamp(settings.tessLevelInner, tessLevelMin, tessLevelMax);
        }

        if (input.KeyPressed(LLGL::Key::RButton))
        {
            settings.tessLevelOuter += motionScaled;
            settings.tessLevelOuter = Gs::Clamp(settings.tessLevelOuter, tessLevelMin, tessLevelMax);
        }

        if ( motion != 0 && ( input.KeyPressed(LLGL::Key::LButton) || input.KeyPressed(LLGL::Key::RButton) ) )
            ShowTessLevel();

        if (input.KeyPressed(LLGL::Key::MButton))
            settings.twist += Gs::Deg2Rad(motionScaled);

        if (input.KeyDown(LLGL::Key::Tab))
            showWireframe = !showWireframe;

        // Update matrices
        Gs::Matrix4f worldMatrix;
        Gs::Translate(worldMatrix, Gs::Vector3f(0, 0, 5));

        settings.wvpMatrix = projection * worldMatrix;

        #ifdef AUTO_ROTATE
        static float rotation;
        rotation += 0.0025f;
        Gs::RotateFree(settings.worldMatrix, Gs::Vector3f(1, 1, 1).Normalized(), rotation);
        #endif
    }

    void DrawScene()
    {
        commands->Begin();
        {
            // Update constant buffer
            commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

            // Set hardware buffers to draw the model
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetIndexBuffer(*indexBuffer);

            // Set the swap-chain as the initial render target
            #ifdef ENABLE_RENDER_PASS
            commands->BeginRenderPass(*swapChain, renderPass);
            #else
            commands->BeginRenderPass(*swapChain);

            // Clear color- and depth buffers
            commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
            #endif
            {
                // Set viewport
                commands->SetViewport(swapChain->GetResolution());

                // Set graphics pipeline with the shader
                commands->SetPipelineState(*pipeline[showWireframe ? 1 : 0]);

                // Bind constant buffer to graphics pipeline
                commands->SetResource(0, *constantBuffer);

                // Draw tessellated quads with 24=4*6 vertices from patches of 4 control points
                commands->DrawIndexed(24, 0);
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



