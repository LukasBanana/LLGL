/*
 * main.cpp (Tutorial11_PostProcessing)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <tutorial.h>
#include <chrono>//!!!


class Tutorial11 : public Tutorial
{

    const LLGL::ColorRGBAf  glowColor           = { 0.9f, 0.7f, 0.3f, 1.0f };

    LLGL::ShaderProgram*    shaderProgramScene  = nullptr;
    LLGL::ShaderProgram*    shaderProgramBlur   = nullptr;
    LLGL::ShaderProgram*    shaderProgramFinal  = nullptr;

    LLGL::PipelineLayout*   layoutScene         = nullptr;
    LLGL::PipelineLayout*   layoutBlur          = nullptr;
    LLGL::PipelineLayout*   layoutFinal         = nullptr;

    LLGL::GraphicsPipeline* pipelineScene       = nullptr;
    LLGL::GraphicsPipeline* pipelineBlur        = nullptr;
    LLGL::GraphicsPipeline* pipelineFinal       = nullptr;

    LLGL::ResourceHeap*     resourceHeapScene   = nullptr;
    LLGL::ResourceHeap*     resourceHeapBlurX   = nullptr;
    LLGL::ResourceHeap*     resourceHeapBlurY   = nullptr;
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

public:

    Tutorial11() :
        Tutorial { L"LLGL Tutorial 11: PostProcessing", { 800, 600 }, 0 }//, false }
    {
        // Create all graphics objects
        CreateBuffers();
        LoadShaders();
        CreateSamplers();
        CreateTextures();
        CreateRenderTargets();
        CreatePipelineLayouts();
        CreatePipelines();
        CreateResourceHeaps();

        // Show some information
        std::cout << "press LEFT MOUSE BUTTON and move the mouse to rotate the outer box" << std::endl;
        std::cout << "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to change the glow intensity" << std::endl;
    }

    void CreateBuffers()
    {
        // Specify vertex format for scene
        vertexFormatScene.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormatScene.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });

        // Create scene buffers
        auto sceneVertices = LoadObjModel("../Media/Models/WiredBox.obj");
        numSceneVertices = static_cast<std::uint32_t>(sceneVertices.size());

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
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            // Load scene shader program
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "shader.hlsl", "VScene", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PScene", "ps_5_0" }
                },
                { vertexFormatScene }
            );

            // Load blur shader program
            shaderProgramBlur = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "shader.hlsl", "VPP",   "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PBlur", "ps_5_0" }
                }
            );

            // Load final shader program
            shaderProgramFinal = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "shader.hlsl", "VPP",    "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PFinal", "ps_5_0" }
                }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            // Load scene shader program
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "scene.vertex.glsl"   },
                    { LLGL::ShaderType::Fragment, "scene.fragment.glsl" }
                },
                { vertexFormatScene }
            );

            // Load blur shader program
            shaderProgramBlur = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "postprocess.vertex.glsl" },
                    { LLGL::ShaderType::Fragment, "blur.fragment.glsl"      }
                }
            );

            // Load final shader program
            shaderProgramFinal = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "postprocess.vertex.glsl" },
                    { LLGL::ShaderType::Fragment, "final.fragment.glsl"     }
                }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            // Load scene shader program
            shaderProgramScene = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "scene.vertex.spv"   },
                    { LLGL::ShaderType::Fragment, "scene.fragment.spv" }
                },
                { vertexFormatScene }
            );

            // Load blur shader program
            shaderProgramBlur = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "postprocess.vertex.spv" },
                    { LLGL::ShaderType::Fragment, "blur.fragment.spv"      }
                }
            );

            // Load final shader program
            shaderProgramFinal = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "postprocess.vertex.spv" },
                    { LLGL::ShaderType::Fragment, "final.fragment.spv"     }
                }
            );
        }

        // Set shader uniforms (only required for GLSL until 4.10)
        shaderProgramBlur->BindConstantBuffer("BlurSettings", 1);

        if (auto uniforms = shaderProgramBlur->LockShaderUniform())
        {
            uniforms->SetUniform1i("colorMap", 2);
            uniforms->SetUniform1i("glossMap", 3);
            shaderProgramBlur->UnlockShaderUniform();
        }

        if (auto uniforms = shaderProgramFinal->LockShaderUniform())
        {
            uniforms->SetUniform1i("colorMap", 2);
            uniforms->SetUniform1i("glossMap", 3);
            shaderProgramFinal->UnlockShaderUniform();
        }
    }

    void CreateSamplers()
    {
        // Create sampler states for all textures
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.mipMapping = false;
        }
        colorMapSampler = renderer->CreateSampler(samplerDesc);
        glossMapSampler = renderer->CreateSampler(samplerDesc);
    }

    void CreateTextures()
    {
        // Create empty color and gloss map
        auto resolution = context->GetVideoMode().resolution;
        colorMap        = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, resolution.width, resolution.height));
        glossMap        = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, resolution.width, resolution.height));

        // Create empty blur pass maps (in quarter resolution)
        resolution.width  /= 4;
        resolution.height /= 4;
        glossMapBlurX   = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, resolution.width, resolution.height));
        glossMapBlurY   = renderer->CreateTexture(LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, resolution.width, resolution.height));
    }

    void CreateRenderTargets()
    {
        auto resolution = context->GetVideoMode().resolution;

        // Create render-target for scene rendering
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.resolution = resolution;
            renderTargetDesc.attachments =
            {
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Depth },
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Color, colorMap },
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Color, glossMap },
            };
            renderTargetDesc.multiSampling = LLGL::MultiSamplingDescriptor(8);
        }
        renderTargetScene = renderer->CreateRenderTarget(renderTargetDesc);

        // Create render-target for horizontal blur pass (no depth buffer needed)
        resolution.width    /= 4;
        resolution.height   /= 4;

        LLGL::RenderTargetDescriptor renderTargetBlurXDesc;
        {
            renderTargetBlurXDesc.resolution = resolution;
            renderTargetBlurXDesc.attachments =
            {
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Color, glossMapBlurX }
            };
        }
        renderTargetBlurX = renderer->CreateRenderTarget(renderTargetBlurXDesc);

        // Create render-target for vertical blur pass (no depth buffer needed)
        LLGL::RenderTargetDescriptor renderTargetBlurYDesc;
        {
            renderTargetBlurYDesc.resolution = resolution;
            renderTargetBlurYDesc.attachments =
            {
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Color, glossMapBlurY }
            };
        }
        renderTargetBlurY = renderer->CreateRenderTarget(renderTargetBlurYDesc);
    }

    // The utility function <LLGL::PipelineLayoutDesc> is used here, to simplify the description of the pipeline layouts
    void CreatePipelineLayouts()
    {
        bool combinedSampler = IsOpenGL();

        // Create pipeline layout for scene rendering
        layoutScene = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(0):vert:frag"));

        // Create pipeline layout for blur post-processor
        if (combinedSampler)
            layoutBlur = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(1):frag, texture(3):frag, sampler(3):frag"));
        else
            layoutBlur = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(1):frag, texture(3):frag, sampler(5):frag"));

        // Create pipeline layout for final post-processor
        if (combinedSampler)
            layoutFinal = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(0):frag, texture(2,3):frag, sampler(2,3):frag"));
        else
            layoutFinal = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(0):frag, texture(2,3):frag, sampler(4,5):frag"));
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDescScene;
        {
            pipelineDescScene.shaderProgram             = shaderProgramScene;
            pipelineDescScene.renderPass                = renderTargetScene->GetRenderPass();
            pipelineDescScene.pipelineLayout            = layoutScene;

            pipelineDescScene.depth.testEnabled         = true;
            pipelineDescScene.depth.writeEnabled        = true;

            pipelineDescScene.rasterizer.cullMode       = LLGL::CullMode::Back;
            pipelineDescScene.rasterizer.multiSampling  = LLGL::MultiSamplingDescriptor(8);
        }
        pipelineScene = renderer->CreateGraphicsPipeline(pipelineDescScene);

        // Create graphics pipeline for blur post-processor
        LLGL::GraphicsPipelineDescriptor pipelineDescPP;
        {
            pipelineDescPP.shaderProgram    = shaderProgramBlur;
            pipelineDescPP.renderPass       = renderTargetBlurX->GetRenderPass();
            pipelineDescPP.pipelineLayout   = layoutBlur;
        }
        pipelineBlur = renderer->CreateGraphicsPipeline(pipelineDescPP);

        // Create graphics pipeline for final post-processor
        LLGL::GraphicsPipelineDescriptor pipelineDescFinal;
        {
            pipelineDescFinal.shaderProgram     = shaderProgramFinal;
            pipelineDescFinal.pipelineLayout    = layoutFinal;
            pipelineDescFinal.renderPass        = context->GetRenderPass();
        }
        pipelineFinal = renderer->CreateGraphicsPipeline(pipelineDescFinal);
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for scene rendering
        LLGL::ResourceHeapDescriptor heapDescScene;
        {
            heapDescScene.pipelineLayout    = layoutScene;
            heapDescScene.resourceViews     = { constantBufferScene };
        }
        resourceHeapScene = renderer->CreateResourceHeap(heapDescScene);

        // Create resource heap for blur-X post-processor
        LLGL::ResourceHeapDescriptor heapDescBlurX;
        {
            heapDescBlurX.pipelineLayout    = layoutBlur;
            heapDescBlurX.resourceViews     = { constantBufferBlur, glossMap, glossMapSampler };
        }
        resourceHeapBlurX = renderer->CreateResourceHeap(heapDescBlurX);

        // Create resource heap for blur-Y post-processor
        LLGL::ResourceHeapDescriptor heapDescBlurY;
        {
            heapDescBlurY.pipelineLayout    = layoutBlur;
            heapDescBlurY.resourceViews     = { constantBufferBlur, glossMapBlurX, glossMapSampler };
        }
        resourceHeapBlurY = renderer->CreateResourceHeap(heapDescBlurY);

        // Create resource heap for final post-processor
        LLGL::ResourceHeapDescriptor heapDescFinal;
        {
            heapDescFinal.pipelineLayout    = layoutFinal;
            heapDescFinal.resourceViews     = { constantBufferScene, colorMap, glossMapBlurY, colorMapSampler, glossMapSampler };
        }
        resourceHeapFinal = renderer->CreateResourceHeap(heapDescFinal);
    }

    void UpdateScreenSize()
    {
        // Release previous textures
        renderer->Release(*renderTargetScene);
        renderer->Release(*renderTargetBlurX);
        renderer->Release(*renderTargetBlurY);

        renderer->Release(*resourceHeapScene);
        renderer->Release(*resourceHeapBlurX);
        renderer->Release(*resourceHeapBlurY);
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

#define TEST_UPDATE_BUFFER

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
        #ifdef TEST_UPDATE_BUFFER
        commands->UpdateBuffer(*constantBufferScene, 0, &sceneSettings, sizeof(sceneSettings));
        #else
        UpdateBuffer(constantBufferScene, sceneSettings);
        #endif
    }

    void SetSceneSettingsOuterModel(float deltaPitch, float deltaYaw)
    {
        // Rotate model around X and Y axes
        static Gs::Matrix4f rotation;

        Gs::Matrix4f deltaRotation;
        Gs::RotateFree(deltaRotation, { 1, 0, 0 }, deltaPitch);
        Gs::RotateFree(deltaRotation, { 0, 1, 0 }, deltaYaw);
        rotation = deltaRotation * rotation;

        // Transform scene mesh
        sceneSettings.wMatrix.LoadIdentity();
        Gs::Translate(sceneSettings.wMatrix, { 0, 0, 5 });
        sceneSettings.wMatrix *= rotation;

        // Set colors and matrix
        sceneSettings.diffuse       = { 0.6f, 0.6f, 0.6f, 1.0f };
        sceneSettings.glossiness    = { 0, 0, 0, 0 };
        sceneSettings.wvpMatrix     = projection * sceneSettings.wMatrix;

        // Update constant buffer for scene settings
        #ifdef TEST_UPDATE_BUFFER
        commands->UpdateBuffer(*constantBufferScene, 0, &sceneSettings, sizeof(sceneSettings));
        #else
        UpdateBuffer(constantBufferScene, sceneSettings);
        #endif
    }

    void SetBlurSettings(const Gs::Vector2f& blurShift)
    {
        // Update constant buffer for blur pass
        blurSettings.blurShift = blurShift;
        #ifdef TEST_UPDATE_BUFFER
        commands->UpdateBuffer(*constantBufferBlur, 0, &blurSettings, sizeof(blurSettings));
        #else
        UpdateBuffer(constantBufferBlur, blurSettings);
        #endif
    }

    void OnDrawFrame() override
    {
        #if 0
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
        static float innerModelRotation;
        innerModelRotation += 0.01f;

        // Update rotation of outer model
        Gs::Vector2f mouseMotion
        {
            static_cast<float>(input->GetMouseMotion().x),
            static_cast<float>(input->GetMouseMotion().y),
        };

        Gs::Vector2f outerModelDeltaRotation;
        if (input->KeyPressed(LLGL::Key::LButton))
            outerModelDeltaRotation = mouseMotion*0.005f;

        // Update effect intensity animation
        if (input->KeyPressed(LLGL::Key::RButton))
        {
            float delta = mouseMotion.x*0.01f;
            sceneSettings.intensity = std::max(0.0f, std::min(sceneSettings.intensity + delta, 3.0f));
            std::cout << "glow intensity: " << static_cast<int>(sceneSettings.intensity*100.0f) << "%    \r";
            std::flush(std::cout);
        }

        // Check if screen size has changed (this could also be done with an event listener)
        static LLGL::Extent2D screenSize { 800, 600 };

        if (screenSize != context->GetVideoMode().resolution)
        {
            screenSize = context->GetVideoMode().resolution;
            UpdateScreenSize();
        }

        // Initialize viewports
        const LLGL::Viewport viewportFull{ { 0, 0 }, screenSize };
        const LLGL::Viewport viewportQuarter{ { 0, 0 }, { screenSize.width / 4, screenSize.height/ 4 } };

        commands->Begin();
        {
            // Set graphics pipeline and vertex buffer for scene rendering
            commands->SetVertexBuffer(*vertexBufferScene);

            // Draw scene into multi-render-target (1st target: color, 2nd target: glossiness)
            SetSceneSettingsOuterModel(outerModelDeltaRotation.y, outerModelDeltaRotation.x);
            commands->BeginRenderPass(*renderTargetScene);
            {
                // Set viewport to full size
                commands->SetViewport(viewportFull);

                // Clear individual buffers in render target (color, glossiness, depth)
                LLGL::AttachmentClear clearCmds[3] =
                {
                    LLGL::AttachmentClear { defaultClearColor, 0 },
                    LLGL::AttachmentClear { LLGL::ColorRGBAf { 0, 0, 0, 0 }, 1 },
                    LLGL::AttachmentClear { 1.0f }
                };
                commands->ClearAttachments(3, clearCmds);

                // Bind pipeline and resources
                commands->SetGraphicsPipeline(*pipelineScene);
                commands->SetGraphicsResourceHeap(*resourceHeapScene);

                // Draw outer scene model
                commands->Draw(numSceneVertices, 0);
            }
            commands->EndRenderPass();

            // Draw inner scene model
            SetSceneSettingsInnerModel(innerModelRotation);
            commands->BeginRenderPass(*renderTargetScene);
            {
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
                commands->SetGraphicsPipeline(*pipelineBlur);
                commands->SetGraphicsResourceHeap(*resourceHeapBlurX);
                commands->Draw(3, 0);
            }
            commands->EndRenderPass();

            // Draw vertical blur pass
            SetBlurSettings({ 0.0f, 4.0f / static_cast<float>(screenSize.height) });
            commands->BeginRenderPass(*renderTargetBlurY);
            {
                // Draw fullscreen triangle (triangle is spanned in the vertex shader)
                commands->SetGraphicsResourceHeap(*resourceHeapBlurY);
                commands->Draw(3, 0);
            }
            commands->EndRenderPass();

            // Draw final post-processing pass
            commands->BeginRenderPass(*context);
            {
                // Set viewport back to full resolution
                commands->SetViewport(viewportFull);
                commands->SetGraphicsPipeline(*pipelineFinal);
                commands->SetGraphicsResourceHeap(*resourceHeapFinal);

                // Draw fullscreen triangle (triangle is spanned in the vertex shader)
                commands->Draw(3, 0);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial11);



