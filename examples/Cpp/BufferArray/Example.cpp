/*
 * Example.cpp (Example_BufferArray)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>


class Example_BufferArray : public ExampleBase
{

    LLGL::PipelineState*    pipeline            = nullptr;

    LLGL::Buffer*           vertexBuffers[3]    = { nullptr };
    LLGL::BufferArray*      vertexBufferArray   = nullptr;

public:

    Example_BufferArray() :
        ExampleBase { L"LLGL Example: BufferArray" }
    {
        // Create all graphics objects
        auto vertexFormats = CreateBuffers();
        CreatePipelines(vertexFormats);
    }

    std::vector<LLGL::VertexFormat> CreateBuffers()
    {
        // Initialize buffer data
        float vertexPositions[][2] =
        {
            {  0,  1 },
            {  1, -1 },
            { -1, -1 },
        };

        float vertexColors[][3] =
        {
            { 1, 0, 0 },
            { 0, 1, 0 },
            { 0, 0, 1 },
        };

        struct InstanceData
        {
            float color[3];
            float offset[2];
            float scale;
        }
        instanceData[] =
        {
            { { 1   , 1   , 1    }, { -0.5f,  0.5f },  0.4f },
            { { 1   , 2   , 3    }, {  0.5f,  0.5f }, -0.4f },
            { { 1   , 0.2f, 0.2f }, {  0.5f, -0.5f },  0.2f },
            { { 0.2f, 1   , 0.2f }, { -0.5f, -0.5f },  0.3f },
        };

        // Specify vertex formats
        LLGL::VertexFormat vertexFormatPositions;
        vertexFormatPositions.attributes =
        {
            LLGL::VertexAttribute{ "position", LLGL::Format::RG32Float, 0, 0, sizeof(float[2]), 0 }
        };

        LLGL::VertexFormat vertexFormatColors;
        vertexFormatColors.attributes =
        {
            LLGL::VertexAttribute{ "color", LLGL::Format::RGB32Float, 1, 0, sizeof(float[3]), 1 }
        };

        LLGL::VertexFormat vertexFormatInstanceData;
        vertexFormatInstanceData.attributes =
        {
            LLGL::VertexAttribute{ "instanceColor",  LLGL::Format::RGB32Float, 2,  0, sizeof(InstanceData), 2, 1 },
            LLGL::VertexAttribute{ "instanceOffset", LLGL::Format::RG32Float,  3, 12, sizeof(InstanceData), 2, 1 },
            LLGL::VertexAttribute{ "instanceScale",  LLGL::Format::R32Float,   4, 20, sizeof(InstanceData), 2, 1 }
        };

        // Create buffer for vertex positions
        LLGL::BufferDescriptor desc;

        desc.size           = sizeof(vertexPositions);
        desc.bindFlags      = LLGL::BindFlags::VertexBuffer;
        desc.vertexAttribs  = vertexFormatPositions.attributes;

        vertexBuffers[0] = renderer->CreateBuffer(desc, vertexPositions);

        // Create buffer for vertex colors
        desc.size           = sizeof(vertexColors);
        desc.vertexAttribs  = vertexFormatColors.attributes;

        vertexBuffers[1] = renderer->CreateBuffer(desc, vertexColors);

        // Create buffer for instance data
        desc.size           = sizeof(instanceData);
        desc.vertexAttribs  = vertexFormatInstanceData.attributes;

        vertexBuffers[2] = renderer->CreateBuffer(desc, instanceData);

        // Create vertex buffer array
        vertexBufferArray = renderer->CreateBufferArray(3, vertexBuffers);

        return { vertexFormatPositions, vertexFormatColors, vertexFormatInstanceData };
    }

    void CreatePipelines(const std::vector<LLGL::VertexFormat>& vertexFormats)
    {
        // Create common graphics pipeline for scene rendering
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = LoadStandardVertexShader("VS", vertexFormats);
            pipelineDesc.fragmentShader                 = LoadStandardFragmentShader("PS");
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

private:

    void OnDrawFrame() override
    {
        commands->Begin();
        {
            // Set buffer array
            commands->SetVertexBufferArray(*vertexBufferArray);

            // Set the swap-chain as the initial render target
            commands->BeginRenderPass(*swapChain);
            {
                // Clear color buffer
                commands->Clear(LLGL::ClearFlags::Color, backgroundColor);

                // Set viewports
                commands->SetViewport(swapChain->GetResolution());

                // Set graphics pipeline state
                commands->SetPipelineState(*pipeline);

                // Draw 4 instances of the triangle with 3 vertices each
                commands->DrawInstanced(3, 0, 4);
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present result on the screen
        swapChain->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_BufferArray);



