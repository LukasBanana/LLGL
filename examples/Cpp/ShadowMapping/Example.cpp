/*
 * Example.cpp (Example_ShadowMapping)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


class Example_ShadowMapping : public ExampleBase
{

    LLGL::ShaderProgram*        shaderProgramShadowMap  = nullptr;
    LLGL::ShaderProgram*        shaderProgramScene      = nullptr;

    LLGL::PipelineLayout*       pipelineLayoutShadowMap = nullptr;
    LLGL::PipelineLayout*       pipelineLayoutScene     = nullptr;

    LLGL::PipelineState*        pipelineShadowMap       = {};
    LLGL::PipelineState*        pipelineScene           = {};

    LLGL::ResourceHeap*         resourceHeapShadowMap   = {};
    LLGL::ResourceHeap*         resourceHeapScene       = {};

    LLGL::Buffer*               vertexBuffer            = nullptr;
    LLGL::Buffer*               constantBuffer          = nullptr;

    LLGL::Texture*              shadowMap               = nullptr;
    LLGL::Sampler*              shadowMapSampler        = nullptr;
    const LLGL::Extent2D        shadowMapResolution     = { 256, 256 };
    LLGL::RenderTarget*         shadowMapRenderTarget   = nullptr;

    std::vector<TriangleMesh>   meshes;

    Gs::Vector3f                boxPosition             = { 0, 0, 0 };
    float                       viewDistanceToBox       = 1.25f;
    Gs::Vector2f                viewRotation;
    float                       spotLightAngle          = 35.0f;
    Gs::Vector3f                lightOffset             = { 0, 1.5f, 0 };

    struct Settings
    {
        Gs::Matrix4f            wMatrix;
        Gs::Matrix4f            vpMatrix;
        Gs::Matrix4f            vpShadowMatrix;
        Gs::Vector3f            lightDir                = Gs::Vector3f(-0.25f, -1.0f, 0.5f).Normalized();
        float                   _pad1;
        LLGL::ColorRGBAf        diffuse                 = { 1.0f, 1.0f, 1.0f, 1.0f };
    }
    settings;

public:

    Example_ShadowMapping() :
        ExampleBase { L"LLGL Example: ShadowMapping" }
    {
        // Create all graphics objects
        CreateShadowMap();
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreatePipelineLayouts();
        CreatePipelines();
        CreateResourceHeaps();

        commands->SetClearColor(backgroundColor);

        // Label objects for debugging
        context->SetName("BackBuffer");

        vertexBuffer->SetName("Buffer.Vertices");
        constantBuffer->SetName("Buffer.Constants");

        shaderProgramShadowMap->SetName("ShadowMap.ShaderProgram");
        shaderProgramScene->SetName("Scene.ShaderProgram");

        shadowMap->SetName("ShadowMap.Texture");
        shadowMapRenderTarget->SetName("ShadowMap.RenderTarget");

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
        meshes.push_back(LoadObjModel(vertices, "../../Media/Models/SimpleRoom.obj"));
        meshes.push_back(LoadObjModel(vertices, "../../Media/Models/WiredBox.obj"));

        meshes[1].color = { 0.4f, 0.5f, 1.0f };

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            shaderProgramShadowMap = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "ShadowMap.vert" }
                },
                { vertexFormat }
            );
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Scene.vert" },
                    { LLGL::ShaderType::Fragment, "Scene.frag" },
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderProgramShadowMap = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "ShadowMap.450core.vert.spv" }
                },
                { vertexFormat }
            );
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Scene.450core.vert.spv" },
                    { LLGL::ShaderType::Fragment, "Scene.450core.frag.spv" },
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            shaderProgramShadowMap = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "Example.hlsl", "VShadowMap", "vs_5_0" }
                },
                { vertexFormat }
            );
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VScene", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "Example.hlsl", "PScene", "ps_5_0" },
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            shaderProgramShadowMap = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "Example.metal", "VShadowMap", "1.1" }
                },
                { vertexFormat }
            );
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.metal", "VScene", "1.1" },
                    { LLGL::ShaderType::Fragment, "Example.metal", "PScene", "1.1" },
                },
                { vertexFormat }
            );
        }
        else
            throw std::runtime_error("shaders not supported for active renderer");
    }

    void CreateShadowMap()
    {
        // Create texture
        LLGL::TextureDescriptor textureDesc;
        {
            textureDesc.type            = LLGL::TextureType::Texture2D;
            textureDesc.bindFlags       = LLGL::BindFlags::DepthStencilAttachment | LLGL::BindFlags::Sampled;
            textureDesc.format          = LLGL::Format::D32Float;
            textureDesc.extent.width    = shadowMapResolution.width;
            textureDesc.extent.height   = shadowMapResolution.height;
            textureDesc.extent.depth    = 1;
        }
        shadowMap = renderer->CreateTexture(textureDesc);

        // Create render target
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.resolution     = shadowMapResolution;
            renderTargetDesc.attachments    =
            {
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Depth, shadowMap }
            };
        }
        shadowMapRenderTarget = renderer->CreateRenderTarget(renderTargetDesc);

        // Create texture sampler
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Border;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Border;
            samplerDesc.addressModeW    = LLGL::SamplerAddressMode::Border;
            samplerDesc.borderColor     = { 1.0f, 1.0f, 1.0f, 1.0f };
            samplerDesc.compareEnabled  = true;
            samplerDesc.mipMapping      = false;
        }
        shadowMapSampler = renderer->CreateSampler(samplerDesc);
    }

    void CreatePipelineLayouts()
    {
        // Create pipeline layouts for shadow-map and scene rendering
        if (IsOpenGL())
        {
            pipelineLayoutShadowMap = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(0):vert")
            );
            pipelineLayoutScene = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(0):frag:vert, texture(0):frag, sampler(0):frag")
            );
        }
        else
        {
            pipelineLayoutShadowMap = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(1):vert")
            );
            pipelineLayoutScene = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(1):frag:vert, texture(2):frag, sampler(3):frag")
            );
        }
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for shadow-map rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram                          = shaderProgramShadowMap;
                pipelineDesc.renderPass                             = shadowMapRenderTarget->GetRenderPass();
                pipelineDesc.pipelineLayout                         = pipelineLayoutShadowMap;
                pipelineDesc.depth.testEnabled                      = true;
                pipelineDesc.depth.writeEnabled                     = true;
                pipelineDesc.rasterizer.cullMode                    = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.depthBias.constantFactor    = 4.0f;
                pipelineDesc.rasterizer.depthBias.slopeFactor       = 4.0f;
                pipelineDesc.blend.targets[0].colorMask             = { false, false, false, false };
                pipelineDesc.viewports                              = { shadowMapResolution };
            }
            pipelineShadowMap = renderer->CreatePipelineState(pipelineDesc);
        }

        // Create graphics pipeline for scene rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram                  = shaderProgramScene;
                pipelineDesc.renderPass                     = context->GetRenderPass();
                pipelineDesc.pipelineLayout                 = pipelineLayoutScene;
                pipelineDesc.depth.testEnabled              = true;
                pipelineDesc.depth.writeEnabled             = true;
                pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            }
            pipelineScene = renderer->CreatePipelineState(pipelineDesc);
        }
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for shadow-map rendering
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayoutShadowMap;
            resourceHeapDesc.resourceViews  = { constantBuffer };
        }
        resourceHeapShadowMap = renderer->CreateResourceHeap(resourceHeapDesc);

        // Create resource heap for scene rendering
        {
            resourceHeapDesc.pipelineLayout = pipelineLayoutScene;
            resourceHeapDesc.resourceViews  = { constantBuffer, shadowMap, shadowMapSampler };
        }
        resourceHeapScene = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void UpdateScene()
    {
        // Update animation
        static float animation;

        if (input->KeyPressed(LLGL::Key::LButton))
        {
            auto motion = input->GetMouseMotion();
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -90.0f, 0.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }

        if (input->KeyPressed(LLGL::Key::RButton))
        {
            auto motion = input->GetMouseMotion();
            animation += static_cast<float>(motion.x) * 0.25f;
        }

        // Update model transform
        meshes[1].transform.LoadIdentity();
        Gs::Translate(meshes[1].transform, boxPosition);
        Gs::RotateFree(meshes[1].transform, Gs::Vector3f(1.0f).Normalized(), Gs::Deg2Rad(animation));
        Gs::Scale(meshes[1].transform, Gs::Vector3f(0.15f));

        // Update view transformation
        settings.vpMatrix.LoadIdentity();
        Gs::Translate(settings.vpMatrix, boxPosition);
        Gs::RotateFree(settings.vpMatrix, { 0, 1, 0 }, Gs::Deg2Rad(viewRotation.y));
        Gs::RotateFree(settings.vpMatrix, { 1, 0, 0 }, Gs::Deg2Rad(viewRotation.x));
        Gs::Translate(settings.vpMatrix, { 0, 0, -viewDistanceToBox });
        settings.vpMatrix.MakeInverse();
        settings.vpMatrix = projection * settings.vpMatrix;

        // Update light transformation
        auto lightProjection = PerspectiveProjection(1.0f, 0.1f, 100.0f, Gs::Deg2Rad(spotLightAngle));

        settings.vpShadowMatrix.LoadIdentity();
        Gs::Translate(settings.vpShadowMatrix, boxPosition + lightOffset);
        Gs::RotateFree(settings.vpShadowMatrix, { 1, 0, 0 }, Gs::Deg2Rad(-90.0f));
        settings.vpShadowMatrix.MakeInverse();
        settings.vpShadowMatrix = lightProjection * settings.vpShadowMatrix;
    }

    void RenderMesh(const TriangleMesh& mesh)
    {
        settings.wMatrix = mesh.transform;
        settings.diffuse = mesh.color;
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));
        commands->Draw(mesh.numVertices, mesh.firstVertex);
    }

    void RenderAllMeshes()
    {
        for (const auto& mesh : meshes)
            RenderMesh(mesh);
    }

    void RenderShadowMap()
    {
        // Render scene into shadow-map texture
        commands->BeginRenderPass(*shadowMapRenderTarget);
        {
            commands->Clear(LLGL::ClearFlags::Depth);
            commands->SetPipelineState(*pipelineShadowMap);
            commands->SetResourceHeap(*resourceHeapShadowMap);
            RenderAllMeshes();
        }
        commands->EndRenderPass();

        // Update MIP-maps of shadow-map texture
        //commands->GenerateMips(*shadowMap);
    }

    void RenderScene()
    {
        // Render scene onto screen
        commands->BeginRenderPass(*context);
        {
            commands->Clear(LLGL::ClearFlags::ColorDepth);
            commands->SetViewport(context->GetResolution());
            commands->SetPipelineState(*pipelineScene);
            commands->SetResourceHeap(*resourceHeapScene);
            RenderAllMeshes();
        }
        commands->EndRenderPass();
    }

    void OnDrawFrame() override
    {
        // Update scene by user input
        UpdateScene();

        commands->Begin();
        {
            // Bind common input assembly
            commands->SetVertexBuffer(*vertexBuffer);

            // Unbind shadow texture before rendering into it
            commands->ResetResourceSlots(LLGL::ResourceType::Texture, 2, 1, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage);

            // Draw scene into shadow-map, then draw scene onto screen
            commands->PushDebugGroup("Shadow Map Pass");
            RenderShadowMap();
            commands->PopDebugGroup();

            commands->PushDebugGroup("Scene Pass");
            RenderScene();
            commands->PopDebugGroup();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_ShadowMapping);



