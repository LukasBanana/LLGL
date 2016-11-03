/*
 * main.cpp (Tutorial11_PostProcessing)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


class Tutorial11 : public Tutorial
{

    const LLGL::ColorRGBAf  glowColor           = { 1, 1, 1, 1 };

    LLGL::ShaderProgram*    shaderProgramScene  = nullptr;
    LLGL::ShaderProgram*    shaderProgramBlur   = nullptr;
    LLGL::ShaderProgram*    shaderProgramFinal  = nullptr;

    LLGL::GraphicsPipeline* pipelineScene       = nullptr;
    LLGL::GraphicsPipeline* pipelineBlur        = nullptr;
    LLGL::GraphicsPipeline* pipelineFinal       = nullptr;

    LLGL::VertexFormat      vertexFormatScene;

    unsigned int            numSceneVertices    = 0;
    
    LLGL::Buffer*           vertexBufferScene   = nullptr;
    LLGL::Buffer*           vertexBufferNull    = nullptr;

    LLGL::Buffer*           constantBufferScene = nullptr;
    LLGL::Buffer*           constantBufferBlur  = nullptr;

    LLGL::Texture*          colorMap            = nullptr;
    LLGL::Texture*          glossMap            = nullptr;

    LLGL::Sampler*          colorMapSampler     = nullptr;
    LLGL::Sampler*          glossMapSampler     = nullptr;

    LLGL::RenderTarget*     renderTargetScene   = nullptr;
    LLGL::RenderTarget*     renderTargetBlurX   = nullptr;
    LLGL::RenderTarget*     renderTargetBlurY   = nullptr;

    LLGL::Texture*          glossMapBlurX       = nullptr;
    LLGL::Texture*          glossMapBlurY       = nullptr;

    struct SceneSettings
    {
        Gs::Matrix4f        wvpMatrix;
        Gs::Matrix4f        wMatrix;
        LLGL::ColorRGBAf    glossiness;
    }
    sceneSettings;

    struct BlurSettings
    {
        Gs::Vector2f        blurShift;
        float               _pad0[2];
    }
    blurSettings;

public:

    Tutorial11() :
        Tutorial( L"LLGL Tutorial 11: PostProcessing", { 800, 600 }, 0 )
    {
        // Create all graphics objects
        CreateBuffers();
        LoadShaders();
        CreatePipelines();
        CreateTextures();
        CreateRenderTargets();
    }

    void CreateBuffers()
    {
        // Specify vertex format for scene
        vertexFormatScene.AppendAttribute({ "position", LLGL::VectorType::Float3 });
        vertexFormatScene.AppendAttribute({ "normal", LLGL::VectorType::Float3 });

        // Create scene buffers
        auto sceneVertices = LoadObjModel("../Media/Models/WiredBox.obj");
        numSceneVertices = static_cast<unsigned int>(sceneVertices.size());

        vertexBufferScene = CreateVertexBuffer(sceneVertices, vertexFormatScene);
        constantBufferScene = CreateConstantBuffer(sceneSettings);

        // Create empty vertex buffer for post-processors,
        // because to draw meshes a vertex buffer is always required, even if it's empty
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.type   = LLGL::BufferType::Vertex;
            vertexBufferDesc.size   = 1;
        }
        vertexBufferNull = renderer->CreateBuffer(vertexBufferDesc);

        // Create post-processing buffers
        constantBufferBlur = CreateConstantBuffer(blurSettings);
    }
    
    void LoadShaders()
    {
        if (renderer->GetRenderingCaps().shadingLanguage >= LLGL::ShadingLanguage::HLSL_2_0)
        {
            // Load scene shader program
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "shader.hlsl", "VScene", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PScene", "ps_5_0" }
                },
                vertexFormatScene
            );

            // Load blur shader program
            shaderProgramBlur = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "shader.hlsl", "VPP", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PBlur", "ps_5_0" }
                },
                {} // empty vertex format
            );

            // Load final shader program
            shaderProgramFinal = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "shader.hlsl", "VPP", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PFinal", "ps_5_0" }
                },
                {} // empty vertex format
            );
        }
        else
        {
            //todo...
        }
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDescScene;
        {
            pipelineDescScene.shaderProgram             = shaderProgramScene;

            pipelineDescScene.depth.testEnabled         = true;
            pipelineDescScene.depth.writeEnabled        = true;

            pipelineDescScene.rasterizer.cullMode       = LLGL::CullMode::Back;
            pipelineDescScene.rasterizer.multiSampling  = LLGL::MultiSamplingDescriptor(8);
        }
        pipelineScene = renderer->CreateGraphicsPipeline(pipelineDescScene);

        // Create graphics pipeline for blur post-processor
        LLGL::GraphicsPipelineDescriptor pipelineDescPP;
        {
            pipelineDescPP.shaderProgram = shaderProgramBlur;
        }
        pipelineBlur = renderer->CreateGraphicsPipeline(pipelineDescPP);

        // Create graphics pipeline for final post-processor
        {
            pipelineDescPP.shaderProgram = shaderProgramFinal;
        }
        pipelineFinal = renderer->CreateGraphicsPipeline(pipelineDescPP);
    }

    void CreateTextures()
    {
        // Release previous textures
        //renderer->Release(*colorMap);
        //renderer->Release(*glossMap);
        //renderer->Release(*glossMapBlurX);
        //renderer->Release(*glossMapBlurY);

        // Create empty color and gloss map
        auto resolution = context->GetVideoMode().resolution.Cast<unsigned int>();

        colorMap        = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::TextureFormat::RGBA, resolution.x, resolution.y));
        glossMap        = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::TextureFormat::RGBA, resolution.x, resolution.y));
        glossMapBlurX   = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::TextureFormat::RGBA, resolution.x, resolution.y));
        glossMapBlurY   = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::TextureFormat::RGBA, resolution.x, resolution.y));

        // Create sampler states for all textures
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.minFilter   = LLGL::TextureFilter::Nearest;
            samplerDesc.magFilter   = LLGL::TextureFilter::Nearest;
            samplerDesc.mipMapping  = false;
        }
        colorMapSampler = renderer->CreateSampler(samplerDesc);
        glossMapSampler = renderer->CreateSampler(samplerDesc);
    }

    void CreateRenderTargets()
    {
        auto resolution = context->GetVideoMode().resolution.Cast<unsigned int>();

        // Release previous render target
        /*renderer->Release(*renderTargetScene);
        renderer->Release(*renderTargetBlurX);
        renderer->Release(*renderTargetBlurY);*/

        // Create render-target for scene rendering
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.multiSampling = LLGL::MultiSamplingDescriptor(8);
        }
        renderTargetScene = renderer->CreateRenderTarget(renderTargetDesc);
        renderTargetScene->AttachDepthBuffer(resolution);
        renderTargetScene->AttachTexture(*colorMap, {});
        renderTargetScene->AttachTexture(*glossMap, {});

        // Create render-target for horizontal blur pass (no depth buffer needed)
        renderTargetBlurX = renderer->CreateRenderTarget(renderTargetDesc);
        renderTargetBlurX->AttachTexture(*glossMapBlurX, {});

        // Create render-target for vertical blur pass (no depth buffer needed)
        renderTargetBlurY = renderer->CreateRenderTarget(renderTargetDesc);
        renderTargetBlurY->AttachTexture(*glossMapBlurY, {});
    }

