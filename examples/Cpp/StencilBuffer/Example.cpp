/*
 * Example.cpp (Example_StencilBuffer)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>


class Example_StencilBuffer : public ExampleBase
{

    LLGL::PipelineLayout*       pipelineLayout              = nullptr;

    LLGL::Shader*               vsScene                     = nullptr;
    LLGL::Shader*               fsScene                     = nullptr;
    LLGL::Shader*               fsReflection                = nullptr;
    LLGL::Shader*               vsStencil                   = nullptr;

    LLGL::PipelineState*        psoBaseScene                = nullptr;
    LLGL::PipelineState*        psoWriteVisibleStencil      = nullptr;
    LLGL::PipelineState*        psoWriteInvisibleStencil    = nullptr; // Renders a stencil without emitting pixel colors
    LLGL::PipelineState*        psoPortalScene              = nullptr;
    LLGL::PipelineState*        psoPlanarReflection         = nullptr;

    LLGL::Buffer*               vertexBuffer                = nullptr;
    LLGL::Buffer*               viewCbuffer                 = nullptr;

    TriangleMesh                meshFloor;
    TriangleMesh                meshFrame;
    TriangleMesh                meshPortal;
    TriangleMesh                meshObject1;
    TriangleMesh                meshObject2;

    float                       viewDistanceToCenter    = 8.0f;
    Gs::Vector2f                viewRotation;

    struct alignas(16) SceneView
    {
        Gs::Matrix4f        wMatrix;
        Gs::Matrix4f        vpMatrix;
        Gs::Vector3f        lightDir            = Gs::Vector3f(-0.25f, -1.0f, 0.5f).Normalized();
        float               reflectionFading    = 0.8f;
        LLGL::ColorRGBAf    diffuse             = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    sceneView;

public:

    Example_StencilBuffer() :
        ExampleBase { "LLGL Example: StencilBuffer" }
    {
        // Create all graphics objects
        CreateBuffers();
        LoadShaders();
        CreatePipelines();

        // Update vectors for projection
        sceneView.lightDir.z *= GetProjectionZAxis();
    }

private:

    void CreateBuffers()
    {
        // Load 3D models
        std::vector<TexturedVertex> vertices;
        meshFloor   = Load3DModel(vertices, "Portal-Floor.obj");
        meshFrame   = Load3DModel(vertices, "Portal-Frame.obj");
        meshPortal  = Load3DModel(vertices, "Portal-Stencil.obj");
        meshObject1 = Load3DModel(vertices, "WiredBox.obj");
        meshObject2 = Load3DModel(vertices, "Pyramid.obj");

        meshObject1.color = { 0.2f, 0.9f, 0.1f };
        meshObject2.color = { 0.9f, 0.1f, 0.2f };

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, sizeof(TexturedVertex));
        viewCbuffer = CreateConstantBuffer(sceneView);
    }

    void LoadShaders()
    {
        // Load shader program
        vsScene = LoadVertexShader("Example", "VSScene");
        fsScene = LoadFragmentShader("Example", "PSScene");
        fsReflection = LoadFragmentShader("Example", "PSReflection");
        vsStencil = LoadVertexShader("Example", "VSStencil");
    }

    void CreatePipelines()
    {
        // Create pipeline layouts for shadow-map and scene rendering
        pipelineLayout = renderer->CreatePipelineLayout(
            LLGL::Parse( "cbuffer(SceneView@1):frag:vert" )
        );

        // Specify vertex format
        const LLGL::VertexAttribute vertexAttribs[] =
        {
            LLGL::VertexAttribute{ "position", LLGL::Format::RGB32Float, 0, offsetof(TexturedVertex, position), sizeof(TexturedVertex) },
            LLGL::VertexAttribute{ "normal",   LLGL::Format::RGB32Float, 1, offsetof(TexturedVertex, normal  ), sizeof(TexturedVertex) },
        };

        // Create graphics PSO for rendering the base scene
        {
            LLGL::GraphicsPipelineDescriptor psoDesc;
            {
                psoDesc.debugName                       = "BaseScene.PSO";
                psoDesc.inputVertexAttribs              = vertexAttribs;
                psoDesc.vertexShader                    = vsScene;
                psoDesc.fragmentShader                  = fsScene;
                psoDesc.renderPass                      = swapChain->GetRenderPass();
                psoDesc.pipelineLayout                  = pipelineLayout;
                psoDesc.depth.testEnabled               = true;
                psoDesc.depth.writeEnabled              = true;
                psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
                psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
            }
            psoBaseScene = renderer->CreatePipelineState(psoDesc);
            ReportPSOErrors(psoBaseScene);
        }

        // Create graphics PSO for rendering the scene + writing stencil values
        {
            LLGL::GraphicsPipelineDescriptor psoDesc;
            {
                psoDesc.debugName                       = "WriteVisibleStencil.PSO";
                psoDesc.inputVertexAttribs              = vertexAttribs;
                psoDesc.vertexShader                    = vsScene;
                psoDesc.fragmentShader                  = fsScene;
                psoDesc.renderPass                      = swapChain->GetRenderPass();
                psoDesc.pipelineLayout                  = pipelineLayout;
                psoDesc.depth.testEnabled               = true;
                psoDesc.depth.writeEnabled              = false;                            // Don't write depth so we can render the planar reflections more easily
                psoDesc.stencil.testEnabled             = true;
                psoDesc.stencil.referenceDynamic        = true;                             // Change stencil reference independently of PSO
                psoDesc.stencil.front.depthPassOp       = LLGL::StencilOp::Replace;
                psoDesc.stencil.front.compareOp         = LLGL::CompareOp::AlwaysPass;
                psoDesc.stencil.front.readMask          = 0u;                               // Read no stencil bits
                psoDesc.stencil.front.writeMask         = ~0u;                              // Write all stencil bits
                psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
                psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
            }
            psoWriteVisibleStencil = renderer->CreatePipelineState(psoDesc);
            ReportPSOErrors(psoWriteVisibleStencil);
        }

        // Create graphics pipeline for stencil-write rendering
        {
            LLGL::GraphicsPipelineDescriptor psoDesc;
            {
                psoDesc.debugName                       = "WriteInvisibleStencil.PSO";
                psoDesc.inputVertexAttribs              = vertexAttribs;
                psoDesc.vertexShader                    = vsStencil;
                psoDesc.renderPass                      = swapChain->GetRenderPass();
                psoDesc.pipelineLayout                  = pipelineLayout;
                psoDesc.depth.testEnabled               = true;                             // Read all depth bits
                psoDesc.depth.writeEnabled              = false;                            // Write no depth bits
                psoDesc.stencil.testEnabled             = true;                             // Enable stencil test, even though we only write the stencil bits
                psoDesc.stencil.front.depthPassOp       = LLGL::StencilOp::Replace;
                psoDesc.stencil.front.reference         = 1;
                psoDesc.stencil.front.compareOp         = LLGL::CompareOp::AlwaysPass;
                psoDesc.stencil.front.readMask          = 0u;                               // Read no stencil bits
                psoDesc.stencil.front.writeMask         = ~0u;                              // Write all stencil bits
                psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
                psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
                psoDesc.blend.targets[0].colorMask      = 0x0;                              // Write no color bits
            }
            psoWriteInvisibleStencil = renderer->CreatePipelineState(psoDesc);
            ReportPSOErrors(psoWriteInvisibleStencil);
        }

        // Create graphics PSO for rendering scene objects between the portal
        {
            LLGL::GraphicsPipelineDescriptor psoDesc;
            {
                psoDesc.debugName                       = "SceneBetweenPortal.PSO";
                psoDesc.inputVertexAttribs              = vertexAttribs;
                psoDesc.vertexShader                    = vsScene;
                psoDesc.fragmentShader                  = fsScene;
                psoDesc.renderPass                      = swapChain->GetRenderPass();
                psoDesc.pipelineLayout                  = pipelineLayout;
                psoDesc.depth.testEnabled               = true;
                psoDesc.depth.writeEnabled              = true;                             // Write all depth bits
                psoDesc.stencil.testEnabled             = true;
                psoDesc.stencil.referenceDynamic        = true;                             // Change stencil reference independently of PSO
                psoDesc.stencil.front.compareOp         = LLGL::CompareOp::Equal;
                psoDesc.stencil.front.readMask          = ~0u;                              // Read all stencil bits
                psoDesc.stencil.front.writeMask         = 0u;                               // Write no stencil bits
                psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
                psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
            }
            psoPortalScene = renderer->CreatePipelineState(psoDesc);
            ReportPSOErrors(psoPortalScene);
        }

        // Create graphics PSO for rendering planar reflections between the portal
        {
            LLGL::GraphicsPipelineDescriptor psoDesc;
            {
                psoDesc.debugName                       = "PlanarReflection.PSO";
                psoDesc.inputVertexAttribs              = vertexAttribs;
                psoDesc.vertexShader                    = vsScene;
                psoDesc.fragmentShader                  = fsReflection;
                psoDesc.renderPass                      = swapChain->GetRenderPass();
                psoDesc.pipelineLayout                  = pipelineLayout;
                psoDesc.depth.testEnabled               = true;
                psoDesc.depth.writeEnabled              = true;                             // Write all depth bits
                psoDesc.stencil.testEnabled             = true;
                psoDesc.stencil.referenceDynamic        = true;                             // Change stencil reference independently of PSO
                psoDesc.stencil.front.compareOp         = LLGL::CompareOp::Equal;
                psoDesc.stencil.front.readMask          = ~0u;                              // Read all stencil bits
                psoDesc.stencil.front.writeMask         = 0u;                               // Write no stencil bits
                psoDesc.rasterizer.cullMode             = LLGL::CullMode::Back;
                psoDesc.rasterizer.multiSampleEnabled   = (GetSampleCount() > 1);
            }
            psoPlanarReflection = renderer->CreatePipelineState(psoDesc);
            ReportPSOErrors(psoPlanarReflection);
        }
    }

    void UpdateScene()
    {
        static float animation;

        const float projZAxis = GetProjectionZAxis();

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
        const Gs::Vector3f objectPosition{ 0, -1, 3 * projZAxis };

        meshObject1.transform.LoadIdentity();
        Gs::Translate(meshObject1.transform, objectPosition);
        Gs::RotateFree(meshObject1.transform, Gs::Vector3f{ 0, 1, 0 }, Gs::pi + Gs::Deg2Rad(animation * projZAxis));

        meshObject2.transform.LoadIdentity();
        Gs::Translate(meshObject2.transform, objectPosition);
        Gs::RotateFree(meshObject2.transform, Gs::Vector3f{ 0, 1, 0 }, Gs::pi + Gs::Deg2Rad(animation * projZAxis));

        // Update view transformation
        sceneView.vpMatrix.LoadIdentity();
        //Gs::RotateFree(sceneView.vpMatrix, { 1, 0, 0 }, Gs::pi*0.5f);
        Gs::RotateFree(sceneView.vpMatrix, { 0, 1, 0 }, Gs::Deg2Rad(viewRotation.y * projZAxis));
        Gs::RotateFree(sceneView.vpMatrix, { 1, 0, 0 }, Gs::Deg2Rad(viewRotation.x * projZAxis));
        Gs::Translate(sceneView.vpMatrix, { 0, 0, -viewDistanceToCenter * projZAxis });
        sceneView.vpMatrix.MakeInverse();
        sceneView.vpMatrix = projection * sceneView.vpMatrix;
    }

    void RenderMesh(const TriangleMesh& mesh)
    {
        sceneView.wMatrix = mesh.transform;
        sceneView.diffuse = mesh.color;

        commands->UpdateBuffer(*viewCbuffer, 0, &sceneView, sizeof(sceneView));
        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderMeshReflection(const TriangleMesh& mesh, float centerPosY)
    {
        sceneView.wMatrix = mesh.transform;
        Gs::Scale(sceneView.wMatrix, Gs::Vector3f{ -1.0f, -1.0f, 1.0f });
        Gs::Translate(sceneView.wMatrix, Gs::Vector3f{ 0.0f, centerPosY*2.0f, 0.0f });

        sceneView.diffuse = mesh.color;

        commands->UpdateBuffer(*viewCbuffer, 0, &sceneView, sizeof(sceneView));
        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderBaseScene()
    {
        // Clear entire framebuffer, i.e. color, depth, and stencil buffers
        commands->Clear(LLGL::ClearFlags::All, backgroundColor);

        // Render scene background
        commands->SetPipelineState(*psoWriteVisibleStencil);
        commands->SetResource(0, *viewCbuffer);

        commands->SetStencilReference(0);
        RenderMesh(meshFloor);

        commands->SetPipelineState(*psoBaseScene);
        commands->SetResource(0, *viewCbuffer);
        RenderMesh(meshFrame);
    }

    void RenderPortalStencil()
    {
        // Render portal stencil (no color is written)
        commands->SetPipelineState(*psoWriteInvisibleStencil);
        RenderMesh(meshPortal);

        RenderMeshReflection(meshPortal, 2.0f);
    }

    void RenderSceneBetweenPortal()
    {
        commands->SetPipelineState(*psoPortalScene);
        commands->SetResource(0, *viewCbuffer);

        // Render scene objects outside portal (stencil = 0)
        commands->SetStencilReference(0);
        RenderMesh(meshObject1);

        // Render scene objects inside portal (stencil = 1)
        commands->SetStencilReference(1);
        RenderMesh(meshObject2);
    }

    void RenderPlanarReflectionBetweenPortal()
    {
        commands->SetPipelineState(*psoPlanarReflection);
        commands->SetResource(0, *viewCbuffer);

        // Render scene objects outside portal (stencil = 0)
        // Frame must be visible inside and outside the portal
        commands->SetStencilReference(0);
        RenderMeshReflection(meshObject1, 1.0f);
        RenderMeshReflection(meshFrame, 2.0f);

        // Render scene objects inside portal (stencil = 1)
        commands->SetStencilReference(1);
        RenderMeshReflection(meshObject2, 1.0f);
        RenderMeshReflection(meshFrame, 2.0f);
    }

    void OnDrawFrame(float dt) override
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
                RenderBaseScene();
                commands->PopDebugGroup();

                commands->PushDebugGroup("Stencil Write Pass (Render Portal)");
                RenderPortalStencil();
                commands->PopDebugGroup();

                commands->PushDebugGroup("Stencil Read Pass (Between Portal)");
                RenderSceneBetweenPortal();
                commands->PopDebugGroup();

                commands->PushDebugGroup("Stencil Read Pass (Planar Reflection)");
                RenderPlanarReflectionBetweenPortal();
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_StencilBuffer);



