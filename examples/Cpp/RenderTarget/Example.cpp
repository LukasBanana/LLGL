/*
 * Example.cpp (Example_RenderTarget)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Platform/Platform.h>


// Enable multi-sampling
#define ENABLE_MULTISAMPLING        1

// Enable custom multi-sampling by rendering directly into a multi-sample texture
#define ENABLE_CUSTOM_MULTISAMPLING (ENABLE_MULTISAMPLING && 0)

// Enable depth texture instead of depth buffer for render target
#define ENABLE_DEPTH_TEXTURE        0

// Enables the resource heap. Otherwise, all resources are bound to the graphics pipeline individually.
#define ENABLE_RESOURCE_HEAP        0

//TODO: needs to be revised
// Enables constant buffer view (CBV) ranges
//#define ENABLE_CBUFFER_RANGE        (ENABLE_RESOURCE_HEAP && 0)


class Example_RenderTarget : public ExampleBase
{

    ShaderPipeline          shaderPipeline;

    LLGL::PipelineState*    pipelines[2]            = {};
    LLGL::PipelineLayout*   pipelineLayout          = nullptr;

    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           indexBuffer             = nullptr;
    LLGL::Buffer*           constantBuffer          = nullptr;

    LLGL::Texture*          colorMap                = nullptr;
    LLGL::Sampler*          samplerState            = nullptr;
    #if ENABLE_RESOURCE_HEAP
    LLGL::ResourceHeap*     resourceHeap            = nullptr;
    #endif

    LLGL::RenderTarget*     renderTarget            = nullptr;
    LLGL::Texture*          renderTargetTex         = nullptr;

    #if ENABLE_DEPTH_TEXTURE
    LLGL::Texture*          renderTargetDepthTex    = nullptr;
    #endif

    #if ENABLE_CUSTOM_MULTISAMPLING
    LLGL::Texture*          dummyTexMS              = nullptr;
    LLGL::Texture*          renderTargetTexMS       = nullptr;
    #endif

    Gs::Matrix4f            renderTargetProj;

    Gs::Vector2f            rotation                = { Gs::Deg2Rad(-20.0f), Gs::Deg2Rad(-30.0f) };

    #if ENABLE_CBUFFER_RANGE
    std::uint64_t           cbufferAlignment        = 0;
    std::vector<char>       cbufferData;
    #endif

    #if ENABLE_CUSTOM_MULTISAMPLING
    const LLGL::Extent2D    renderTargetSize        = { 64, 64 };
    #else
    const LLGL::Extent2D    renderTargetSize        = { 512, 512 };
    #endif

    struct alignas(16) Settings
    {
        Gs::Matrix4f        wvpMatrix;
        Gs::Matrix4f        wMatrix;
        int                 useTexture2DMS = 0;
    };

public:

    Example_RenderTarget() :
        ExampleBase { "LLGL Example: RenderTarget" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreateColorMap();
        CreateRenderTarget();
        CreatePipelines();
        #if ENABLE_RESOURCE_HEAP
        CreateResourceHeap();
        #endif

        // Show some information
        LLGL::Log::Printf(
            "press LEFT MOUSE BUTTON and move the mouse on the X-axis to rotate the OUTER cube\n"
            "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to rotate the INNER cube\n"
            "press RETURN KEY to save the render target texture to a PNG file\n"
        );
    }

private:

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });

        // Initialize vertices (scale texture-coordinates a little bit, to show the texture border)
        auto vertices = GenerateTexturedCubeVertices();

        constexpr float borderSize = 0.02f;
        for (auto& v : vertices)
            v.texCoord = (v.texCoord - Gs::Vector2f(0.5f))*(1.0f + borderSize) + Gs::Vector2f(0.5f);

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateTexturedCubeTriangleIndices(), LLGL::Format::R32UInt);

        #if ENABLE_CBUFFER_RANGE

        // Allocate CPU local buffer for shader settings with alignment (usually 256 bytes for constant buffers)
        const auto& caps = renderer->GetRenderingCaps();
        cbufferAlignment = std::max(sizeof(Settings), static_cast<std::size_t>(caps.limits.minConstantBufferAlignment));
        cbufferData.resize(static_cast<std::size_t>(cbufferAlignment * 2));

        // Create constant buffer for two <Settings> entries with alignment
        constantBuffer = renderer->CreateBuffer(LLGL::ConstantBufferDesc(cbufferAlignment * 2));

        #else

        // Create constant buffer for a single <Settings> entry
        constantBuffer = renderer->CreateBuffer(LLGL::ConstantBufferDesc(sizeof(Settings)));

        #endif

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        LLGL::ShaderMacro psDefines[] =
        {
            #if ENABLE_CUSTOM_MULTISAMPLING
            LLGL::ShaderMacro{ "ENABLE_CUSTOM_MULTISAMPLING" },
            #endif
            LLGL::ShaderMacro{}
        };

        // Load shader program
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            shaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" }, { vertexFormat });
            shaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" }, {}, psDefines);
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL) || Supported(LLGL::ShadingLanguage::ESSL))
        {
            // Patch clipping origin in vertex shader in case the GL server does not support GL_ARB_clip_control
            shaderPipeline.vs = LoadShaderAndPatchClippingOrigin({ LLGL::ShaderType::Vertex,   "Example.vert" }, { vertexFormat });
            shaderPipeline.ps = LoadShader(
                #ifdef __APPLE__
                { LLGL::ShaderType::Fragment, "Example.410core.frag" },
                #else
                { LLGL::ShaderType::Fragment, "Example.frag" },
                #endif
                {}, psDefines
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.450core.vert.spv" }, { vertexFormat });
            shaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.450core.frag.spv" });
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            shaderPipeline.vs = LoadShader({ LLGL::ShaderType::Vertex,   "Example.metal", "VS", "1.1" }, { vertexFormat });
            shaderPipeline.ps = LoadShader({ LLGL::ShaderType::Fragment, "Example.metal", "PS", "1.1" });
        }
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;
        {
            #if ENABLE_RESOURCE_HEAP
            layoutDesc.heapBindings =
            #else
            layoutDesc.bindings =
            #endif
            {
                LLGL::BindingDescriptor{ "Settings",        LLGL::ResourceType::Buffer,   LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage | LLGL::StageFlags::VertexStage, 3 },
                LLGL::BindingDescriptor{ "colorMapSampler", LLGL::ResourceType::Sampler,  0,                               LLGL::StageFlags::FragmentStage,                                 1 },
                LLGL::BindingDescriptor{ "colorMap",        LLGL::ResourceType::Texture,  LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 2 },
                #if ENABLE_CUSTOM_MULTISAMPLING
                LLGL::BindingDescriptor{ "colorMapMS",      LLGL::ResourceType::Texture,  LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 3 },
                #endif
            };
            layoutDesc.combinedTextureSamplers =
            {
                LLGL::CombinedTextureSamplerDescriptor{ "colorMap", "colorMap", "colorMapSampler", 2 }
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create graphics pipeline for swap-chain
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = shaderPipeline.vs;
            pipelineDesc.fragmentShader                 = shaderPipeline.ps;
            pipelineDesc.renderPass                     = swapChain->GetRenderPass();
            pipelineDesc.pipelineLayout                 = pipelineLayout;

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            // Enable culling of back-facing polygons
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipelines[1] = renderer->CreatePipelineState(pipelineDesc);
        ReportPSOErrors(pipelines[0]);

        // Create graphics pipeline for render target
        {
            pipelineDesc.renderPass = renderTarget->GetRenderPass();
            pipelineDesc.viewports  = { renderTarget->GetResolution() };

            #if ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampleEnabled = true;
            #else
            pipelineDesc.rasterizer.multiSampleEnabled = false;
            #endif
        }
        pipelines[0] = renderer->CreatePipelineState(pipelineDesc);
        ReportPSOErrors(pipelines[1]);
    }

    void CreateColorMap()
    {
        // Load color map texture from file
        colorMap = LoadTexture("Crate.jpg");

        // Create common sampler state for all textures
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.maxAnisotropy = 8;
        }
        samplerState = renderer->CreateSampler(samplerDesc);
    }

    void CreateRenderTarget()
    {
        // Initialize multisampling
        #if ENABLE_MULTISAMPLING
        const std::uint32_t samples = 8;
        #else
        const std::uint32_t samples = 1;
        #endif

        // Create empty render-target texture
        renderTargetTex = renderer->CreateTexture(
            LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, renderTargetSize.width, renderTargetSize.height)
        );
        renderTargetTex->SetDebugName("RenderTargetTex");

        #if ENABLE_DEPTH_TEXTURE

        // Create depth texture
        LLGL::TextureDescriptor depthTexDesc;
        {
            depthTexDesc.debugName      = "RenderTargetDepthTex";
            depthTexDesc.bindFlags      = LLGL::BindFlags::DepthStencilAttachment;
            depthTexDesc.format         = LLGL::Format::D32Float;
            depthTexDesc.extent.width   = renderTargetSize.width;
            depthTexDesc.extent.height  = renderTargetSize.height;
            depthTexDesc.mipLevels      = 1;
            depthTexDesc.samples        = samples;
            depthTexDesc.type           = (depthTexDesc.samples > 1 ? LLGL::TextureType::Texture2DMS : LLGL::TextureType::Texture2D);
            depthTexDesc.miscFlags      = LLGL::MiscFlags::NoInitialData;
        }
        renderTargetDepthTex = renderer->CreateTexture(depthTexDesc);

        #endif // /ENABLE_DEPTH_TEXTURE

        #if ENABLE_CUSTOM_MULTISAMPLING

        dummyTexMS = renderer->CreateTexture(
            LLGL::Texture2DMSDesc(LLGL::Format::R8UNorm, renderTargetSize.width, renderTargetSize.height, samples)
        );

        renderTargetTexMS = renderer->CreateTexture(
            LLGL::Texture2DMSDesc(LLGL::Format::RGBA8UNorm, renderTargetSize.width, renderTargetSize.height, samples)
        );

        #endif // /ENABLE_CUSTOM_MULTISAMPLING

        // Create render-target with multi-sampling
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.debugName  = "RenderTarget";
            renderTargetDesc.resolution = renderTargetSize;
            renderTargetDesc.samples    = samples;

            #if ENABLE_CUSTOM_MULTISAMPLING

            // Only render into custom multi-sampled texture
            renderTargetDesc.colorAttachments[0] = renderTargetTexMS;

            #else

            if (samples > 1)
            {
                // Render into multi-sampled texture (with same format), then resolve into our target texture
                renderTargetDesc.colorAttachments[0]    = renderTargetTex->GetFormat();
                renderTargetDesc.resolveAttachments[0]  = renderTargetTex;
            }
            else
            {
                // Render directly into target texture
                renderTargetDesc.colorAttachments[0] = renderTargetTex;
            }

            #endif // /ENABLE_CUSTOM_MULTISAMPLING

            #if ENABLE_DEPTH_TEXTURE
            renderTargetDesc.depthStencilAttachment = renderTargetDepthTex;
            #else
            //renderTargetDesc.depthStencilAttachment = LLGL::Format::D32Float;
            #endif
        }
        renderTarget = renderer->CreateRenderTarget(renderTargetDesc);

        // Initialize projection matrix for render-target scene rendering
        renderTargetProj = PerspectiveProjection(1.0f, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));
    }

    #if ENABLE_RESOURCE_HEAP

    void CreateResourceHeap()
    {
        // Create resource heap for render target
        LLGL::ResourceViewDescriptor resourceViews[] =
        {
            constantBuffer, samplerState, colorMap, /*colorMap,*/
            constantBuffer, samplerState, renderTargetTex, /*renderTargetTex,*/
        };

        #if ENABLE_CBUFFER_RANGE

        auto& cbufferView0 = resourceViews[0].bufferView;
        {
            cbufferView0.offset = 0;
            cbufferView0.size   = cbufferAlignment;
        }

        auto& cbufferView1 = resourceViews[3].bufferView;
        {
            cbufferView1.offset = cbufferAlignment;
            cbufferView1.size   = cbufferAlignment;
        }

        #endif // /ENABLE_CBUFFER_RANGE

        resourceHeap = renderer->CreateResourceHeap(pipelineLayout, resourceViews);
    }

    #endif // /ENABLE_RESOURCE_HEAP

    void UpdateModelTransform(Settings& settings, const Gs::Matrix4f& proj, float rotation, const Gs::Vector3f& axis = { 0, 1, 0 })
    {
        Gs::Translate(settings.wMatrix, { 0, 0, 5 });
        Gs::RotateFree(settings.wMatrix, axis.Normalized(), rotation);
        settings.wvpMatrix = proj * settings.wMatrix;
    }

    static const auto shaderStages = LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage;

    void UpdateSettingsForTexture(Settings& settings)
    {
        // Update model transformation with render-target projection
        UpdateModelTransform(settings, renderTargetProj, rotation.y, Gs::Vector3f(1));

        #if ENABLE_CUSTOM_MULTISAMPLING

        // Disable multi-sample texture in fragment shader
        settings.useTexture2DMS = 0;

        #endif // /ENABLE_CUSTOM_MULTISAMPLING
    }

    void UpdateSettingsForScreen(Settings& settings)
    {
        #if ENABLE_CUSTOM_MULTISAMPLING

        // Enable multi-sample texture in fragment shader
        settings.useTexture2DMS = 1;

        #endif // ENABLE_CUSTOM_MULTISAMPLING

        UpdateModelTransform(settings, projection, rotation.x);
    }

    void UpdateScene()
    {
        // Update scene animation (simple rotation)
        if (input.KeyPressed(LLGL::Key::LButton))
            rotation.x += static_cast<float>(input.GetMouseMotion().x)*0.005f;
        if (input.KeyPressed(LLGL::Key::RButton))
            rotation.y += static_cast<float>(input.GetMouseMotion().x)*0.005f;

        // Check if user wants to sage the render target texture to file
        if (input.KeyDown(LLGL::Key::F4))
            SaveTexture(*renderTargetTex, "Screenshot." + GetModuleName() + ".png");
    }

    void DrawSceneIntoTexture()
    {
        #if !ENABLE_CBUFFER_RANGE

        // Update constant buffer with current settings
        Settings settings;
        UpdateSettingsForTexture(settings);
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        #endif // /ENABLE_CBUFFER_RANGE

        // Begin render pass for render target
        commands->BeginRenderPass(*renderTarget);
        {
            // Clear color and depth buffers of active framebuffer (i.e. the render target)
            commands->Clear(LLGL::ClearFlags::ColorDepth, { 0.2f, 0.7f, 0.1f, 1.0f });

            // Bind graphics pipeline for render target
            commands->SetPipelineState(*pipelines[0]);

            // Set common buffers and sampler states
            commands->SetIndexBuffer(*indexBuffer);
            commands->SetVertexBuffer(*vertexBuffer);

            #if ENABLE_RESOURCE_HEAP
            if (resourceHeap)
            {
                // Set graphics pipeline resources
                commands->SetResourceHeap(*resourceHeap, 0);
            }
            else
            #endif // /ENABLE_RESOURCE_HEAP
            {
                // Set resource directly
                commands->SetResource(0, *constantBuffer);
                commands->SetResource(1, *samplerState);
                commands->SetResource(2, *colorMap);
                #if ENABLE_CUSTOM_MULTISAMPLING
                commands->SetResource(3, *dummyTexMS);
                #endif
            }

            // Draw scene
            commands->DrawIndexed(36, 0);
        }
        commands->EndRenderPass();
    }

    void DrawSceneOntoScreen()
    {
        #if !ENABLE_CBUFFER_RANGE

        // Update model transformation with standard projection
        Settings settings;
        UpdateSettingsForScreen(settings);
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        #endif // /ENABLE_CBUFFER_RANGE

        // Generate MIP-maps again after texture has been written by the render-target
        commands->GenerateMips(*renderTargetTex);

        // Begin render pass for swap-chain
        commands->BeginRenderPass(*swapChain);
        {
            // Clear color and depth buffers of active framebuffer (i.e. the screen)
            commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);

            // Binds graphics pipeline for swap-chain
            commands->SetPipelineState(*pipelines[1]);

            // Set viewport to fullscreen.
            // Note: this must be done AFTER the respective graphics pipeline has been set,
            //       since the previous pipeline has no dynamic viewport!
            commands->SetViewport(swapChain->GetResolution());

            #if ENABLE_RESOURCE_HEAP
            if (resourceHeap)
            {
                // Set graphics pipeline resources
                commands->SetResourceHeap(*resourceHeap, 1);
            }
            else
            #endif // /ENABLE_RESOURCE_HEAP
            {
                // Set previous resources again since we invalidated them via SetPipelineState()
                commands->SetResource(0, *constantBuffer);
                commands->SetResource(1, *samplerState);

                // Set render-target texture
                commands->SetResource(2, *renderTargetTex);
                #if ENABLE_CUSTOM_MULTISAMPLING
                commands->SetResource(3, *renderTargetTexMS);
                #endif
            }

            // Draw scene
            commands->DrawIndexed(36, 0);
        }
        commands->EndRenderPass();
    }

    #if ENABLE_CBUFFER_RANGE

    Settings& GetCbufferData(std::size_t idx)
    {
        // Returns address to the CPU local constant buffer data with alignment
        const auto offset = static_cast<std::size_t>(cbufferAlignment) * idx;
        return *reinterpret_cast<Settings*>(&(cbufferData[offset]));
    }

    #endif // /ENABLE_CBUFFER_RANGE

    void OnDrawFrame() override
    {
        // Update scene by user input
        UpdateScene();

        commands->Begin();
        {
            #if ENABLE_CBUFFER_RANGE

            // Update constant buffer with all settings at once
            UpdateSettingsForTexture(GetCbufferData(0));
            UpdateSettingsForScreen(GetCbufferData(1));
            commands->UpdateBuffer(*constantBuffer, 0, cbufferData.data(), static_cast<std::uint16_t>(cbufferData.size()));

            #endif // /ENABLE_CBUFFER_RANGE

            // Draw scene into texture, then draw scene onto screen
            commands->PushDebugGroup("RenderTexture");
            {
                DrawSceneIntoTexture();
            }
            commands->PopDebugGroup();

            commands->PushDebugGroup("RenderScreen");
            {
                DrawSceneOntoScreen();
            }
            commands->PopDebugGroup();
        }
        commands->End();
        commandQueue->Submit(*commands);
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_RenderTarget);



