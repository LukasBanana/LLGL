/*
 * Example.cpp (Example_StencilBuffer)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>


class Example_StencilBuffer : public ExampleBase
{

    LLGL::PipelineLayout*       pipelineLayout          = nullptr;
    LLGL::ResourceHeap*         resourceHeap            = {};

    LLGL::Shader*               vsScene                 = nullptr;
    LLGL::Shader*               fsScene                 = nullptr;
    LLGL::Shader*               vsStencil               = nullptr;

    LLGL::PipelineState*        pipelineScene           = nullptr;
    LLGL::PipelineState*        pipelineStencilWrite    = nullptr;
    LLGL::PipelineState*        pipelineStencilRead     = nullptr;

    LLGL::Buffer*               vertexBuffer            = nullptr;
    LLGL::Buffer*               constantBuffer          = nullptr;

    TriangleMesh                meshScene;
    TriangleMesh                meshPortal;
    TriangleMesh                meshObject1;
    TriangleMesh                meshObject2;

    Gs::Vector3f                objectPosition          = { 0, -1, 3 };
    float                       viewDistanceToCenter    = 8.0f;
    Gs::Vector2f                viewRotation;

    struct Settings
    {
        Gs::Matrix4f            wMatrix;
        Gs::Matrix4f            vpMatrix;
        Gs::Vector3f            lightDir                = Gs::Vector3f(-0.25f, -1.0f, 0.5f).Normalized();
        float                   _pad1;
        LLGL::ColorRGBAf        diffuse                 = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    settings;

public:

    Example_StencilBuffer() :
        ExampleBase { "LLGL Example: StencilBuffer" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreatePipelineLayouts();
        CreatePipelines();
        CreateResourceHeaps();

        #if 0
        // Show some information
        LLGL::Log::Printf(
            "press LEFT MOUSE BUTTON and move the mouse on the X-axis to rotate the OUTER cube\n"
            "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to rotate the INNER cube\n"
            "press RETURN KEY to save the render target texture to a PNG file\n"
        );
        #endif
    }

private:

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
        vertexFormat.SetStride(sizeof(TexturedVertex));

        // Load 3D models
        std::vector<TexturedVertex> vertices;
        meshScene   = LoadObjModel(vertices, "Portal-Scene.obj");
        meshPortal  = LoadObjModel(vertices, "Portal-Stencil.obj");
        meshObject1 = LoadObjModel(vertices, "WiredBox.obj");
        meshObject2 = LoadObjModel(vertices, "Pyramid.obj");

        meshObject1.color = { 0.2f, 0.9f, 0.1f };
        meshObject2.color = { 0.9f, 0.1f, 0.2f };

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VScene", "vs_5_0" }, { vertexFormat });
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PScene", "ps_5_0" });

            vsStencil = LoadShader({ LLGL::ShaderType::Vertex, "Example.hlsl", "VStencil", "vs_5_0" }, { vertexFormat });
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL) || Supported(LLGL::ShadingLanguage::ESSL))
        {
            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Scene.vert" }, { vertexFormat });
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Scene.frag" });

            vsStencil = LoadShader({ LLGL::ShaderType::Vertex, "Stencil.vert" }, { vertexFormat });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Scene.450core.vert.spv" }, { vertexFormat });
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Scene.450core.frag.spv" });

            vsStencil = LoadShader({ LLGL::ShaderType::Vertex, "Stencil.450core.vert.spv" }, { vertexFormat });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VScene", "1.1" }, { vertexFormat });
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", "PScene", "1.1" });

            vsStencil = LoadShader({ LLGL::ShaderType::Vertex, "Example.metal", "VStencil", "1.1" }, { vertexFormat });
        }
        else
            throw std::runtime_error("shaders not supported for active renderer");
    }

    void CreatePipelineLayouts()
    {
        // Create pipeline layouts for shadow-map and scene rendering
        pipelineLayout = renderer->CreatePipelineLayout(LLGL::Parse("heap{ cbuffer(Settings@1):frag:vert }"));
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for scene rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.vertexShader                   = vsScene;
                pipelineDesc.fragmentShader                 = fsScene;
                pipelineDesc.renderPass                     = swapChain->GetRenderPass();
                pipelineDesc.pipelineLayout                 = pipelineLayout;
                pipelineDesc.depth.testEnabled              = true;
                pipelineDesc.depth.writeEnabled             = true;
                pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            }
            pipelineScene = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(pipelineScene);
        }

        // Create graphics pipeline for stencil-write rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.vertexShader                   = vsStencil;
                pipelineDesc.renderPass                     = swapChain->GetRenderPass();
                pipelineDesc.pipelineLayout                 = pipelineLayout;
                pipelineDesc.depth.testEnabled              = true;                             // Read all depth bits
                pipelineDesc.depth.writeEnabled             = false;                            // Write no depth bits
                pipelineDesc.stencil.testEnabled            = true;                             // Enable stencil test, even though we only write the stencil bits
                pipelineDesc.stencil.front.depthPassOp      = LLGL::StencilOp::Replace;
                pipelineDesc.stencil.front.compareOp        = LLGL::CompareOp::AlwaysPass;
                pipelineDesc.stencil.front.reference        = 1;
                pipelineDesc.stencil.front.readMask         = 0u;                               // Read no stencil bits
                pipelineDesc.stencil.front.writeMask        = ~0u;                              // Write all stencil bits
                pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
                pipelineDesc.blend.targets[0].colorMask     = 0x0;                              // Write no color bits
            }
            pipelineStencilWrite = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(pipelineStencilWrite);
        }

        // Create graphics pipeline for stencil-read rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.vertexShader                   = vsScene;
                pipelineDesc.fragmentShader                 = fsScene;
                pipelineDesc.renderPass                     = swapChain->GetRenderPass();
                pipelineDesc.pipelineLayout                 = pipelineLayout;
                pipelineDesc.depth.testEnabled              = true;
                pipelineDesc.depth.writeEnabled             = true;                             // Write all depth bits
                pipelineDesc.stencil.testEnabled            = true;
                pipelineDesc.stencil.referenceDynamic       = true;                             // Change stencil reference independently of PSO
                pipelineDesc.stencil.front.compareOp        = LLGL::CompareOp::Equal;
                pipelineDesc.stencil.front.readMask         = ~0u;                              // Read all stencil bits
                pipelineDesc.stencil.front.writeMask        = 0u;                               // Write no stencil bits
                pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            }
            pipelineStencilRead = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(pipelineStencilRead);
        }
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for scene rendering
        resourceHeap = renderer->CreateResourceHeap(pipelineLayout, { constantBuffer });
    }

    void UpdateScene()
    {
        static float animation;

        // Update animation
        if (input.KeyPressed(LLGL::Key::LButton))
        {
            auto motion = input.GetMouseMotion();
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -45.0f, 0.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }

        if (input.KeyPressed(LLGL::Key::RButton))
        {
            auto motion = input.GetMouseMotion();
            animation += static_cast<float>(motion.x) * 0.25f;
        }

        // Update model transform
        meshObject1.transform.LoadIdentity();
        Gs::Translate(meshObject1.transform, objectPosition);
        Gs::RotateFree(meshObject1.transform, Gs::Vector3f{ 0, 1, 0 }, Gs::pi + Gs::Deg2Rad(animation));

        meshObject2.transform.LoadIdentity();
        Gs::Translate(meshObject2.transform, objectPosition);
        Gs::RotateFree(meshObject2.transform, Gs::Vector3f{ 0, 1, 0 }, Gs::pi + Gs::Deg2Rad(animation));

        // Update view transformation
        settings.vpMatrix.LoadIdentity();
        //Gs::RotateFree(settings.vpMatrix, { 1, 0, 0 }, Gs::pi*0.5f);
        Gs::RotateFree(settings.vpMatrix, { 0, 1, 0 }, Gs::Deg2Rad(viewRotation.y));
        Gs::RotateFree(settings.vpMatrix, { 1, 0, 0 }, Gs::Deg2Rad(viewRotation.x));
        Gs::Translate(settings.vpMatrix, { 0, 0, -viewDistanceToCenter });
        settings.vpMatrix.MakeInverse();
        settings.vpMatrix = projection * settings.vpMatrix;
    }

    void RenderMesh(const TriangleMesh& mesh)
    {
        settings.wMatrix = mesh.transform;
        settings.diffuse = mesh.color;
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));
        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderScene()
    {
        // Clear entire framebuffer, i.e. color, depth, and stencil buffers
        commands->Clear(LLGL::ClearFlags::All, backgroundColor);

        // Render scene background
        commands->SetPipelineState(*pipelineScene);
        commands->SetResourceHeap(*resourceHeap);
        RenderMesh(meshScene);
    }

    void RenderPortalStencil()
    {
        // Render portal stencil (no color is written)
        commands->SetPipelineState(*pipelineStencilWrite);
        RenderMesh(meshPortal);
    }

    void RenderSceneBetweenPortal()
    {
        commands->SetPipelineState(*pipelineStencilRead);

        // Render scene objects outside portal (stencil = 0)
        commands->SetStencilReference(0);
        RenderMesh(meshObject1);

        // Render scene objects inside portal (stencil = 1)
        commands->SetStencilReference(1);
        RenderMesh(meshObject2);
    }

    void OnDrawFrame() override
    {
        // Update scene by user input
        UpdateScene();

        commands->Begin();
        {
            // Bind common input assembly
            commands->SetVertexBuffer(*vertexBuffer);

            // Render everything directly into the swap-chain
            commands->BeginRenderPass(*swapChain);
            {
                commands->SetViewport(swapChain->GetResolution());

                // Draw scene, then draw the portal into the stencil buffer, and finally draw the hidden object inside the portal
                commands->PushDebugGroup("Scene Pass (Render Background)");
                RenderScene();
                commands->PopDebugGroup();

                commands->PushDebugGroup("Stencil Write Pass (Render Portal)");
                RenderPortalStencil();
                commands->PopDebugGroup();

                commands->PushDebugGroup("Stencil Read Pass (Between Portal)");
                RenderSceneBetweenPortal();
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_StencilBuffer);



