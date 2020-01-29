/*
 * Example.cpp (Example_RenderTarget)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


// Enable multi-sampling
#define ENABLE_MULTISAMPLING

// Enable custom multi-sampling by rendering directly into a multi-sample texture
//#define ENABLE_CUSTOM_MULTISAMPLING

// Enable depth texture instead of depth buffer for render target
//#define ENABLE_DEPTH_TEXTURE

// Enables constant buffer view (CBV) ranges
//#define ENABLE_CBUFFER_RANGE


#ifndef ENABLE_MULTISAMPLING
#undef ENABLE_CUSTOM_MULTISAMPLING
#endif

class Example_RenderTarget : public ExampleBase
{

    LLGL::ShaderProgram*    shaderProgram           = nullptr;

    LLGL::PipelineState*    pipelines[2]            = {};
    LLGL::PipelineLayout*   pipelineLayout          = nullptr;

    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           indexBuffer             = nullptr;
    LLGL::Buffer*           constantBuffer          = nullptr;

    LLGL::Texture*          colorMap                = nullptr;
    LLGL::Sampler*          samplerState            = nullptr;
    LLGL::ResourceHeap*     resourceHeap            = nullptr;

    LLGL::RenderTarget*     renderTarget            = nullptr;
    LLGL::Texture*          renderTargetTex         = nullptr;

    #ifdef ENABLE_DEPTH_TEXTURE
    LLGL::Texture*          renderTargetDepthTex    = nullptr;
    #endif

    Gs::Matrix4f            renderTargetProj;

    Gs::Vector2f            rotation;

    #ifdef ENABLE_CBUFFER_RANGE
    std::uint64_t           cbufferAlignment        = 0;
    std::vector<char>       cbufferData;
    #endif

    const LLGL::Extent2D    renderTargetSize        = LLGL::Extent2D(
        #ifdef ENABLE_CUSTOM_MULTISAMPLING
        64, 64
        #else
        512, 512
        #endif
    );

    struct alignas(16) Settings
    {
        Gs::Matrix4f        wvpMatrix;
        int                 useTexture2DMS = 0;
    };

public:

    Example_RenderTarget() :
        ExampleBase { L"LLGL Example: RenderTarget" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreateColorMap();
        CreateRenderTarget();
        CreatePipelines();
        CreateResourceHeap();

        // Show some information
        std::cout << "press LEFT MOUSE BUTTON and move the mouse on the X-axis to rotate the OUTER cube" << std::endl;
        std::cout << "press RIGHT MOUSE BUTTON and move the mouse on the X-axis to rotate the INNER cube" << std::endl;
        std::cout << "press RETURN KEY to save the render target texture to a PNG file" << std::endl;
    }

private:

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });

        // Initialize vertices (scale texture-coordiantes a little bit, to show the texture border)
        auto vertices = GenerateTexturedCubeVertices();

        for (auto& v : vertices)
            v.texCoord = (v.texCoord - Gs::Vector2f(0.5f))*1.05f + Gs::Vector2f(0.5f);

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateTexturedCubeTriangleIndices(), LLGL::Format::R32UInt);

        #ifdef ENABLE_CBUFFER_RANGE

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
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "Example.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.vert" },
                    #ifdef __APPLE__
                    { LLGL::ShaderType::Fragment, "Example.410core.frag" }
                    #else
                    { LLGL::ShaderType::Fragment, "Example.frag" }
                    #endif
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.450core.vert.spv" },
                    { LLGL::ShaderType::Fragment, "Example.450core.frag.spv" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::Metal))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,   "Example.metal", "VS", "1.1" },
                    { LLGL::ShaderType::Fragment, "Example.metal", "PS", "1.1" }
                },
                { vertexFormat }
            );
        }
    }

    void CreatePipelines()
    {
        bool combinedSampler = IsOpenGL();

        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor layoutDesc;
        {
            layoutDesc.bindings =
            {
                LLGL::BindingDescriptor{ "Settings",        LLGL::ResourceType::Buffer,   LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::FragmentStage | LLGL::StageFlags::VertexStage, 3                           },
                LLGL::BindingDescriptor{ "colorMapSampler", LLGL::ResourceType::Sampler,  0,                               LLGL::StageFlags::FragmentStage,                                 1                           },
                LLGL::BindingDescriptor{ "colorMap",        LLGL::ResourceType::Texture,  LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 (combinedSampler ? 1u : 2u) },
              //LLGL::BindingDescriptor{ "colorMapMS",      LLGL::ResourceType::Texture,  LLGL::BindFlags::Sampled,        LLGL::StageFlags::FragmentStage,                                 3                           },
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create graphics pipeline for render context
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.renderPass                     = context->GetRenderPass();
            pipelineDesc.pipelineLayout                 = pipelineLayout;

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            // Enable culling of back-facing polygons
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipelines[1] = renderer->CreatePipelineState(pipelineDesc);

        // Create graphics pipeline for render target
        {
            pipelineDesc.renderPass = renderTarget->GetRenderPass();
            pipelineDesc.viewports  = { LLGL::Viewport{ { 0, 0 }, renderTarget->GetResolution() } };

            #ifdef ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampleEnabled = true;
            #else
            pipelineDesc.rasterizer.multiSampleEnabled = false;
            #endif

            if (IsOpenGL())
            {
                /*
                Set front face to counter-clock wise (CCW) to be uniform between OpenGL and Direct3D:
                A huge difference between OpenGL and Direct3D is,
                that OpenGL stores image data from the lower-left to the upper-right in a texture,
                but Direct3D stores image data from the upper-left to the lower-right in a texture.
                The default screen-space origin of LLGL is the upper-left, so when rendering into a texture,
                we need to render vertically flipped when OpenGL is used.
                To do this we flip the Y-axis of the world-view-projection matrix and invert the front-facing,
                so that the face-culling works as excepted.
                */
                pipelineDesc.rasterizer.frontCCW = true;
            }
        }
        pipelines[0] = renderer->CreatePipelineState(pipelineDesc);
    }

    void CreateColorMap()
    {
        // Load color map texture from file
        colorMap = LoadTexture("../../Media/Textures/Crate.jpg");

        // Create common sampler state for all textures
        LLGL::SamplerDescriptor samplerDesc;
        {
            samplerDesc.addressModeU    = LLGL::SamplerAddressMode::Border;
            samplerDesc.addressModeV    = LLGL::SamplerAddressMode::Border;
            samplerDesc.maxAnisotropy   = 8;
            samplerDesc.borderColor     = { 0, 0, 0, 1 };
        }
        samplerState = renderer->CreateSampler(samplerDesc);
    }

    void CreateRenderTarget()
    {
        // Initialize multisampling
        #ifdef ENABLE_MULTISAMPLING
        const std::uint32_t samples = 8;
        #else
        const std::uint32_t samples = 1;
        #endif

        // Create empty render-target texture
        #ifdef ENABLE_CUSTOM_MULTISAMPLING

        renderTargetTex = renderer->CreateTexture(
            LLGL::Texture2DMSDesc(LLGL::Format::RGBA8UNorm, renderTargetSize.width, renderTargetSize.height, multiSamplingDesc.samples)
        );

        #else

        renderTargetTex = renderer->CreateTexture(
            LLGL::Texture2DDesc(LLGL::Format::RGBA8UNorm, renderTargetSize.width, renderTargetSize.height)
        );

        #endif

        #ifdef ENABLE_DEPTH_TEXTURE

        // Create depth texture
        LLGL::TextureDescriptor depthTexDesc;
        {
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

        #endif

        // Create render-target with multi-sampling
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.resolution = renderTargetSize;

            #ifdef ENABLE_MULTISAMPLING
            renderTargetDesc.samples                = samples;
            #   ifdef ENABLE_CUSTOM_MULTISAMPLING
            renderTargetDesc.customMultiSampling    = true;
            #   endif
            #endif

            renderTargetDesc.attachments =
            {
                #ifdef ENABLE_DEPTH_TEXTURE
                LLGL::AttachmentDescriptor{ LLGL::AttachmentType::Depth, renderTargetDepthTex },
                #else
                LLGL::AttachmentDescriptor{ LLGL::AttachmentType::Depth },
                #endif
                LLGL::AttachmentDescriptor{ LLGL::AttachmentType::Color, renderTargetTex }
            };
        }
        renderTarget = renderer->CreateRenderTarget(renderTargetDesc);

        // Initialize projection matrix for render-target scene rendering
        renderTargetProj = PerspectiveProjection(1.0f, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));
    }

    void CreateResourceHeap()
    {
        // Create resource heap for render target
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews =
            {
                constantBuffer, samplerState, colorMap, /*colorMap,*/
                constantBuffer, samplerState, renderTargetTex, /*renderTargetTex,*/
            };

            #ifdef ENABLE_CBUFFER_RANGE

            auto& cbufferView0 = resourceHeapDesc.resourceViews[0].bufferView;
            {
                cbufferView0.offset = 0;
                cbufferView0.size   = cbufferAlignment;
            }

            auto& cbufferView1 = resourceHeapDesc.resourceViews[3].bufferView;
            {
                cbufferView1.offset = cbufferAlignment;
                cbufferView1.size   = cbufferAlignment;
            }

            #endif // /ENABLE_CBUFFER_RANGE
        }
        resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void UpdateModelTransform(Settings& settings, const Gs::Matrix4f& proj, float rotation, const Gs::Vector3f& axis = { 0, 1, 0 })
    {
        settings.wvpMatrix = proj;
        Gs::Translate(settings.wvpMatrix, { 0, 0, 5 });
        Gs::RotateFree(settings.wvpMatrix, axis.Normalized(), rotation);
    }

    static const auto shaderStages = LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage;

    void UpdateSettingsForTexture(Settings& settings)
    {
        // Update model transformation with render-target projection
        UpdateModelTransform(settings, renderTargetProj, rotation.y, Gs::Vector3f(1));

        if (IsOpenGL())
        {
            /*
            Now flip the Y-axis (0 for X-axis, 1 for Y-axis, 2 for Z-axis) of the
            world-view-projection matrix to render vertically flipped into the render-target
            */
            Gs::FlipAxis(settings.wvpMatrix, 1);
        }

        #ifdef ENABLE_CUSTOM_MULTISAMPLING

        // Disable multi-sample texture in fragment shader
        settings.useTexture2DMS = 0;

        #endif // /ENABLE_CUSTOM_MULTISAMPLING
    }

    void UpdateSettingsForScreen(Settings& settings)
    {
        #ifdef ENABLE_CUSTOM_MULTISAMPLING

        // Enable multi-sample texture in fragment shader
        settings.useTexture2DMS = 1;

        #endif // ENABLE_CUSTOM_MULTISAMPLING

        UpdateModelTransform(settings, projection, rotation.x);
    }

    void UpdateScene()
    {
        // Update scene animation (simple rotation)
        if (input->KeyPressed(LLGL::Key::LButton))
            rotation.x += static_cast<float>(input->GetMouseMotion().x)*0.005f;
        if (input->KeyPressed(LLGL::Key::RButton))
            rotation.y += static_cast<float>(input->GetMouseMotion().x)*0.005f;

        // Check if user wants to sage the render target texture to file
        if (input->KeyDown(LLGL::Key::F4))
            SaveTexture(*renderTargetTex, "Screenshot." + GetModuleName() + ".png");
    }

    void DrawSceneIntoTexture()
    {
        #ifndef ENABLE_CBUFFER_RANGE

        // Update constant buffer with current settings
        Settings settings;
        UpdateSettingsForTexture(settings);
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        #endif // /ENABLE_CBUFFER_RANGE

        // Begin render pass for render target
        commands->BeginRenderPass(*renderTarget);
        {
            // Clear color and depth buffers of active framebuffer (i.e. the render target)
            commands->SetClearColor({ 0.2f, 0.7f, 0.1f });
            commands->Clear(LLGL::ClearFlags::ColorDepth);

            // Bind graphics pipeline for render target
            commands->SetPipelineState(*pipelines[0]);

            // Set common buffers and sampler states
            commands->SetIndexBuffer(*indexBuffer);
            commands->SetVertexBuffer(*vertexBuffer);

            if (resourceHeap)
            {
                // Set graphics pipeline resources
                commands->SetResourceHeap(*resourceHeap, 0);
            }
            else
            {
                // Set resource directly
                commands->SetResource(*constantBuffer, 3, LLGL::BindFlags::ConstantBuffer, shaderStages);
                commands->SetResource(*colorMap, 2, LLGL::BindFlags::Sampled, shaderStages);
                commands->SetResource(*samplerState, 1, 0, shaderStages);
            }

            // Draw scene
            commands->DrawIndexed(36, 0);
        }
        commands->EndRenderPass();
    }

    void DrawSceneOntoScreen()
    {
        #ifndef ENABLE_CBUFFER_RANGE

        // Update model transformation with standard projection
        Settings settings;
        UpdateSettingsForScreen(settings);
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        #endif // /ENABLE_CBUFFER_RANGE

        // Generate MIP-maps again after texture has been written by the render-target
        commands->GenerateMips(*renderTargetTex);

        // Begin render pass for render context
        commands->BeginRenderPass(*context);
        {
            // Clear color and depth buffers of active framebuffer (i.e. the screen)
            commands->SetClearColor(backgroundColor);
            commands->Clear(LLGL::ClearFlags::ColorDepth);

            // Binds graphics pipeline for render context
            commands->SetPipelineState(*pipelines[1]);

            // Set viewport to fullscreen.
            // Note: this must be done AFTER the respective graphics pipeline has been set,
            //       since the previous pipeline has no dynamic viewport!
            commands->SetViewport(context->GetResolution());

            if (resourceHeap)
            {
                // Set graphics pipeline resources
                commands->SetResourceHeap(*resourceHeap, 1);
            }
            else
            {
                #ifdef ENABLE_CUSTOM_MULTISAMPLING

                // Set multi-sample render-target texture
                commands->SetResource(*renderTargetTex, 1, LLGL::BindFlags::Sampled, shaderStages);

                #else

                // Set render-target texture
                commands->SetResource(*renderTargetTex, 2, LLGL::BindFlags::Sampled, shaderStages);

                #endif // /ENABLE_CUSTOM_MULTISAMPLING
            }

            // Draw scene
            commands->DrawIndexed(36, 0);
        }
        commands->EndRenderPass();
    }

    #ifdef ENABLE_CBUFFER_RANGE

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
            #ifdef ENABLE_CBUFFER_RANGE

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

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_RenderTarget);



