/*
 * main.cpp (Tutorial07_Array)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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
        Tutorial { L"LLGL Tutorial 07: Array" }
    {
        // Create all graphics objects
        auto vertexFormats = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram(vertexFormats);
        CreatePipelines();
    }

    std::vector<LLGL::VertexFormat> CreateBuffers()
    {
        // Specify vertex formats
        LLGL::VertexFormat vertexFormatPositions;
        vertexFormatPositions.inputSlot = 0;
        vertexFormatPositions.AppendAttribute({ "position", LLGL::VectorType::Float2 });

        LLGL::VertexFormat vertexFormatColors;
        vertexFormatColors.inputSlot = 1;
        vertexFormatColors.AppendAttribute({ "color", LLGL::VectorType::Float3 });

        LLGL::VertexFormat vertexFormatInstanceData;
        vertexFormatInstanceData.inputSlot = 2;
        vertexFormatInstanceData.AppendAttribute({ "instanceColor",  LLGL::VectorType::Float3, 1 });
        vertexFormatInstanceData.AppendAttribute({ "instanceOffset", LLGL::VectorType::Float2, 1 });
        vertexFormatInstanceData.AppendAttribute({ "instanceScale",  LLGL::VectorType::Float,  1 });

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

        return { vertexFormatPositions, vertexFormatColors, vertexFormatInstanceData };
    }

    void CreatePipelines()
    {
        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);

            #if 1//TODO: for Vulkan
            const auto resolution = context->GetVideoMode().resolution;
            const auto viewportSize = resolution.Cast<float>();
            pipelineDesc.viewports.push_back(LLGL::Viewport(0.0f, 0.0f, viewportSize.x, viewportSize.y));
            pipelineDesc.scissors.push_back(LLGL::Scissor(0, 0, resolution.x, resolution.y));
            pipelineDesc.blend.targets.push_back({});
            #endif
        }
        pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

private:

    void OnDrawFrame() override
    {
        // Set the render context as the initial render target
        commands->SetRenderTarget(*context);

        // Clear color buffer
        commands->Clear(LLGL::ClearFlags::Color);

        // Set buffer array
        commands->SetVertexBufferArray(*vertexBufferArray);

        // Set graphics pipeline state
        commands->SetGraphicsPipeline(*pipeline);

        // Draw 4 instances of the triangle with 3 vertices each
        commands->DrawInstanced(3, 0, 4);

        // Present result on the screen
        context->Present();

        renderer->GetCommandQueue()->WaitForFinish();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial07);



