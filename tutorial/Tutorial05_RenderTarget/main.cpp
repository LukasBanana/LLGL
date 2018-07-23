/*
 * main.cpp (Tutorial05_RenderTarget)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <tutorial.h>


// Enable multi-sampling
//#define ENABLE_MULTISAMPLING

// Enable custom multi-sampling by rendering directly into a multi-sample texture
//#define ENABLE_CUSTOM_MULTISAMPLING

// Enable depth texture instead of depth buffer for render target
//#define ENABLE_DEPTH_TEXTURE


#ifndef ENABLE_MULTISAMPLING
#undef ENABLE_CUSTOM_MULTISAMPLING
#endif

class Tutorial05 : public Tutorial
{

    LLGL::ShaderProgram*    shaderProgram           = nullptr;

    LLGL::GraphicsPipeline* pipelines[2]            = {};
    LLGL::PipelineLayout*   pipelineLayout          = nullptr;

    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           indexBuffer             = nullptr;
    LLGL::Buffer*           constantBuffer          = nullptr;

    LLGL::Texture*          colorMap                = nullptr;
    LLGL::Sampler*          samplerState            = nullptr;
    LLGL::ResourceHeap*     resourceHeaps[2]        = {};

    LLGL::RenderTarget*     renderTarget            = nullptr;
    LLGL::Texture*          renderTargetTex         = nullptr;

    #ifdef ENABLE_DEPTH_TEXTURE
    LLGL::Texture*          renderTargetDepthTex    = nullptr;
    #endif

    Gs::Matrix4f            renderTargetProj;

    Gs::Vector2f            rotation;

    const LLGL::Extent2D    renderTargetSize        = LLGL::Extent2D(
        #ifdef ENABLE_CUSTOM_MULTISAMPLING
        64, 64
        #else
        512, 512
        #endif
    );

    struct Settings
    {
        Gs::Matrix4f        wvpMatrix;
        int                 useTexture2DMS = 0;
        int                 _pad0[3];
    }
    settings;

public:

    Tutorial05() :
        Tutorial { L"LLGL Tutorial 05: RenderTarget" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreateColorMap();
        CreateRenderTarget();
        CreatePipelines();

        #ifndef __APPLE__
        CreateResourceHeaps();
        #endif

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
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float });

        // Initialize vertices (scale texture-coordiantes a little bit, to show the texture border)
        auto vertices = GenerateTexturedCubeVertices();

        for (auto& v : vertices)
            v.texCoord = (v.texCoord - Gs::Vector2f(0.5f))*1.05f + Gs::Vector2f(0.5f);

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateTexturedCubeTriangleIndices(), LLGL::DataType::UInt32);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "shader.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::Fragment, "shader.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "vertex.glsl" },
                    #ifdef __APPLE__
                    { LLGL::ShaderType::Fragment, "fragment.410core.glsl" }
                    #else
                    { LLGL::ShaderType::Fragment, "fragment.glsl" }
                    #endif
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex, "vertex.450core.spv" },
                    { LLGL::ShaderType::Fragment, "fragment.450core.spv" }
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
                LLGL::BindingDescriptor { LLGL::ResourceType::ConstantBuffer, LLGL::StageFlags::FragmentStage | LLGL::StageFlags::VertexStage, 0 },
                LLGL::BindingDescriptor { LLGL::ResourceType::Sampler,        LLGL::StageFlags::FragmentStage,                                 1 },
                LLGL::BindingDescriptor { LLGL::ResourceType::Texture,        LLGL::StageFlags::FragmentStage,                                 (combinedSampler ? 1u : 2u) },
                //LLGL::BindingDescriptor { LLGL::ResourceType::Texture,        LLGL::StageFlags::FragmentStage,                               3 },
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(layoutDesc);

        // Create graphics pipeline for render context
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.renderPass                 = context->GetRenderPass();
            pipelineDesc.pipelineLayout             = pipelineLayout;

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.depth.writeEnabled         = true;

            // Enable culling of back-facing polygons
            pipelineDesc.rasterizer.cullMode        = LLGL::CullMode::Back;

            #ifdef ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);
            #endif
        }
        pipelines[1] = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Create graphics pipeline for render target
        {
            pipelineDesc.renderPass = renderTarget->GetRenderPass();
            pipelineDesc.viewports  = { LLGL::Viewport{ { 0, 0 }, renderTarget->GetResolution() } };

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
        pipelines[0] = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

    void CreateColorMap()
    {
        // Load color map texture from file
        colorMap = LoadTexture("colorMap.jpg");

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
        LLGL::MultiSamplingDescriptor multiSamplingDesc { 8 };
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
        renderTargetDepthTex = renderer->CreateTexture(
            LLGL::Texture2DDesc(LLGL::Format::D32Float, renderTargetSize.width, renderTargetSize.height)
        );

        #endif

        #if 0//TEST
        // Create depth texture
        auto renderTargetDepthTex2 = renderer->CreateTexture(
            LLGL::Texture2DDesc(LLGL::Format::D32Float, renderTargetSize.width, renderTargetSize.height)
        );
        #endif

        // Generate all MIP-map levels
        renderer->GenerateMips(*renderTargetTex);

        // Create render-target with multi-sampling
        LLGL::RenderTargetDescriptor renderTargetDesc;
        {
            renderTargetDesc.resolution = renderTargetSize;

            #ifdef ENABLE_MULTISAMPLING
            renderTargetDesc.multiSampling          = multiSamplingDesc;
            #   ifdef ENABLE_CUSTOM_MULTISAMPLING
            renderTargetDesc.customMultiSampling    = true;
            #   endif
            #endif

            renderTargetDesc.attachments =
            {
                #ifdef ENABLE_DEPTH_TEXTURE
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Depth, renderTargetDepthTex },
                #else
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Depth },
                #endif
                LLGL::AttachmentDescriptor { LLGL::AttachmentType::Color, renderTargetTex }
            };
        }
        renderTarget = renderer->CreateRenderTarget(renderTargetDesc);

        // Initialize projection matrix for render-target scene rendering
        renderTargetProj = PerspectiveProjection(1.0f, 0.1f, 100.0f, Gs::Deg2Rad(45.0f));
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for render target
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews = { constantBuffer, samplerState, colorMap /*, colorMap*/ };
        }
        resourceHeaps[0] = renderer->CreateResourceHeap(resourceHeapDesc);

        // Create resource heap for final render
        {
            resourceHeapDesc.resourceViews = { constantBuffer, samplerState, renderTargetTex /*, renderTargetTex*/ };
        }
        resourceHeaps[1] = renderer->CreateResourceHeap(resourceHeapDesc);
    }

    void UpdateModelTransform(const Gs::Matrix4f& proj, float rotation, const Gs::Vector3f& axis = { 0, 1, 0 })
    {
        settings.wvpMatrix = proj;
        Gs::Translate(settings.wvpMatrix, { 0, 0, 5 });
        Gs::RotateFree(settings.wvpMatrix, axis.Normalized(), rotation);
    }

    static const auto shaderStages = LLGL::StageFlags::VertexStage | LLGL::StageFlags::FragmentStage;

    void UpdateScene()
    {
        // Update scene animation (simple rotation)
        if (input->KeyPressed(LLGL::Key::LButton))
            rotation.x += static_cast<float>(input->GetMouseMotion().x)*0.005f;
        if (input->KeyPressed(LLGL::Key::RButton))
            rotation.y += static_cast<float>(input->GetMouseMotion().x)*0.005f;

        // Check if user wants to sage the render target texture to file
        if (input->KeyDown(LLGL::Key::Return))
            SaveTexture(*renderTargetTex, "RenderTargetTexture.png");
    }

    void DrawSceneIntoTexture()
    {
        // Update model transformation with render-target projection
        UpdateModelTransform(renderTargetProj, rotation.y, Gs::Vector3f(1));

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

        #endif

        // Update constant buffer with current settings
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        // Begin render pass for render target
        commands->BeginRenderPass(*renderTarget);
        {
            // Clear color and depth buffers of active framebuffer (i.e. the render target)
            commands->SetClearColor({ 0.2f, 0.7f, 0.1f });
            commands->Clear(LLGL::ClearFlags::ColorDepth);

            // Bind graphics pipeline for render target
            commands->SetGraphicsPipeline(*pipelines[0]);

            // Set common buffers and sampler states
            commands->SetIndexBuffer(*indexBuffer);
            commands->SetVertexBuffer(*vertexBuffer);

            if (resourceHeaps[0])
            {
                // Set graphics pipeline resources
                commands->SetGraphicsResourceHeap(*resourceHeaps[0]);
            }
            else
            {
                commandsExt->SetConstantBuffer(*constantBuffer, 0, shaderStages);
                commandsExt->SetTexture(*colorMap, 0, shaderStages);
                commandsExt->SetSampler(*samplerState, 0, shaderStages);
            }

            // Draw scene
            commands->DrawIndexed(36, 0);
        }
        commands->EndRenderPass();
    }

    void DrawSceneOntoScreen()
    {
        #ifdef ENABLE_CUSTOM_MULTISAMPLING

        // Enable multi-sample texture in fragment shader
        settings.useTexture2DMS = 1;

        #endif // ENABLE_CUSTOM_MULTISAMPLING

        // Update model transformation with standard projection
        UpdateModelTransform(projection, rotation.x);
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));

        // Begin render pass for render context
        commands->BeginRenderPass(*context);
        {
            // Clear color and depth buffers of active framebuffer (i.e. the screen)
            commands->SetClearColor(defaultClearColor);
            commands->Clear(LLGL::ClearFlags::ColorDepth);

            // Binds graphics pipeline for render context
            commands->SetGraphicsPipeline(*pipelines[1]);

            // Set viewport to fullscreen.
            // Note: this must be done AFTER the respective graphics pipeline has been set,
            //       since the previous pipeline has no dynamic viewport!
            commands->SetViewport(LLGL::Viewport{ { 0, 0 }, context->GetResolution() });

            // Generate MIP-maps again after texture has been written by the render-target
            renderer->GenerateMips(*renderTargetTex);

            if (resourceHeaps[1])
            {
                // Set graphics pipeline resources
                commands->SetGraphicsResourceHeap(*resourceHeaps[1]);
            }
            else
            {
                #ifdef ENABLE_CUSTOM_MULTISAMPLING

                // Set multi-sample render-target texture
                commandsExt->SetTexture(*renderTargetTex, 1, shaderStages);

                #else

                // Set render-target texture
                commandsExt->SetTexture(*renderTargetTex, 0, shaderStages);

                #endif // /ENABLE_CUSTOM_MULTISAMPLING
            }

            // Draw scene
            commands->DrawIndexed(36, 0);
        }
        commands->EndRenderPass();
    }

    void OnDrawFrame() override
    {
        // Update scene by user input
        UpdateScene();

        commands->Begin();
        {
            // Draw scene into texture, then draw scene onto screen
            DrawSceneIntoTexture();
            DrawSceneOntoScreen();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial05);



