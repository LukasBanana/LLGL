/*
 * main.cpp (Tutorial02_Tessellation)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


// TODO: TESSELLATION SHADER NOT YET CREATED !!!

class Tutorial02 : public Tutorial
{

    LLGL::ShaderProgram*    shaderProgram       = nullptr;
    LLGL::GraphicsPipeline* pipeline            = nullptr;

    LLGL::VertexBuffer*     vertexBuffer        = nullptr;
    LLGL::IndexBuffer*      indexBuffer         = nullptr;
    LLGL::ConstantBuffer*   constantBuffer      = nullptr;

    unsigned int            constantBufferIndex = 0;

    struct Matrices
    {
        Gs::Matrix4f    projectionMatrix;
        Gs::Matrix4f    viewMatrix;
        Gs::Matrix4f    worldMatrix;
        float           tessLevelInner  = 1.0f;
        float           tessLevelOuter  = 1.0f;
        float           _pad0[2];
    }
    matrices;

public:

    Tutorial02() :
        Tutorial( "OpenGL", L"LLGL Tutorial 02: Tessellation" )
    {
        // Check if constant buffers are supported
        auto renderCaps = renderer->QueryRenderingCaps();

        if (!renderCaps.hasConstantBuffers)
            throw std::runtime_error("constant buffers are not supported by this renderer");

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

            // Enable back-face culling
            pipelineDesc.rasterizer.polygonMode         = LLGL::PolygonMode::Wireframe;
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
            pipelineDesc.rasterizer.frontCCW            = true;
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AddAttribute("position", LLGL::DataType::Float, 3);

        // Create vertex- and index buffers for a simple 3D cube model
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeQuadlIndices(), LLGL::DataType::UInt);

        // Create constant buffer
        constantBuffer = renderer->CreateConstantBuffer();
        renderer->SetupConstantBuffer(*constantBuffer, nullptr, sizeof(Matrices), LLGL::BufferUsage::Dynamic);

        // Set initial matrices
        auto resolution = context->GetVideoMode().resolution.Cast<float>();

        matrices.projectionMatrix = Gs::ProjectionMatrix4f::Perspective(
            (resolution.x / resolution.y),  // aspect ratio
            0.1f,                           // near clipping plane
            100.0f,                         // far clipping plane
            Gs::Deg2Rad(45.0f),             // field-of-view from degree to radians
            Gs::ProjectionFlags::OpenGLPreset
        ).ToMatrix4();
    }

private:

    void OnDrawFrame() override
    {
        // Update matrices
        static float rotation;
        rotation += 0.01f;
        matrices.worldMatrix.LoadIdentity();
        Gs::Translate(matrices.worldMatrix, Gs::Vector3f(0, 0, -5));
        Gs::RotateFree(matrices.worldMatrix, Gs::Vector3f(1, 1, 1).Normalized(), rotation);

        // Clear color- and depth buffers
        context->ClearBuffers(LLGL::ClearBuffersFlags::Color | LLGL::ClearBuffersFlags::Depth);

        // Update constant buffer
        renderer->WriteConstantBuffer(*constantBuffer, &matrices, sizeof(matrices), 0);

        // Bind hardware buffers to draw the model
        context->BindVertexBuffer(*vertexBuffer);
        context->BindIndexBuffer(*indexBuffer);
        context->BindConstantBuffer(constantBufferIndex, *constantBuffer);

        // Bind graphics pipeline with the shader
        context->BindGraphicsPipeline(*pipeline);

        // Draw indexed mesh
        context->SetDrawMode(LLGL::DrawMode::Patches);
        context->DrawIndexed(24, 0);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial02);



