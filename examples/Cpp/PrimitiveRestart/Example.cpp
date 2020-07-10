/*
 * Example.cpp (Example_PrimitiveRestart)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <vector>

class Example_PrimitiveRestart : public ExampleBase
{
    LLGL::ShaderProgram*    shaderProgram       = nullptr;
    LLGL::PipelineState*    pipeline            = nullptr;
    LLGL::Buffer*           vertexBuffer        = nullptr;
    LLGL::Buffer*           indexBuffer16       = nullptr;
    LLGL::Buffer*           indexBuffer32       = nullptr;

    size_t                  indexCount16        = 0;
    size_t                  indexCount32        = 0;

public:

    Example_PrimitiveRestart() :
    ExampleBase { L"LLGL Example: PrimitiveRestart" }
    {
        auto vertexFormats = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram(vertexFormats);
        CreatePipeline();
    }

private:

    // Vertex data structure
    struct Vertex
    {
        Gs::Vector2f        position;
        LLGL::ColorRGBAub   color;
    };

    template <typename T>
    static void AddSquare(float centerX, float centerY, float size, std::vector<Vertex>& vertices, std::vector<T>& indices)
    {
        float left = centerX - size / 2.0;
        float right = centerX + size / 2.0;
        float top = centerY + size / 2.0;
        float bottom = centerY - size / 2.0;

        const T startIndex = static_cast<T>(vertices.size());
        vertices.push_back({{ right, top    }, { 255,   0,   0, 255 } });
        vertices.push_back({{ right, bottom }, {   0, 255,   0, 255 } });
        vertices.push_back({{ left,  top    }, {   0,   0, 255, 255 } });
        vertices.push_back({{ left,  bottom }, { 255, 255, 255, 255 } });

        if (!indices.empty())
            indices.push_back(T(0xFFFFFFFF));
        indices.push_back(startIndex);
        indices.push_back(startIndex + 1);
        indices.push_back(startIndex + 2);
        indices.push_back(startIndex + 3);
    }

    std::vector<LLGL::VertexFormat> CreateBuffers()
    {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices16;
        std::vector<uint32_t> indices32;

        AddSquare( .5f,  .5f, 0.8f, vertices, indices16);
        AddSquare(-.5f,  .5f, 0.8f, vertices, indices16);

        AddSquare( .5f, -.5f, 0.8f, vertices, indices32);
        AddSquare(-.5f, -.5f, 0.8f, vertices, indices32);

        indexCount16 = indices16.size();
        indexCount32 = indices32.size();

        // Vertex format
        LLGL::VertexFormat vertexFormat;

        // Append 2D float vector for position attribute
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });

        // Append 3D unsigned byte vector for color
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });

        // Update stride in case out vertex structure is not 4-byte aligned
        vertexFormat.SetStride(sizeof(Vertex));

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = vertices.size() * sizeof(Vertex);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, &vertices[0]);

        {
            LLGL::BufferDescriptor indexBufferDesc16;
            indexBufferDesc16.size           = indices16.size() * 2;
            indexBufferDesc16.format         = LLGL::Format::R16UInt;
            indexBufferDesc16.bindFlags      = LLGL::BindFlags::IndexBuffer;
            indexBuffer16 = renderer->CreateBuffer(indexBufferDesc16, &indices16[0]);
        }

        {
            LLGL::BufferDescriptor indexBufferDesc32;
            indexBufferDesc32.size           = indices32.size() * 4;
            indexBufferDesc32.format         = LLGL::Format::R32UInt;
            indexBufferDesc32.bindFlags      = LLGL::BindFlags::IndexBuffer;
            indexBuffer32 = renderer->CreateBuffer(indexBufferDesc32, &indices32[0]);
        }

        return { vertexFormat };
    }

    void CreatePipeline()
    {
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.primitiveTopology              = LLGL::PrimitiveTopology::TriangleStrip;
            pipelineDesc.renderPass                     = context->GetRenderPass();
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
        }
        pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

private:
    void OnDrawFrame() override
    {
        // Begin recording commands
        commands->Begin();
        {
            // Set viewport and scissor rectangle
            commands->SetViewport(context->GetResolution());

            // Set graphics pipeline
            commands->SetPipelineState(*pipeline);

            // Set vertex buffer
            commands->SetVertexBuffer(*vertexBuffer);

            // Set the render context as the initial render target
            commands->BeginRenderPass(*context);
            {
                // Clear color buffer
                commands->Clear(LLGL::ClearFlags::Color);

                if (indexCount16)
                {
                    commands->SetIndexBuffer(*indexBuffer16);
                    commands->DrawIndexed(indexCount16, 0);
                }

                if (indexCount32)
                {
                    commands->SetIndexBuffer(*indexBuffer32);
                    commands->DrawIndexed(indexCount32, 0);
                }
            }
            commands->EndRenderPass();
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Present the result on the screen
        context->Present();
    }
};

LLGL_IMPLEMENT_EXAMPLE(Example_PrimitiveRestart);
