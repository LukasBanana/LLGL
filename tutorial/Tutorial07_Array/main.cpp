/*
 * main.cpp (Tutorial07_Array)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../tutorial.h"


class Tutorial07 : public Tutorial
{

    LLGL::ShaderProgram*    shaderProgram       = nullptr;

    LLGL::GraphicsPipeline* pipeline            = nullptr;

    LLGL::Buffer*           vertexBuffers[3]    = { nullptr };
    LLGL::BufferArray*      vertexBufferArray   = nullptr;

public:

    Tutorial07() :
        Tutorial( L"LLGL Tutorial 07: Array" )
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram(vertexFormat);
        CreatePipelines();
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex formats
        LLGL::VertexFormat vertexFormatPositions;
        vertexFormatPositions.AppendAttribute({ "position", LLGL::VectorType::Float2 });

        LLGL::VertexFormat vertexFormatColors;
        vertexFormatColors.AppendAttribute({ "color", LLGL::VectorType::Float3 });

        LLGL::VertexFormat vertexFormatInstanceData;
        vertexFormatInstanceData.AppendAttribute({ "instanceColor",  LLGL::VectorType::Float3, 1 });
        vertexFormatInstanceData.AppendAttribute({ "instanceOffset", LLGL::VectorType::Float2, 1 });
        vertexFormatInstanceData.AppendAttribute({ "instanceScale",  LLGL::VectorType::Float,  1 });

        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttributes(vertexFormatPositions);
        vertexFormat.AppendAttributes(vertexFormatColors);
        vertexFormat.AppendAttributes(vertexFormatInstanceData);

        // Initialize buffer data
        Gs::Vector2f vertexPositions[] =
        {
            {  0,  1 },
            {  1, -1 },
            { -1, -1 },
        };

        LLGL::ColorRGBf vertexColors[] =
        {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
        };

        struct InstanceData
        {
            LLGL::ColorRGBf color;
            Gs::Vector2f    offset;
            float           scale;
        }
        instanceData[] =
        {
            { { 1   , 1   , 1    }, { -0.5f,  0.5f },  0.4f },
            { { 1   , 2   , 3    }, {  0.5f,  0.5f }, -0.4f },
            { { 1   , 0.2f, 0.2f }, {  0.5f, -0.5f },  0.2f },
            { { 0.2f, 1   , 0.2f }, { -0.5f, -0.5f },  0.3f },
        };

        // Create buffer for vertex positions
        LLGL::BufferDescriptor desc;
        
        desc.type                   = LLGL::BufferType::Vertex;
        desc.size                   = sizeof(vertexPositions);
        desc.vertexBuffer.format    = vertexFormatPositions;
        
        vertexBuffers[0] = renderer->CreateBuffer(desc, vertexPositions);

        // Create buffer for vertex colors
        desc.size                   = sizeof(vertexColors);
        desc.vertexBuffer.format    = vertexFormatColors;

        vertexBuffers[1] = renderer->CreateBuffer(desc, vertexColors);

        // Create buffer for instance data
        desc.size                   = sizeof(instanceData);
        desc.vertexBuffer.format    = vertexFormatInstanceData;

        vertexBuffers[2] = renderer->CreateBuffer(desc, instanceData);

        // Create vertex buffer array
        vertexBufferArray = renderer->CreateBufferArray(3, vertexBuffers);

        return vertexFormat;
    }

    void CreatePipelines()
    {
        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

private:

    void OnDrawFrame() override
    {
        // Clear color buffer
        commands->ClearBuffers(LLGL::ClearBuffersFlags::Color);

        // Set buffer array
        commands->SetVertexBufferArray(*vertexBufferArray);

        // Set graphics pipeline state
        commands->SetGraphicsPipeline(*pipeline);

        // Draw 4 instances of the triangle with 3 vertices each
        commands->DrawInstanced(3, 0, 4);

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial07);



