/*
 * Example.cpp (Example_VolumeRendering)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <PerlinNoise.h>


class Example_VolumeRendering : public ExampleBase
{

    const std::uint32_t         noiseTextureSize        = 64;

    LLGL::ShaderProgram*        shaderProgramDepthOnly  = nullptr;
    LLGL::ShaderProgram*        shaderProgramFinalPass  = nullptr;

    LLGL::PipelineLayout*       pipelineLayoutCbuffer   = nullptr;
    LLGL::PipelineLayout*       pipelineLayoutFinalPass = nullptr;

    LLGL::PipelineState*        pipelineRangePass       = nullptr;
    LLGL::PipelineState*        pipelineZPrePass        = nullptr;
    LLGL::PipelineState*        pipelineFinalPass       = nullptr;

    LLGL::ResourceHeap*         resourceHeapCbuffer     = nullptr;
    LLGL::ResourceHeap*         resourceHeapFinalPass   = nullptr;

    LLGL::Buffer*               vertexBuffer            = nullptr;
    LLGL::Buffer*               constantBuffer          = nullptr;

    LLGL::Texture*              noiseTexture            = nullptr;  // 3D noise texture
    LLGL::Sampler*              linearSampler           = nullptr;

    LLGL::Texture*              depthRangeTexture       = nullptr;
    LLGL::RenderTarget*         depthRangeRenderTarget  = nullptr;

    TriangleMesh                mesh;
    Gs::Matrix4f                rotation;

    PerlinNoise                 perlinNoise;

    struct Settings
    {
        Gs::Matrix4f            wMatrix;
        Gs::Matrix4f            wMatrixInv;
        Gs::Matrix4f            vpMatrix;
        Gs::Matrix4f            vpMatrixInv;
        Gs::Vector3f            lightDir                = Gs::Vector3f(-0.25f, -0.7f, 1.25f).Normalized();
        float                   shininess               = 55.0f;                        // Blinn-phong specular power factor
        Gs::Vector3f            viewPos;                                                // World-space camera position
        float                   threshold               = 0.1f;                         // Density threshold in the range [0, 0.5].
        LLGL::ColorRGBf         albedo                  = { 0.5f, 0.6f, 1.0f };         // Albedo material color
        float                   reflectance             = 0.4f;                         // Specular reflectance intensity
    }
    settings;

public:

    Example_VolumeRendering() :
        ExampleBase { L"LLGL Example: VolumeRendering", { 800, 600 }, 0 }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreateTextures();
        CreateSamplers();
        CreatePipelineLayouts();
        CreatePipelines();
        CreateResourceHeaps();

        // Show some information
        std::cout << "press LEFT MOUSE BUTTON and move the mouse to ROTATE the model" << std::endl;
        std::cout << "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to change the DENSITY THRESHOLD" << std::endl;
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
        mesh = LoadObjModel(vertices, "../../Media/Models/Suzanne.obj");

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader programs
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            shaderProgramDepthOnly = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VScene", "vs_5_0" },
                },
                { vertexFormat }
            );
            shaderProgramFinalPass = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VScene", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "Example.hlsl", "PScene", "ps_5_0" },
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            shaderProgramDepthOnly = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.vert" },
                },
                { vertexFormat }
            );
            shaderProgramFinalPass = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.vert" },
                    { LLGL::ShaderType::Fragment, "Example.frag" },
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderProgramDepthOnly = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.450core.vert.spv" },
                },
                { vertexFormat }
            );
            shaderProgramFinalPass = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.450core.vert.spv" },
                    { LLGL::ShaderType::Fragment, "Example.450core.frag.spv" },
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            shaderProgramDepthOnly = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.metal", "VScene", "1.1" },
                },
                { vertexFormat }
            );
            shaderProgramFinalPass = LoadShaderProgram(
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

    void CreateDepthRangeTextureAndRenderTarget(const LLGL::Extent2D& resolution)
    {
        // Release previous resources
        if (depthRangeTexture != nullptr)
        {
            renderer->Release(*depthRangeTexture);
            depthRangeTexture = nullptr;
        }

        if (depthRangeRenderTarget != nullptr)
        {
            renderer->Release(*depthRangeRenderTarget);
            depthRangeRenderTarget = nullptr;
        }

        // Create depth texture
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type        = LLGL::TextureType::Texture2D;
            texDesc.bindFlags   = LLGL::BindFlags::DepthStencilAttachment | LLGL::BindFlags::Sampled;
            texDesc.miscFlags   = LLGL::MiscFlags::NoInitialData;
            texDesc.format      = LLGL::Format::D32Float;
            texDesc.extent      = { resolution.width, resolution.height, 1 };
            texDesc.mipLevels   = 1;
        }
        depthRangeTexture = renderer->CreateTexture(texDesc);

        // Create render target
        LLGL::RenderTargetDescriptor rtDesc;
        {
            rtDesc.resolution       = resolution;
            //rtDesc.multiSampling    = GetMultiSampleDesc();
            rtDesc.attachments      =
            {
                LLGL::AttachmentDescriptor{ LLGL::AttachmentType::Depth, depthRangeTexture }
            };
        }
        depthRangeRenderTarget = renderer->CreateRenderTarget(rtDesc);
    }

    void CreateTextures()
    {
        // Generate 3D perlin noise texture
        std::vector<std::uint8_t> imageData;
        {
            perlinNoise.GenerateBuffer(imageData, noiseTextureSize, noiseTextureSize, noiseTextureSize, 4);
        }
        LLGL::SrcImageDescriptor imageDesc;
        {
            imageDesc.format    = LLGL::ImageFormat::R;
            imageDesc.dataType  = LLGL::DataType::UInt8;
            imageDesc.data      = imageData.data();
            imageDesc.dataSize  = imageData.size();
        }
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.type        = LLGL::TextureType::Texture3D;
            texDesc.format      = LLGL::Format::R8UNorm;
            texDesc.extent      = { noiseTextureSize, noiseTextureSize, noiseTextureSize };
            texDesc.mipLevels   = 1;
        }
        noiseTexture = renderer->CreateTexture(texDesc, &imageDesc);

        // Create render target texture for depth-range
        CreateDepthRangeTextureAndRenderTarget(context->GetResolution());
    }

    void CreateSamplers()
    {
        // Create default sampler state
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.mipMapping = false;
        }
        linearSampler = renderer->CreateSampler(samplerDesc);
    }

    void CreatePipelineLayouts()
    {
        // Create pipeline layout with only a single constnat buffer for depth-range pass and Z-pre pass
        pipelineLayoutCbuffer = renderer->CreatePipelineLayout(
            LLGL::PipelineLayoutDesc("cbuffer(Settings@1):frag:vert")
        );

        // Create pipeline layout for final scene rendering
        if (IsOpenGL())
        {
            pipelineLayoutFinalPass = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(Settings@1):frag:vert, texture(noiseTexture@2):frag, texture(depthRangeTexture@3):frag, sampler(2):frag, sampler(3):frag")
            );
        }
        else
        {
            pipelineLayoutFinalPass = renderer->CreatePipelineLayout(
                LLGL::PipelineLayoutDesc("cbuffer(Settings@1):frag:vert, texture(noiseTexture@2):frag, texture(depthRangeTexture@3):frag, sampler(linearSampler@4):frag")
            );
        }
    }

    void CreatePipelines()
    {
        // Create graphics pipeline for depth-range pass
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram              = shaderProgramDepthOnly;
                pipelineDesc.renderPass                 = depthRangeRenderTarget->GetRenderPass();
                pipelineDesc.pipelineLayout             = pipelineLayoutCbuffer;
                pipelineDesc.depth.testEnabled          = true;
                pipelineDesc.depth.writeEnabled         = true;
                pipelineDesc.depth.compareOp            = LLGL::CompareOp::Greater;
                pipelineDesc.rasterizer.cullMode        = LLGL::CullMode::Front;
                //pipelineDesc.rasterizer.multiSampling   = GetMultiSampleDesc();
                pipelineDesc.blend.targets[0].colorMask = { false, false, false, false };
            }
            pipelineRangePass = renderer->CreatePipelineState(pipelineDesc);
        }

        // Create graphics pipeline for Z-pre pass
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram              = shaderProgramDepthOnly;
                pipelineDesc.renderPass                 = context->GetRenderPass();
                pipelineDesc.pipelineLayout             = pipelineLayoutCbuffer;
                pipelineDesc.depth.testEnabled          = true;
                pipelineDesc.depth.writeEnabled         = true;
                pipelineDesc.depth.compareOp            = LLGL::CompareOp::Less;
                pipelineDesc.rasterizer.cullMode        = LLGL::CullMode::Back;
                //pipelineDesc.rasterizer.multiSampling   = GetMultiSampleDesc();
                pipelineDesc.blend.targets[0].colorMask = { false, false, false, false };
            }
            pipelineZPrePass = renderer->CreatePipelineState(pipelineDesc);
        }

        // Create graphics pipeline for final scene rendering
        {
            LLGL::GraphicsPipelineDescriptor pipelineDesc;
            {
                pipelineDesc.shaderProgram              = shaderProgramFinalPass;
                pipelineDesc.renderPass                 = context->GetRenderPass();
                pipelineDesc.pipelineLayout             = pipelineLayoutFinalPass;
                pipelineDesc.depth.testEnabled          = true;
                pipelineDesc.depth.writeEnabled         = false;
                pipelineDesc.depth.compareOp            = LLGL::CompareOp::Equal;
                pipelineDesc.rasterizer.cullMode        = LLGL::CullMode::Back;
                //pipelineDesc.rasterizer.multiSampling   = GetMultiSampleDesc();

                auto& blendTarget = pipelineDesc.blend.targets[0];
                blendTarget.blendEnabled                = true;
                blendTarget.dstAlpha                    = LLGL::BlendOp::One;
                blendTarget.srcAlpha                    = LLGL::BlendOp::SrcAlpha;
                blendTarget.dstColor                    = LLGL::BlendOp::One;
                blendTarget.srcColor                    = LLGL::BlendOp::SrcAlpha;
            }
            pipelineFinalPass = renderer->CreatePipelineState(pipelineDesc);
        }
    }

    void CreateResourceHeaps()
    {
        // Release only previous resource heaps that refer to resources that are window size dependent
        if (resourceHeapFinalPass != nullptr)
        {
            renderer->Release(*resourceHeapFinalPass);
            resourceHeapFinalPass = nullptr;
        }

        // Create resource heap for Z-pre pass
        if (resourceHeapCbuffer == nullptr)
        {
            LLGL::ResourceHeapDescriptor resourceHeapDesc;
            {
                resourceHeapDesc.pipelineLayout = pipelineLayoutCbuffer;
                resourceHeapDesc.resourceViews  = { constantBuffer };
            }
            resourceHeapCbuffer = renderer->CreateResourceHeap(resourceHeapDesc);
        }

        // Create resource heap for scene rendering
        {
            LLGL::ResourceHeapDescriptor resourceHeapDesc;
            {
                resourceHeapDesc.pipelineLayout = pipelineLayoutFinalPass;
                if (IsOpenGL())
                    resourceHeapDesc.resourceViews = { constantBuffer, noiseTexture, depthRangeTexture, linearSampler, linearSampler };
                else
                    resourceHeapDesc.resourceViews = { constantBuffer, noiseTexture, depthRangeTexture, linearSampler };
            }
            resourceHeapFinalPass = renderer->CreateResourceHeap(resourceHeapDesc);
        }
    }

    void UpdateScene()
    {
        // Update input
        const Gs::Vector2f mouseMotion
        {
            static_cast<float>(input->GetMouseMotion().x),
            static_cast<float>(input->GetMouseMotion().y),
        };

        Gs::Vector2f rotationVec;
        if (input->KeyPressed(LLGL::Key::LButton))
            rotationVec = mouseMotion*0.005f;

        // Update density threshold
        if (input->KeyPressed(LLGL::Key::RButton))
        {
            float delta = mouseMotion.x*0.002f;
            settings.threshold = std::max(0.0f, std::min(settings.threshold + delta, 0.5f));
            std::cout << "density threshold: " << static_cast<int>(settings.threshold*200.0f) << "%    \r";
            std::flush(std::cout);
        }

        // Rotate model around X and Y axes
        Gs::Matrix4f deltaRotation;
        Gs::RotateFree(deltaRotation, { 1, 0, 0 }, rotationVec.y);
        Gs::RotateFree(deltaRotation, { 0, 1, 0 }, rotationVec.x);
        rotation = deltaRotation * rotation;

        // Transform scene mesh
        settings.wMatrix.LoadIdentity();
        Gs::Translate(settings.wMatrix, { 0, 0, 5 });
        settings.wMatrix *= rotation;

        settings.wMatrixInv = settings.wMatrix.Inverse();

        // Update view-projection matrix
        settings.vpMatrix       = projection;
        settings.vpMatrixInv    = projection.Inverse();
    }

    void OnResize(const LLGL::Extent2D& resolution) override
    {
        // Re-create depth-range texture and its render target.
        CreateDepthRangeTextureAndRenderTarget(resolution);

        // Also re-create resource haps that refer to the re-created depth-texture
        CreateResourceHeaps();
    }

    void OnDrawFrame() override
    {
        // Update scene by user input
        UpdateScene();

        commands->Begin();
        {
            // Bind vertex input assembly and update constant buffer with scene settings
            commands->SetVertexBuffer(*vertexBuffer);
            commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

            // Render maximum scene depth into render target
            commands->BeginRenderPass(*depthRangeRenderTarget);
            {
                commands->SetClearDepth(0.0f);
                commands->Clear(LLGL::ClearFlags::ColorDepth);
                commands->SetViewport(depthRangeRenderTarget->GetResolution());

                // Render depth-range pass
                commands->PushDebugGroup("Range Pass");
                {
                    commands->SetPipelineState(*pipelineRangePass);
                    commands->SetResourceHeap(*resourceHeapCbuffer);
                    commands->Draw(mesh.numVertices, mesh.firstVertex);
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();

            commands->ResetResourceSlots(LLGL::ResourceType::Texture, 3, 1, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage);

            // Render everything directly into the render context
            commands->BeginRenderPass(*context);
            {
                commands->SetClearDepth(1.0f);
                commands->Clear(LLGL::ClearFlags::ColorDepth);
                commands->SetViewport(context->GetResolution());

                // Render Z-pre pass
                commands->PushDebugGroup("Z-Pre Pass");
                {
                    commands->SetPipelineState(*pipelineZPrePass);
                    commands->SetResourceHeap(*resourceHeapCbuffer);
                    commands->Draw(mesh.numVertices, mesh.firstVertex);
                }
                commands->PopDebugGroup();

                // Render final scene pass
                commands->PushDebugGroup("Final Pass");
                {
                    commands->SetPipelineState(*pipelineFinalPass);
                    commands->SetResourceHeap(*resourceHeapFinalPass);
                    commands->Draw(mesh.numVertices, mesh.firstVertex);
                }
                commands->PopDebugGroup();
            }
            commands->EndRenderPass();

            commands->ResetResourceSlots(LLGL::ResourceType::Texture, 3, 1, LLGL::BindFlags::Sampled, LLGL::StageFlags::FragmentStage);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_VolumeRendering);



