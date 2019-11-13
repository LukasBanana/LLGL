/*
 * Example.cpp (Example_StencilBuffer)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


class Example_StencilBuffer : public ExampleBase
{

    LLGL::PipelineLayout*       pipelineLayout          = nullptr;
    LLGL::ResourceHeap*         resourceHeap            = {};

    LLGL::ShaderProgram*        shaderProgramScene      = nullptr;
    LLGL::ShaderProgram*        shaderProgramStencil    = nullptr;

    LLGL::PipelineState*        pipelineScene           = {};
    LLGL::PipelineState*        pipelineStencilWrite    = {};
    LLGL::PipelineState*        pipelineStencilRead     = {};

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
        ExampleBase { L"LLGL Example: StencilBuffer" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreatePipelineLayouts();
        CreatePipelines();
        CreateResourceHeaps();

        commands->SetClearColor(backgroundColor);

        #if 0
        // Show some information
        std::cout << "press LEFT MOUSE BUTTON and move the mouse on the X-axis to rotate the OUTER cube" << std::endl;
        std::cout << "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to rotate the INNER cube" << std::endl;
        std::cout << "press RETURN KEY to save the render target texture to a PNG file" << std::endl;
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
        meshScene   = LoadObjModel(vertices, "../../Media/Models/Portal-Scene.obj");
        meshPortal  = LoadObjModel(vertices, "../../Media/Models/Portal-Stencil.obj");
        meshObject1 = LoadObjModel(vertices, "../../Media/Models/WiredBox.obj");
        meshObject2 = LoadObjModel(vertices, "../../Media/Models/Pyramid.obj");

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
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VScene", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "Example.hlsl", "PScene", "ps_5_0" },
                },
                { vertexFormat }
            );
            shaderProgramStencil = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "Example.hlsl", "VStencil", "vs_5_0" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Scene.vert" },
                    { LLGL::ShaderType::Fragment, "Scene.frag" },
                },
                { vertexFormat }
            );
            shaderProgramStencil = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "Stencil.vert" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Scene.450core.vert.spv" },
                    { LLGL::ShaderType::Fragment, "Scene.450core.frag.spv" },
                },
                { vertexFormat }
            );
            shaderProgramStencil = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "Stencil.450core.vert.spv" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.metal", "VScene", "1.1" },
                    { LLGL::ShaderType::Fragment, "Example.metal", "PScene", "1.1" },
                },
                { vertexFormat }
            );
            shaderProgramStencil = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "Example.metal", "VStencil", "1.1" }
                },
                { vertexFormat }
            );
        }
        else
            throw std::runtime_error("shaders not supported for active renderer");
    }

    void CreatePipelineLayouts()
    {
        // Create pipeline layouts for shadow-map and scene rendering
        pipelineLayout = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(Settings@1):frag:vert")
        );
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for scene rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram                  = shaderProgramScene;
                pipelineDesc.renderPass                     = context->GetRenderPass();
                pipelineDesc.pipelineLayout                 = pipelineLayout;
                pipelineDesc.depth.testEnabled              = true;
                pipelineDesc.depth.writeEnabled             = true;
                pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            }
            pipelineScene = renderer->CreatePipelineState(pipelineDesc);
        }

        // Create graphics pipeline for stencil-write rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram                  = shaderProgramStencil;
                pipelineDesc.renderPass                     = context->GetRenderPass();
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
                pipelineDesc.blend.targets[0].colorMask     = { false, false, false, false };   // Write no color bits
            }
            pipelineStencilWrite = renderer->CreatePipelineState(pipelineDesc);
        }

        // Create graphics pipeline for stencil-read rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram                  = shaderProgramScene;
                pipelineDesc.renderPass                     = context->GetRenderPass();
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
        }
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for scene rendering
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews  = { constantBuffer };
        }
        resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void UpdateScene()
    {
        static float animation;

        // Update animation
        if (input->KeyPressed(LLGL::Key::LButton))
        {
            auto motion = input->GetMouseMotion();
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -45.0f, 0.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }

        if (input->KeyPressed(LLGL::Key::RButton))
        {
            auto motion = input->GetMouseMotion();
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
        commands->Clear(LLGL::ClearFlags::All);

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

            // Render everything directly into the render context
            commands->BeginRenderPass(*context);
            {
                commands->SetViewport(context->GetResolution());

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

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_StencilBuffer);



