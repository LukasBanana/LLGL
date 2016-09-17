/*
 * main.cpp (Tutorial02_Tessellation)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


// Show scene in wireframe polygon mode
#define SHOW_WIREFRAME

// Automatically rotate the model
//#define AUTO_ROTATE


class Tutorial02 : public Tutorial
{

    LLGL::ShaderProgram*    shaderProgram       = nullptr;
    LLGL::GraphicsPipeline* pipeline            = nullptr;

    LLGL::VertexBuffer*     vertexBuffer        = nullptr;
    LLGL::IndexBuffer*      indexBuffer         = nullptr;
    LLGL::ConstantBuffer*   constantBuffer      = nullptr;

    unsigned int            constantBufferIndex = 0;

    struct Settings
    {
        Gs::Matrix4f    projectionMatrix;
        Gs::Matrix4f    viewMatrix;
        Gs::Matrix4f    worldMatrix;
        float           tessLevelInner  = 5.0f;
        float           tessLevelOuter  = 5.0f;
        float           twist           = 0.0f;
        float           _pad0;                  // <-- padding for 16 byte pack alignment of constant buffers
    }
    settings;

public:

    Tutorial02() :
        Tutorial( "OpenGL", L"LLGL Tutorial 02: Tessellation" )
    {
        // Check if constant buffers are supported
        auto renderCaps = renderer->QueryRenderingCaps();

        if (!renderCaps.hasConstantBuffers)
            throw std::runtime_error("constant buffers are not supported by this renderer");
        if (!renderCaps.hasTessellationShaders)
            throw std::runtime_error("tessellation shaders are not supported by this renderer");

        // Load shader program
        shaderProgram = LoadShaderProgram(
            { { LLGL::ShaderType::Vertex, "vertex.glsl" },
              { LLGL::ShaderType::TessControl, "tesscontrol.glsl" },
              { LLGL::ShaderType::TessEvaluation, "tesseval.glsl" },
              { LLGL::ShaderType::Fragment, "fragment.glsl" } }
        );

        // Bind constant buffer location to the index we use later with the render context
        shaderProgram->BindConstantBuffer("Settings", constantBufferIndex);

        // Create graphics pipeline
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            // Set shader program
            pipelineDesc.shaderProgram                  = shaderProgram;

            // Enable multi-sample anti-aliasing
            pipelineDesc.rasterizer.multiSampleEnabled  = true;
            pipelineDesc.rasterizer.samples             = 8;

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            // Set polygon mode to wireframe (if macro is enabled)
            #ifdef SHOW_WIREFRAME
            pipelineDesc.rasterizer.polygonMode         = LLGL::PolygonMode::Wireframe;
            #endif

            // Enable back-face culling
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            pipelineDesc.rasterizer.frontCCW            = true;
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AddAttribute("position", LLGL::DataType::Float32, 3);

        // Create vertex- and index buffers for a simple 3D cube model
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeQuadlIndices(), LLGL::DataType::UInt32);

        // Create constant buffer
        constantBuffer = renderer->CreateConstantBuffer();
        renderer->SetupConstantBuffer(*constantBuffer, nullptr, sizeof(Settings), LLGL::BufferUsage::Dynamic);

        // Set initial matrices
        auto resolution = context->GetVideoMode().resolution.Cast<float>();

        settings.projectionMatrix = Gs::ProjectionMatrix4f::Perspective(
            (resolution.x / resolution.y),  // aspect ratio
            0.1f,                           // near clipping plane
            100.0f,                         // far clipping plane
            Gs::Deg2Rad(45.0f)              // field-of-view from degree to radians
        ).ToMatrix4();

        // Show info to user
        std::cout << "press left mouse button and move mouse on X axis to increase/decrease inner tessellation" << std::endl;
        std::cout << "press right mouse button and move mouse on X axis to increase/decrease outer tessellation" << std::endl;
        std::cout << "press middle mouse button and move mouse on X axis to increase/decrease twist" << std::endl;
        ShowTessLevel();
    }

    void ShowTessLevel()
    {
        std::cout << "tessellation level (inner = " << settings.tessLevelInner << ", outer = " << settings.tessLevelOuter << ")      \r";
        std::flush(std::cout);
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

        // Update matrices
        settings.worldMatrix.LoadIdentity();
        Gs::Translate(settings.worldMatrix, Gs::Vector3f(0, 0, 5));

        #ifdef AUTO_ROTATE
        static float rotation;
        rotation += 0.0025f;
        Gs::RotateFree(settings.worldMatrix, Gs::Vector3f(1, 1, 1).Normalized(), rotation);
        #endif
    }

    void DrawScene()
    {
        // Clear color- and depth buffers
        context->ClearBuffers(LLGL::ClearBuffersFlags::Color | LLGL::ClearBuffersFlags::Depth);

        // Update constant buffer
        renderer->WriteConstantBuffer(*constantBuffer, &settings, sizeof(settings), 0);

        // Set hardware buffers to draw the model
        context->SetVertexBuffer(*vertexBuffer);
        context->SetIndexBuffer(*indexBuffer);
        context->SetConstantBuffer(*constantBuffer, constantBufferIndex);

        // Set graphics pipeline with the shader
        context->SetGraphicsPipeline(*pipeline);

        // Draw tessellated quads with 24=4*6 vertices from patches of 4 control points
        context->SetPrimitiveTopology(LLGL::PrimitiveTopology::Patches4);
        context->DrawIndexed(24, 0);

        // Present result on the screen
        context->Present();
    }

    void OnDrawFrame() override
    {
        UpdateUserInput();
        DrawScene();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial02);



