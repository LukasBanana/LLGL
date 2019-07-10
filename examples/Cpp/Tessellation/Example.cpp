/*
 * Example.cpp (Example_Tessellation)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


// Automatically rotate the model
//#define AUTO_ROTATE

// Enable multi-sampling anti-aliasing
#define ENABLE_MULTISAMPLING

// Use render pass to optimize attachment clearing
#define ENABLE_RENDER_PASS

// Test constant buffer array
//#define _TEST_BUFFER_ARRAY_


class Example_Tessellation : public ExampleBase
{

    LLGL::ShaderProgram*    shaderProgram       = nullptr;
    LLGL::GraphicsPipeline* pipeline[2]         = { nullptr };

    LLGL::Buffer*           vertexBuffer        = nullptr;
    LLGL::Buffer*           indexBuffer         = nullptr;
    LLGL::Buffer*           constantBuffer      = nullptr;

    LLGL::PipelineLayout*   pipelineLayout      = nullptr;
    LLGL::ResourceHeap*     resourceHeap        = nullptr;

    #ifdef ENABLE_RENDER_PASS
    LLGL::RenderPass*       renderPass          = nullptr;
    #endif

    #ifdef _TEST_BUFFER_ARRAY_
    LLGL::BufferArray*      constantBufferArray = nullptr;
    #endif

    std::uint32_t           constantBufferIndex = 0;

    bool                    showWireframe       = false;

    struct Settings
    {
        Gs::Matrix4f    wvpMatrix;
        float           tessLevelInner  = 5.0f;
        float           tessLevelOuter  = 5.0f;
        float           twist           = 0.0f;
        float           _pad0;                  // <-- padding for 16 byte pack alignment of constant buffers
    }
    settings;

public:

    Example_Tessellation() :
        ExampleBase( L"LLGL Example: Tessellation" )//, { 800, 600 }, 0 )
    {
        // Check if constant buffers and tessellation shaders are supported
        const auto& renderCaps = renderer->GetRenderingCaps();

        if (!renderCaps.features.hasConstantBuffers)
            throw std::runtime_error("constant buffers are not supported by this renderer");
        if (!renderCaps.features.hasTessellationShaders)
            throw std::runtime_error("tessellation shaders are not supported by this renderer");

        // Create graphics object
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        #ifdef ENABLE_RENDER_PASS
        CreateRenderPass();
        #endif
        CreatePipelines();

        // Print some information on the standard output
        std::cout << "press LEFT MOUSE BUTTON and move mouse on X axis to increase/decrease inner tessellation" << std::endl;
        std::cout << "press RIGHT MOUSE BUTTON and move mouse on X axis to increase/decrease outer tessellation" << std::endl;
        std::cout << "press MIDDLE MOUSE BUTTON and move mouse on X axis to increase/decrease twist" << std::endl;
        std::cout << "press TAB KEY to switch between wireframe modes" << std::endl;
        ShowTessLevel();
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

        UpdateUserInput();

        // Create buffers for a simple 3D cube model
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeQuadlIndices(), LLGL::Format::R32UInt);
        constantBuffer = CreateConstantBuffer(settings);

        #ifdef _TEST_BUFFER_ARRAY_

        // Create constant buffer array
        constantBufferArray = renderer->CreateBufferArray(1, &constantBuffer);

        #endif

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        if (Supported(LLGL::ShadingLanguage::GLSL))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,         "Example.vert" },
                    { LLGL::ShaderType::TessControl,    "Example.tesc" },
                    { LLGL::ShaderType::TessEvaluation, "Example.tese" },
                    { LLGL::ShaderType::Fragment,       "Example.frag" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::SPIRV))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,         "Example.450core.vert.spv" },
                    { LLGL::ShaderType::TessControl,    "Example.450core.tesc.spv" },
                    { LLGL::ShaderType::TessEvaluation, "Example.450core.tese.spv" },
                    { LLGL::ShaderType::Fragment,       "Example.450core.frag.spv" }
                },
                { vertexFormat }
            );
        }
        else if (Supported(LLGL::ShadingLanguage::HLSL))
        {
            shaderProgram = LoadShaderProgram(
                {
                    { LLGL::ShaderType::Vertex,         "Example.hlsl", "VS", "vs_5_0" },
                    { LLGL::ShaderType::TessControl,    "Example.hlsl", "HS", "hs_5_0" },
                    { LLGL::ShaderType::TessEvaluation, "Example.hlsl", "DS", "ds_5_0" },
                    { LLGL::ShaderType::Fragment,       "Example.hlsl", "PS", "ps_5_0" }
                },
                { vertexFormat }
            );
        }
    }

    #ifdef ENABLE_RENDER_PASS
    void CreateRenderPass()
    {
        LLGL::RenderPassDescriptor renderPassDesc;
        {
            renderPassDesc.colorAttachments =
            {
                LLGL::AttachmentFormatDescriptor { context->QueryColorFormat(), LLGL::AttachmentLoadOp::Clear },
            };
            renderPassDesc.depthAttachment =
            (
                LLGL::AttachmentFormatDescriptor { context->QueryDepthStencilFormat(), LLGL::AttachmentLoadOp::Clear }
            );
        }
        renderPass = renderer->CreateRenderPass(renderPassDesc);
    }
    #endif

    void CreatePipelines()
    {
        // Create pipeline layout
        LLGL::PipelineLayoutDescriptor plDesc;
        {
            plDesc.bindings =
            {
                LLGL::BindingDescriptor
                {
                    LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::AllTessStages, constantBufferIndex, 1u, "Settings"
                }
            };
        }
        pipelineLayout = renderer->CreatePipelineLayout(plDesc);

        // Create resource view heap
        LLGL::ResourceHeapDescriptor resourceHeapDesc;
        {
            resourceHeapDesc.pipelineLayout = pipelineLayout;
            resourceHeapDesc.resourceViews  = { constantBuffer };
        }
        resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);

        // Setup graphics pipeline descriptors
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            // Set references to shader program, render pass, and pipeline layout
            pipelineDesc.shaderProgram              = shaderProgram;
            #ifdef ENABLE_RENDER_PASS
            pipelineDesc.renderPass                 = renderPass;
            #endif
            pipelineDesc.pipelineLayout             = pipelineLayout;

            // Set input-assembler state (draw pachtes with 4 control points)
            pipelineDesc.primitiveTopology          = LLGL::PrimitiveTopology::Patches4;

            // Enable multi-sample anti-aliasing
            #ifdef ENABLE_MULTISAMPLING
            pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);
            #endif

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.depth.writeEnabled         = true;

            // Enable back-face culling
            pipelineDesc.rasterizer.cullMode        = LLGL::CullMode::Back;
        }

        // Create graphics pipelines
        pipeline[0] = renderer->CreateGraphicsPipeline(pipelineDesc);

        pipelineDesc.rasterizer.polygonMode = LLGL::PolygonMode::Wireframe;
        pipeline[1] = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

    void ShowTessLevel()
    {
        //std::cout << "tessellation level (inner = " << settings.tessLevelInner << ", outer = " << settings.tessLevelOuter << ")      \r";
        //std::flush(std::cout);
    }

private:

    void UpdateUserInput()
    {
        // Tessellation level-of-detail limits
        static const float tessLevelMin = 1.0f, tessLevelMax = 64.0f;

        // Update tessellation levels by user input
        auto motion = input->GetMouseMotion().x;
        auto motionScaled = static_cast<float>(motion)*0.1f;

        if (input->KeyPressed(LLGL::Key::LButton))
        {
            settings.tessLevelInner += motionScaled;
            settings.tessLevelInner = Gs::Clamp(settings.tessLevelInner, tessLevelMin, tessLevelMax);
        }

        if (input->KeyPressed(LLGL::Key::RButton))
        {
            settings.tessLevelOuter += motionScaled;
            settings.tessLevelOuter = Gs::Clamp(settings.tessLevelOuter, tessLevelMin, tessLevelMax);
        }

        if ( motion != 0 && ( input->KeyPressed(LLGL::Key::LButton) || input->KeyPressed(LLGL::Key::RButton) ) )
            ShowTessLevel();

        if (input->KeyPressed(LLGL::Key::MButton))
            settings.twist += Gs::Deg2Rad(motionScaled);

        if (input->KeyDown(LLGL::Key::Tab))
            showWireframe = !showWireframe;

        // Update matrices
        Gs::Matrix4f worldMatrix;
        Gs::Translate(worldMatrix, Gs::Vector3f(0, 0, 5));

        settings.wvpMatrix = projection * worldMatrix;

        #ifdef AUTO_ROTATE
        static float rotation;
        rotation += 0.0025f;
        Gs::RotateFree(settings.worldMatrix, Gs::Vector3f(1, 1, 1).Normalized(), rotation);
        #endif
    }

    void DrawScene()
    {
        commands->Begin();
        {
            // Update constant buffer
            UpdateBuffer(constantBuffer, settings);

            // Set hardware buffers to draw the model
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetIndexBuffer(*indexBuffer);

            // Set the render context as the initial render target
            #ifdef ENABLE_RENDER_PASS
            commands->BeginRenderPass(*context, renderPass);
            #else
            commands->BeginRenderPass(*context);

            // Clear color- and depth buffers
            commands->Clear(LLGL::ClearFlags::ColorDepth);
            #endif
            {
                // Set viewport
                const auto resolution = context->GetVideoMode().resolution;
                commands->SetViewport(LLGL::Viewport{ { 0, 0 }, resolution });

                // Set graphics pipeline with the shader
                commands->SetGraphicsPipeline(*pipeline[showWireframe ? 1 : 0]);

                if (resourceHeap)
                {
                    // Bind resource view heap to graphics pipeline
                    commands->SetGraphicsResourceHeap(*resourceHeap, 0);
                }
                else
                {
                    // Set constant buffer only to tessellation shader stages
                    #ifdef _TEST_BUFFER_ARRAY_
                    commands->SetConstantBufferArray(*constantBufferArray, constantBufferIndex, LLGL::StageFlags::AllTessStages);
                    #else
                    commandsExt->SetConstantBuffer(*constantBuffer, constantBufferIndex, LLGL::StageFlags::AllTessStages);
                    #endif
                }

                // Draw tessellated quads with 24=4*6 vertices from patches of 4 control points
                commands->DrawIndexed(24, 0);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        context->Present();

        //renderer->GetCommandQueue()->WaitIdle();
    }

    void OnDrawFrame() override
    {
        UpdateUserInput();
        DrawScene();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Tessellation);



