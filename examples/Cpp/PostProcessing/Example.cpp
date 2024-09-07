/*
 * Example.cpp (Example_PostProcessing)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>

//#define DEBUG_FPS
#ifdef DEBUG_FPS
#include <chrono>//!!!
#endif

// Enables custom render pass to clear at the begin of a render pass section (more efficient)
#define ENABLE_CUSTOM_RENDER_PASS 1


class Example_PostProcessing : public ExampleBase
{

    const LLGL::ColorRGBAf  glowColor           = { 0.9f, 0.7f, 0.3f, 1.0f };

    ShaderPipeline          shaderPipelineScene;
    ShaderPipeline          shaderPipelineBlur;
    ShaderPipeline          shaderPipelineFinal;

    LLGL::PipelineLayout*   layoutScene         = nullptr;
    LLGL::PipelineLayout*   layoutBlur          = nullptr;
    LLGL::PipelineLayout*   layoutFinal         = nullptr;

    LLGL::PipelineState*    pipelineScene       = nullptr;
    LLGL::PipelineState*    pipelineBlur        = nullptr;
    LLGL::PipelineState*    pipelineFinal       = nullptr;

    LLGL::ResourceHeap*     resourceHeapScene   = nullptr;
    LLGL::ResourceHeap*     resourceHeapBlur    = nullptr;
    LLGL::ResourceHeap*     resourceHeapFinal   = nullptr;

    LLGL::VertexFormat      vertexFormatScene;

    std::uint32_t           numSceneVertices    = 0;

    LLGL::Buffer*           vertexBufferScene   = nullptr;
    LLGL::Buffer*           vertexBufferNull    = nullptr;

    LLGL::Buffer*           constantBufferScene = nullptr;
    LLGL::Buffer*           constantBufferBlur  = nullptr;

    LLGL::Sampler*          colorMapSampler     = nullptr;
    LLGL::Sampler*          glossMapSampler     = nullptr;

    LLGL::Texture*          colorMap            = nullptr;
    LLGL::Texture*          glossMap            = nullptr;
    LLGL::Texture*          glossMapBlurX       = nullptr;
    LLGL::Texture*          glossMapBlurY       = nullptr;

    LLGL::RenderTarget*     renderTargetScene   = nullptr;
    LLGL::RenderTarget*     renderTargetBlurX   = nullptr;
    LLGL::RenderTarget*     renderTargetBlurY   = nullptr;

    #if ENABLE_CUSTOM_RENDER_PASS
    LLGL::RenderPass*       renderPassScene     = nullptr;
    #endif

    struct SceneSettings
    {
        Gs::Matrix4f        wvpMatrix;
        Gs::Matrix4f        wMatrix;
        LLGL::ColorRGBAf    diffuse;
        LLGL::ColorRGBAf    glossiness;
        float               intensity           = 3.0f;
        float               _pad0[3];
    }
    sceneSettings;

    struct BlurSettings
    {
        Gs::Vector2f        blurShift;
        float               _pad0[2];
    }
    blurSettings;

    struct Animation
    {
        Gs::Matrix4f    rotation;
        float           innerModelRotation = 0.0f;
    }
    animation;

public:

    Example_PostProcessing() :
        ExampleBase { "LLGL Example: PostProcessing" }
    {
        // Create all graphics objects
        CreateBuffers();
        LoadShaders();
        CreateSamplers();
        CreateTextures();
        CreateRenderTargets();
        #if ENABLE_CUSTOM_RENDER_PASS
        CreateRenderPasses();
        #endif
        CreatePipelineLayouts();
        CreatePipelines();
        CreateResourceHeaps();

        // Show some information
        LLGL::Log::Printf(
            "press LEFT MOUSE BUTTON and move the mouse to rotate the outer box\n"
            "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to change the glow intensity\n"
        );
    }

    void CreateBuffers()
    {
        // Specify vertex format for scene
        vertexFormatScene.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormatScene.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
        vertexFormatScene.SetStride(sizeof(TexturedVertex));

        // Create scene buffers
        auto sceneVertices = LoadObjModel("WiredBox.obj");
        numSceneVertices = static_cast<std::uint32_t>(sceneVertices.size());

        vertexBufferScene = CreateVertexBuffer(sceneVertices, vertexFormatScene);
        constantBufferScene = CreateConstantBuffer(sceneSettings);

        // Create empty vertex buffer for post-processors,
        // because to draw meshes a vertex buffer is always required, even if it's empty
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size       = 1;
            vertexBufferDesc.bindFlags  = LLGL::BindFlags::VertexBuffer;
        }
        vertexBufferNull = renderer->CreateBuffer(vertexBufferDesc);

        // Create post-processing buffers
        constantBufferBlur = CreateConstantBuffer(blurSettings);
    }

    void LoadShaders()
    {
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            // Load scene shader program
            shaderPipelineScene.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VScene", "vs_5_0" }, { vertexFormatScene });
            shaderPipelineScene.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PScene", "ps_5_0" });

            // Load blur shader program
            shaderPipelineBlur.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VPP",   "vs_5_0" });
            shaderPipelineBlur.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PBlur", "ps_5_0" });

            // Load final shader program
            shaderPipelineFinal.vs = shaderPipelineBlur.vs;
            shaderPipelineFinal.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PFinal", "ps_5_0" });
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL) || Supported(LLGL::ShadingLanguage::ESSL))
        {
            // Load scene shader program
            shaderPipelineScene.vs = LoadShaderAndPatchClippingOrigin({ LLGL::ShaderType::Vertex,   "Scene.vert" }, { vertexFormatScene });
            shaderPipelineScene.ps = LoadShader                      ({ LLGL::ShaderType::Fragment, "Scene.frag" });

            // Load blur shader program
            shaderPipelineBlur.vs = LoadShaderAndPatchClippingOrigin({ LLGL::ShaderType::Vertex,   "PostProcess.vert" });
            shaderPipelineBlur.ps = LoadShader                      ({ LLGL::ShaderType::Fragment, "Blur.frag"        });

            // Load final shader program
            shaderPipelineFinal.vs = LoadShader({ LLGL::ShaderType::Vertex,   "PostProcess.vert" });
            shaderPipelineFinal.ps = LoadShader({ LLGL::ShaderType::Fragment, "Final.frag"       });
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            // Load scene shader program
            shaderPipelineScene.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Scene.450core.vert.spv" }, { vertexFormatScene });
            shaderPipelineScene.ps = LoadShader({ LLGL::ShaderType::Fragment, "Scene.450core.frag.spv" });

            // Load blur shader program
            shaderPipelineBlur.vs = LoadShader({ LLGL::ShaderType::Vertex,   "PostProcess.450core.vert.spv" });
            shaderPipelineBlur.ps = LoadShader({ LLGL::ShaderType::Fragment, "Blur.450core.frag.spv"        });

            // Load final shader program
            shaderPipelineFinal.vs = shaderPipelineBlur.vs;
            shaderPipelineFinal.ps = LoadShader({ LLGL::ShaderType::Fragment, "Final.450core.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            // Load scene shader program
            shaderPipelineScene.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VScene", "1.1" }, { vertexFormatScene });
            shaderPipelineScene.ps = LoadShader( { LLGL::ShaderType::Fragment, "Example.metal", "PScene", "1.1" });

            // Load blur shader program
            shaderPipelineBlur.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VPP",   "1.1" });
            shaderPipelineBlur.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", "PBlur", "1.1" });

            // Load final shader program
            shaderPipelineFinal.vs = shaderPipelineBlur.vs;
            shaderPipelineFinal.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", "PFinal", "1.1" });
        }
    }

    void CreateSamplers()
    {
        // Create sampler states for all textures
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.mipMapEnabled = false;
        }
        colorMapSampler = renderer->CreateSampler(samplerDesc);
        glossMapSampler = renderer->CreateSampler(samplerDesc);
    }

    void CreateTextures()
    {
        // Create empty color and gloss map
        auto resolution = swapChain->GetResolution();

        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type            = LLGL::TextureType::Texture2D;
            texDesc.format          = LLGL::Format::RGBA8UNorm;
            texDesc.bindFlags       = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment;
            texDesc.miscFlags       = LLGL::MiscFlags::NoInitialData;
            texDesc.extent.width    = resolution.width;
            texDesc.extent.height   = resolution.height;
            texDesc.mipLevels       = 1;
        }
        colorMap = renderer->CreateTexture(texDesc);
        glossMap = renderer->CreateTexture(texDesc);

        // Create empty blur pass maps (in quarter resolution)
        texDesc.extent.width  /= 4;
        texDesc.extent.height /= 4;

        glossMapBlurX = renderer->CreateTexture(texDesc);
        glossMapBlurY = renderer->CreateTexture(texDesc);
    }

    void CreateRenderTargets()
    {
        auto resolution = swapChain->GetResolution();

        // Create render-target for scene rendering
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.resolution = resolution;
            renderTargetDesc.samples    = GetSampleCount();
            if (renderTargetDesc.samples > 1)
            {
                renderTargetDesc.colorAttachments[0]    = colorMap->GetFormat();
                renderTargetDesc.colorAttachments[1]    = glossMap->GetFormat();
                renderTargetDesc.resolveAttachments[0]  = colorMap;
                renderTargetDesc.resolveAttachments[1]  = glossMap;
            }
            else
            {
                renderTargetDesc.colorAttachments[0]    = colorMap;
                renderTargetDesc.colorAttachments[1]    = glossMap;
            }
            renderTargetDesc.depthStencilAttachment = LLGL::Format::D32Float;
        }
        renderTargetScene = renderer->CreateRenderTarget(renderTargetDesc);

        // Create render-target for horizontal blur pass (no depth buffer needed)
        resolution.width    /= 4;
        resolution.height   /= 4;

        LLGL::RenderTargetDescriptor renderTargetBlurXDesc;
        {
            renderTargetBlurXDesc.resolution            = resolution;
            renderTargetBlurXDesc.colorAttachments[0]   = glossMapBlurX;
        }
        renderTargetBlurX = renderer->CreateRenderTarget(renderTargetBlurXDesc);

        // Create render-target for vertical blur pass (no depth buffer needed)
        LLGL::RenderTargetDescriptor renderTargetBlurYDesc;
        {
            renderTargetBlurYDesc.resolution            = resolution;
            renderTargetBlurYDesc.colorAttachments[0]   = glossMapBlurY;
        }
        renderTargetBlurY = renderer->CreateRenderTarget(renderTargetBlurYDesc);
    }

    #if ENABLE_CUSTOM_RENDER_PASS

    void CreateRenderPasses()
    {
        //TODO: should be able to query depth-stencil format from <RenderTarget> just like with <SwapChain>
        LLGL::RenderPassDescriptor renderPassDesc;
        {
            renderPassDesc.colorAttachments[0]  = LLGL::AttachmentFormatDescriptor{ colorMap->GetFormat(),  LLGL::AttachmentLoadOp::Clear };
            renderPassDesc.colorAttachments[1]  = LLGL::AttachmentFormatDescriptor{ glossMap->GetFormat(),  LLGL::AttachmentLoadOp::Clear };
            renderPassDesc.depthAttachment      = LLGL::AttachmentFormatDescriptor{ LLGL::Format::D32Float, LLGL::AttachmentLoadOp::Clear };
            renderPassDesc.samples              = GetSampleCount();
        }
        renderPassScene = renderer->CreateRenderPass(renderPassDesc);
    }

    #endif // /ENABLE_CUSTOM_RENDER_PASS

    // The utility function <LLGL::Parse> is used here, to simplify the description of the pipeline layouts
    void CreatePipelineLayouts()
    {
        bool combinedSampler = IsOpenGL();

        // Create pipeline layout for scene rendering
        layoutScene = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(SceneSettings@1):vert:frag}"));

        // Create pipeline layout for blur post-processor
        if (combinedSampler)
            layoutBlur = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(BlurSettings@2):frag, texture(glossMap@4):frag, sampler(4):frag}"));
        else
            layoutBlur = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(BlurSettings@2):frag, texture(glossMap@4):frag, sampler(6):frag}"));

        // Create pipeline layout for final post-processor
        if (combinedSampler)
            layoutFinal = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(SceneSettings@1):frag, texture(colorMap@3,glossMap@4):frag, sampler(3,4):frag}"));
        else
            layoutFinal = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(SceneSettings@1):frag, texture(colorMap@3,glossMap@4):frag, sampler(5,6):frag}"));
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDescScene;
        {
            pipelineDescScene.vertexShader                  = shaderPipelineScene.vs;
            pipelineDescScene.fragmentShader                = shaderPipelineScene.ps;
            pipelineDescScene.renderPass                    = renderTargetScene->GetRenderPass();
            pipelineDescScene.pipelineLayout                = layoutScene;

            pipelineDescScene.depth.testEnabled             = true;
            pipelineDescScene.depth.writeEnabled            = true;

            pipelineDescScene.rasterizer.cullMode           = LLGL::CullMode::Back;
            pipelineDescScene.rasterizer.multiSampleEnabled = (GetSampleCount() > 1);
        }
        pipelineScene = renderer->CreatePipelineState(pipelineDescScene);
        ReportPSOErrors(pipelineScene);

        // Create graphics pipeline for blur post-processor
        LLGL::GraphicsPipelineDescriptor pipelineDescPP;
        {
            pipelineDescPP.vertexShader     = shaderPipelineBlur.vs;
            pipelineDescPP.fragmentShader   = shaderPipelineBlur.ps;
            pipelineDescPP.renderPass       = renderTargetBlurX->GetRenderPass();
            pipelineDescPP.pipelineLayout   = layoutBlur;
        }
        pipelineBlur = renderer->CreatePipelineState(pipelineDescPP);
        ReportPSOErrors(pipelineBlur);

        // Create graphics pipeline for final post-processor
        LLGL::GraphicsPipelineDescriptor pipelineDescFinal;
        {
            pipelineDescFinal.vertexShader                  = shaderPipelineFinal.vs;
            pipelineDescFinal.fragmentShader                = shaderPipelineFinal.ps;
            pipelineDescFinal.pipelineLayout                = layoutFinal;
            pipelineDescFinal.renderPass                    = swapChain->GetRenderPass();
            pipelineDescFinal.rasterizer.multiSampleEnabled = (GetSampleCount() > 1);
        }
        pipelineFinal = renderer->CreatePipelineState(pipelineDescFinal);
        ReportPSOErrors(pipelineFinal);
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for scene rendering
        resourceHeapScene = renderer->CreateResourceHeap(layoutScene, { constantBufferScene });
        resourceHeapScene->SetDebugName("ResourceHeap.Scene");

        // Create resource heap for blur-X and blur-Y post-processor
        const LLGL::ResourceViewDescriptor resourceViewsBlurPass[] =
        {
            constantBufferBlur, glossMap,      glossMapSampler, // Resources for blur-X pass
            constantBufferBlur, glossMapBlurX, glossMapSampler, // Resources for blur-Y pass
        };
        resourceHeapBlur = renderer->CreateResourceHeap(layoutBlur, resourceViewsBlurPass);
        resourceHeapBlur->SetDebugName("ResourceHeap.Blur");

        // Create resource heap for final post-processor
        const LLGL::ResourceViewDescriptor resourceViewsFinalPass[] =
        {
            constantBufferScene, colorMap, glossMapBlurY, colorMapSampler, glossMapSampler
        };
        resourceHeapFinal = renderer->CreateResourceHeap(layoutFinal, resourceViewsFinalPass);
        resourceHeapFinal->SetDebugName("ResourceHeap.Final");
    }

    void UpdateScreenSize()
    {
        // Release previous textures
        renderer->Release(*renderTargetScene);
        renderer->Release(*renderTargetBlurX);
        renderer->Release(*renderTargetBlurY);

        renderer->Release(*resourceHeapScene);
        renderer->Release(*resourceHeapBlur);
        renderer->Release(*resourceHeapFinal);

        renderer->Release(*colorMap);
        renderer->Release(*glossMap);
        renderer->Release(*glossMapBlurX);
        renderer->Release(*glossMapBlurY);

        renderer->Release(*pipelineScene);
        renderer->Release(*pipelineBlur);
        renderer->Release(*pipelineFinal);

        // Recreate objects
        CreateTextures();
        CreateResourceHeaps();
        CreateRenderTargets();
        CreatePipelines();
    }

private:

    void SetSceneSettingsInnerModel(float rotation)
    {
        // Transform scene mesh
        sceneSettings.wMatrix.LoadIdentity();
        Gs::Translate(sceneSettings.wMatrix, { 0, 0, 5 });

        // Rotate model around the (1, 1, 1) axis
        Gs::RotateFree(sceneSettings.wMatrix, Gs::Vector3f(1).Normalized(), rotation);
        Gs::Scale(sceneSettings.wMatrix, Gs::Vector3f(0.5f));

        // Set colors and matrix
        sceneSettings.diffuse       = glowColor;
        sceneSettings.glossiness    = glowColor;
        sceneSettings.wvpMatrix     = projection * sceneSettings.wMatrix;

        // Update constant buffer for scene settings
        commands->UpdateBuffer(*constantBufferScene, 0, &sceneSettings, sizeof(sceneSettings));
    }

    void SetSceneSettingsOuterModel(float deltaPitch, float deltaYaw)
    {
        // Rotate model around X and Y axes
        Gs::Matrix4f deltaRotation;
        Gs::RotateFree(deltaRotation, { 1, 0, 0 }, deltaPitch);
        Gs::RotateFree(deltaRotation, { 0, 1, 0 }, deltaYaw);
        animation.rotation = deltaRotation * animation.rotation;

        // Transform scene mesh
        sceneSettings.wMatrix.LoadIdentity();
        Gs::Translate(sceneSettings.wMatrix, { 0, 0, 5 });
        sceneSettings.wMatrix *= animation.rotation;

        // Set colors and matrix
        sceneSettings.diffuse       = { 0.6f, 0.6f, 0.6f, 1.0f };
        sceneSettings.glossiness    = { 0, 0, 0, 0 };
        sceneSettings.wvpMatrix     = projection * sceneSettings.wMatrix;

        // Update constant buffer for scene settings
        commands->UpdateBuffer(*constantBufferScene, 0, &sceneSettings, sizeof(sceneSettings));
    }

    void SetBlurSettings(const Gs::Vector2f& blurShift)
    {
        // Update constant buffer for blur pass
        blurSettings.blurShift = blurShift;
        commands->UpdateBuffer(*constantBufferBlur, 0, &blurSettings, sizeof(blurSettings));
    }

    void OnResize(const LLGL::Extent2D& /*resolution*/) override
    {
        UpdateScreenSize();
    }

    void OnDrawFrame() override
    {
        #ifdef DEBUG_FPS
        // Show frame time
        static std::unique_ptr<LLGL::Timer> frameTimer;
        static std::chrono::time_point<std::chrono::system_clock> printTime;
        if (!frameTimer)
            frameTimer = LLGL::Timer::Create();

        frameTimer->MeasureTime();

        auto currentTime = std::chrono::system_clock::now();
        auto elapsedTime = (currentTime - printTime);
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

        if (elapsedMs > 250)
        {
            printf("Elapsed Time = %fms (FPS = %f)\n", static_cast<float>(frameTimer->GetDeltaTime()*1000.0f), static_cast<float>(1.0 / frameTimer->GetDeltaTime()));
            printTime = currentTime;
        }
        #endif

        // Update rotation of inner model
        animation.innerModelRotation += 0.01f;

        // Update rotation of outer model
        Gs::Vector2f mouseMotion
        {
            static_cast<float>(input.GetMouseMotion().x),
            static_cast<float>(input.GetMouseMotion().y),
        };

        Gs::Vector2f outerModelDeltaRotation;
        if (input.KeyPressed(LLGL::Key::LButton))
            outerModelDeltaRotation = mouseMotion*0.005f;

        // Update effect intensity animation
        if (input.KeyPressed(LLGL::Key::RButton))
        {
            float delta = mouseMotion.x*0.01f;
            sceneSettings.intensity = std::max(0.0f, std::min(sceneSettings.intensity + delta, 3.0f));
            LLGL::Log::Printf("glow intensity: %d%%    \r", static_cast<int>(sceneSettings.intensity*100.0f));
        }

        // Initialize viewports
        const auto screenSize = swapChain->GetResolution();

        const LLGL::Viewport viewportFull{ { 0, 0 }, screenSize };
        const LLGL::Viewport viewportQuarter{ { 0, 0 }, { screenSize.width / 4, screenSize.height/ 4 } };

        commands->Begin();
        {
            // Set graphics pipeline and vertex buffer for scene rendering
            commands->SetVertexBuffer(*vertexBufferScene);

            #if ENABLE_CUSTOM_RENDER_PASS

            // Clear scene render target with render pass and initial clear values
            LLGL::ClearValue clearValues[3];
            clearValues[0].color[0] = backgroundColor[0];
            clearValues[0].color[1] = backgroundColor[1];
            clearValues[0].color[2] = backgroundColor[2];
            clearValues[0].color[3] = backgroundColor[3];
            clearValues[1].color[3] = 1.0f;
            clearValues[2].depth    = 1.0f;

            // Draw scene into multi-render-target (1st target: color, 2nd target: glossiness)
            commands->BeginRenderPass(*renderTargetScene, renderPassScene, 3, clearValues);

            #else

            commands->BeginRenderPass(*renderTargetScene);

            #endif // /ENABLE_CUSTOM_RENDER_PASS
            {
                // Set viewport to full size
                commands->SetViewport(viewportFull);

                #if !ENABLE_CUSTOM_RENDER_PASS

                // Clear individual buffers in render target (color, glossiness, depth)
                constexpr float glossinessColor[4] = { 0, 0, 0, 0 };
                LLGL::AttachmentClear clearCmds[3] =
                {
                    LLGL::AttachmentClear{ backgroundColor, 0 },
                    LLGL::AttachmentClear{ glossinessColor, 1 },
                    LLGL::AttachmentClear{ 1.0f }
                };
                commands->ClearAttachments(3, clearCmds);

                #endif // /ENABLE_CUSTOM_RENDER_PASS

                // Bind pipeline and resources
                commands->SetPipelineState(*pipelineScene);
                commands->SetResourceHeap(*resourceHeapScene);

                // Draw outer scene model
                SetSceneSettingsOuterModel(outerModelDeltaRotation.y, outerModelDeltaRotation.x);
                commands->Draw(numSceneVertices, 0);

                // Draw inner scene model
                SetSceneSettingsInnerModel(animation.innerModelRotation);
                commands->Draw(numSceneVertices, 0);
            }
            commands->EndRenderPass();

            // Set graphics pipeline and vertex buffer for post-processors
            commands->SetVertexBuffer(*vertexBufferNull);

            // Draw horizontal blur pass
            SetBlurSettings({ 4.0f / static_cast<float>(screenSize.width), 0.0f });
            commands->BeginRenderPass(*renderTargetBlurX);
            {
                // Draw blur passes in quarter resolution
                commands->SetViewport(viewportQuarter);

                // Draw fullscreen triangle (triangle is spanned in the vertex shader)
                commands->SetPipelineState(*pipelineBlur);
                commands->SetResourceHeap(*resourceHeapBlur, 0);
                commands->Draw(3, 0);
            }
            commands->EndRenderPass();

            // Draw vertical blur pass
            SetBlurSettings({ 0.0f, 4.0f / static_cast<float>(screenSize.height) });
            commands->BeginRenderPass(*renderTargetBlurY);
            {
                // Draw fullscreen triangle (triangle is spanned in the vertex shader)
                commands->SetResourceHeap(*resourceHeapBlur, 1);
                commands->Draw(3, 0);
            }
            commands->EndRenderPass();

            // Draw final post-processing pass
            commands->BeginRenderPass(*swapChain);
            {
                // Set viewport back to full resolution
                commands->SetViewport(viewportFull);
                commands->SetPipelineState(*pipelineFinal);
                commands->SetResourceHeap(*resourceHeapFinal);

                // Draw fullscreen triangle (triangle is spanned in the vertex shader)
                commands->Draw(3, 0);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_PostProcessing);



