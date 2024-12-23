/*
 * Example.cpp (Example_ShadowMapping)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>


class Example_ShadowMapping : public ExampleBase
{

    LLGL::Shader*               vsShadowMap             = nullptr;
    LLGL::Shader*               vsScene                 = nullptr;
    LLGL::Shader*               fsScene                 = nullptr;

    LLGL::PipelineLayout*       pipelineLayoutShadowMap = nullptr;
    LLGL::PipelineLayout*       pipelineLayoutScene     = nullptr;

    LLGL::PipelineState*        pipelineShadowMap       = nullptr;
    LLGL::PipelineState*        pipelineScene           = nullptr;

    LLGL::ResourceHeap*         resourceHeapShadowMap   = nullptr;
    LLGL::ResourceHeap*         resourceHeapScene       = nullptr;

    LLGL::Buffer*               vertexBuffer            = nullptr;
    LLGL::Buffer*               constantBuffer          = nullptr;

    LLGL::Texture*              shadowMap               = nullptr;
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
        ExampleBase { "LLGL Example: ShadowMapping" }
    {
        // Create all graphics objects
        CreateShadowMap();
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
        meshes.push_back(LoadObjModel(vertices, "SimpleRoom.obj"));
        meshes.push_back(LoadObjModel(vertices, "WiredBox.obj"));

        meshes[1].color = { 0.4f, 0.5f, 1.0f };

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::GLSL) || Supported(LLGL::ShadingLanguage::ESSL))
        {
            vsShadowMap = LoadShaderAndPatchClippingOrigin({ LLGL::ShaderType::Vertex, "ShadowMap.vert" }, { vertexFormat });

            vsScene     = LoadShader({ LLGL::ShaderType::Vertex,   "Scene.vert" }, { vertexFormat });
            fsScene     = LoadShader({ LLGL::ShaderType::Fragment, "Scene.frag" });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            vsShadowMap = LoadShader({ LLGL::ShaderType::Vertex, "ShadowMap.450core.vert.spv" }, { vertexFormat });

            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Scene.450core.vert.spv" }, { vertexFormat });
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Scene.450core.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            vsShadowMap = LoadShader({ LLGL::ShaderType::Vertex, "Example.hlsl", "VShadowMap", "vs_5_0" }, { vertexFormat });

            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VScene", "vs_5_0" }, { vertexFormat });
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PScene", "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            vsShadowMap = LoadShader({ LLGL::ShaderType::Vertex, "Example.metal", "VShadowMap", "1.1" }, { vertexFormat });

            vsScene = LoadShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VScene", "1.1" }, { vertexFormat });
            fsScene = LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", "PScene", "1.1" });
        }
        else
            throw std::runtime_error("shaders not supported for active renderer");
    }

    void CreateShadowMap()
    {
        // Create texture
        LLGL::TextureDescriptor textureDesc;
        {
            textureDesc.debugName       = "ShadowMap.Texture";
            textureDesc.type            = LLGL::TextureType::Texture2D;
            textureDesc.bindFlags       = LLGL::BindFlags::DepthStencilAttachment | LLGL::BindFlags::Sampled;
            textureDesc.format          = LLGL::Format::D32Float;
            textureDesc.extent.width    = shadowMapResolution.width;
            textureDesc.extent.height   = shadowMapResolution.height;
            textureDesc.extent.depth    = 1;
            textureDesc.mipLevels       = 1;
        }
        shadowMap = renderer->CreateTexture(textureDesc);

        // Create render target
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.debugName              = "ShadowMap.RenderTarget";
            renderTargetDesc.resolution             = shadowMapResolution;
            renderTargetDesc.depthStencilAttachment = shadowMap;
        }
        shadowMapRenderTarget = renderer->CreateRenderTarget(renderTargetDesc);
    }

    void CreatePipelineLayouts()
    {
        // Initialize shadow-map sampler
        LLGL::SamplerDescriptor shadowSamplerDesc;
        {
            // Clamp-to-border sampler address mode requires GLES 3.2, so use standard clamp mode in case hardware only supports GLES 3.0
            if (renderer->GetRendererID() == LLGL::RendererID::OpenGLES ||
                renderer->GetRendererID() == LLGL::RendererID::WebGL)
            {
                shadowSamplerDesc.addressModeU      = LLGL::SamplerAddressMode::Clamp;
                shadowSamplerDesc.addressModeV      = LLGL::SamplerAddressMode::Clamp;
                shadowSamplerDesc.addressModeW      = LLGL::SamplerAddressMode::Clamp;
            }
            else
            {
                shadowSamplerDesc.addressModeU      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.addressModeV      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.addressModeW      = LLGL::SamplerAddressMode::Border;
                shadowSamplerDesc.borderColor[0]    = 1.0f;
                shadowSamplerDesc.borderColor[1]    = 1.0f;
                shadowSamplerDesc.borderColor[2]    = 1.0f;
                shadowSamplerDesc.borderColor[3]    = 1.0f;
            }
            shadowSamplerDesc.compareEnabled    = true;
            shadowSamplerDesc.mipMapEnabled     = false;
        }

        // Create pipeline layouts for shadow-map rendering
        LLGL::PipelineLayoutDescriptor shadowLayoutDesc;
        {
            shadowLayoutDesc.heapBindings =
            {
                LLGL::BindingDescriptor{ "Settings", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
            };
        }
        pipelineLayoutShadowMap = renderer->CreatePipelineLayout(shadowLayoutDesc);

        // Create pipeline layout for scene rendering
        LLGL::PipelineLayoutDescriptor sceneLayoutDesc;
        {
            sceneLayoutDesc.heapBindings =
            {
                LLGL::BindingDescriptor{ "Settings",  LLGL::ResourceType::Buffer,  LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage | LLGL::StageFlags::VertexStage, 1 },
                LLGL::BindingDescriptor{ "shadowMap", LLGL::ResourceType::Texture, LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 2 },
            };
            sceneLayoutDesc.staticSamplers =
            {
                LLGL::StaticSamplerDescriptor{ "shadowMapSampler", LLGL::StageFlags::FragmentStage, 3, shadowSamplerDesc }
            };
            sceneLayoutDesc.combinedTextureSamplers =
            {
                LLGL::CombinedTextureSamplerDescriptor{ "shadowMap", "shadowMap", "shadowMapSampler", 2 }
            };
        }
        pipelineLayoutScene = renderer->CreatePipelineLayout(sceneLayoutDesc);
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for shadow-map rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.vertexShader                           = vsShadowMap;
                pipelineDesc.renderPass                             = shadowMapRenderTarget->GetRenderPass();
                pipelineDesc.pipelineLayout                         = pipelineLayoutShadowMap;
                pipelineDesc.depth.testEnabled                      = true;
                pipelineDesc.depth.writeEnabled                     = true;
                pipelineDesc.rasterizer.cullMode                    = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.depthBias.constantFactor    = 4.0f;
                pipelineDesc.rasterizer.depthBias.slopeFactor       = 1.5f;
                pipelineDesc.blend.targets[0].colorMask             = 0x0;
                pipelineDesc.viewports                              = { shadowMapResolution };
            }
            pipelineShadowMap = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(pipelineShadowMap);
        }

        // Create graphics pipeline for scene rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.vertexShader                   = vsScene;
                pipelineDesc.fragmentShader                 = fsScene;
                pipelineDesc.renderPass                     = swapChain->GetRenderPass();
                pipelineDesc.pipelineLayout                 = pipelineLayoutScene;
                pipelineDesc.depth.testEnabled              = true;
                pipelineDesc.depth.writeEnabled             = true;
                pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
                pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            }
            pipelineScene = renderer->CreatePipelineState(pipelineDesc);
            ReportPSOErrors(pipelineScene);
        }
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for shadow-map rendering
        resourceHeapShadowMap = renderer->CreateResourceHeap(pipelineLayoutShadowMap, { constantBuffer });

        // Create resource heap for scene rendering
        resourceHeapScene = renderer->CreateResourceHeap(pipelineLayoutScene, { constantBuffer, shadowMap });
    }

    void UpdateScene()
    {
        // Update animation
        static float animation;

        if (input.KeyPressed(LLGL::Key::LButton))
        {
            auto motion = input.GetMouseMotion();
            viewRotation.x += static_cast<float>(motion.y) * 0.25f;
            viewRotation.x = Gs::Clamp(viewRotation.x, -90.0f, 0.0f);
            viewRotation.y += static_cast<float>(motion.x) * 0.25f;
        }

        if (input.KeyPressed(LLGL::Key::RButton))
        {
            auto motion = input.GetMouseMotion();
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
        commands->BeginRenderPass(*swapChain);
        {
            commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);
            commands->SetViewport(swapChain->GetResolution());
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
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_ShadowMapping);