private:

    void UpdateSceneSettings(float pitch, float yaw, bool innerModel)
    {
        // Transform scene mesh
        sceneSettings.wMatrix.LoadIdentity();
        Gs::Translate(sceneSettings.wMatrix, { 0, 0, 5 });

        if (innerModel)
        {
            static float rotAnim;
            rotAnim += 0.01f;
            Gs::RotateFree(sceneSettings.wMatrix, Gs::Vector3f(1).Normalized(), rotAnim);
            Gs::Scale(sceneSettings.wMatrix, Gs::Vector3f(0.5f));
            sceneSettings.glossiness = glowColor;
        }
        else
        {
            Gs::RotateFree(sceneSettings.wMatrix, { 1, 0, 0 }, pitch);
            Gs::RotateFree(sceneSettings.wMatrix, { 0, 1, 0 }, yaw);
            sceneSettings.glossiness = { 0, 0, 0, 0 };
        }

        sceneSettings.wvpMatrix = projection;
        sceneSettings.wvpMatrix *= sceneSettings.wMatrix;

        // Update constant buffer for scene settings
        UpdateBuffer(constantBufferScene, sceneSettings);
    }

    void OnDrawFrame() override
    {
        static const auto shaderStages = LLGL::ShaderStageFlags::VertexStage | LLGL::ShaderStageFlags::FragmentStage;

        // Update scene animation (simple rotation)
        static float pitch, yaw;

        if (input->KeyPressed(LLGL::Key::LButton))
        {
            auto rot = input->GetMouseMotion().Cast<float>()*0.005f;
            pitch   += rot.y;
            yaw     += rot.x;
        }

        // Set common buffers and sampler states
        commands->SetConstantBuffer(*constantBufferScene, 0, shaderStages);
        commands->SetConstantBuffer(*constantBufferBlur, 1, LLGL::ShaderStageFlags::FragmentStage);

        commands->SetSampler(*colorMapSampler, 0, LLGL::ShaderStageFlags::FragmentStage);
        commands->SetSampler(*glossMapSampler, 1, LLGL::ShaderStageFlags::FragmentStage);

        // Set graphics pipeline, vertex buffer, and render target for scene rendering
        commands->SetGraphicsPipeline(*pipelineScene);
        commands->SetVertexBuffer(*vertexBufferScene);
        //commands->SetRenderTarget(*renderTargetScene);
        commands->SetRenderTarget(*context);
        {
            // Clear color and depth buffers of active framebuffer (i.e. the render target)
            commands->SetClearColor({ 0.2f, 0.7f, 0.1f });
            commands->Clear(LLGL::ClearFlags::ColorDepth);

            // Draw outer scene model
            UpdateSceneSettings(pitch, yaw, false);
            commands->Draw(numSceneVertices, 0);

            // Draw inner scene model
            UpdateSceneSettings(pitch, yaw, true);
            commands->Draw(numSceneVertices, 0);
        }

        #if 0
        // Draw horizontal blur pass
        commands->SetGraphicsPipeline(*pipelineBlur);
        commands->SetVertexBuffer(*vertexBufferNull);
        commands->SetRenderTarget(*renderTargetBlurX);
        {
            // Draw fullscreen triangle (triangle is spanned in the vertex shader)
            commands->Draw(3, 0);
        }

        // Draw vertical blur pass
        commands->SetRenderTarget(*renderTargetBlurY);
        {
            // Draw fullscreen triangle (triangle is spanned in the vertex shader)
            commands->Draw(3, 0);
        }

        // Draw final post-processing pass
        commands->SetGraphicsPipeline(*pipelineFinal);
        commands->SetRenderTarget(*context);
        {
            // Draw fullscreen triangle (triangle is spanned in the vertex shader)
            commands->Draw(3, 0);
        }
        #endif

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial11);



