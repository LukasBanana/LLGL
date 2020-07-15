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

    LLGL::ShaderProgram*    shaderProgram   = nullptr;
    LLGL::PipelineState*    pipeline        = nullptr;
    LLGL::Buffer*           vertexBuffer    = nullptr;
    LLGL::Buffer*           indexBuffer     = nullptr;

    std::uint64_t           indexOffset16   = 0;
    std::uint32_t           indexCount16    = 0;

    std::uint64_t           indexOffset32   = 0;
    std::uint32_t           indexCount32    = 0;

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

    void AddSquare(
        float                       centerX,
        float                       centerY,
        float                       size,
        std::vector<Vertex>&        vertices,
        std::vector<std::uint16_t>& indices,
        std::uint32_t&              indexCount,
        bool                        indexType16Bits)
    {
        float left = centerX - size / 2.0f;
        float right = centerX + size / 2.0f;
        float top = centerY + size / 2.0f;
        float bottom = centerY - size / 2.0f;

        const auto startIndex = vertices.size();
        vertices.push_back({{ right, top    }, { 255,   0,   0, 255 } });
        vertices.push_back({{ right, bottom }, {   0, 255,   0, 255 } });
        vertices.push_back({{ left,  top    }, {   0,   0, 255, 255 } });
        vertices.push_back({{ left,  bottom }, { 255, 255, 255, 255 } });

        auto AddIndex = [&indices, &indexCount, indexType16Bits](std::size_t idx)
        {
            if (indexType16Bits)
            {
                // Add single 16-bit index to buffer
                indices.push_back(static_cast<std::uint16_t>(idx & 0xFFFF));
            }
            else
            {
                // Add split 32-bit index to buffer
                indices.push_back(static_cast<std::uint16_t>(idx & 0xFFFF));
                indices.push_back(static_cast<std::uint16_t>((idx >> 16) & 0xFFFF));
            }

            // Only count a single index, even if we have to add two 16-bit entries for a single 32-bit index.
            ++indexCount;
        };

        AddIndex(startIndex);
        AddIndex(startIndex + 1);
        AddIndex(startIndex + 2);
        AddIndex(startIndex + 3);
        AddIndex(0xFFFFFFFF);
    }

    std::vector<LLGL::VertexFormat> CreateBuffers()
    {
        std::vector<Vertex>         vertices;
        std::vector<std::uint16_t>  indices;

        // Add 16-bit indices
        indexOffset16 = indices.size() * sizeof(std::uint16_t);
        AddSquare( .5f,  .5f, 0.8f, vertices, indices, indexCount16, /*indexType16Bits:*/ true);
        AddSquare(-.5f,  .5f, 0.8f, vertices, indices, indexCount16, /*indexType16Bits:*/ true);

        // Add 32-bit indices
        indexOffset32 = indices.size() * sizeof(std::uint16_t);
        AddSquare( .5f, -.5f, 0.8f, vertices, indices, indexCount32, /*indexType16Bits:*/ false);
        AddSquare(-.5f, -.5f, 0.8f, vertices, indices, indexCount32, /*indexType16Bits:*/ false);

        // Setup vertex format: 2D float vector for position, 4D unsigned byte vector for color
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });
        vertexFormat.AppendAttribute({ "color",    LLGL::Format::RGBA8UNorm });
        vertexFormat.SetStride(sizeof(Vertex));

        // Create vertex buffer
        LLGL::BufferDescriptor vertexBufferDesc;
        {
            vertexBufferDesc.size           = vertices.size() * sizeof(Vertex);
            vertexBufferDesc.bindFlags      = LLGL::BindFlags::VertexBuffer;
            vertexBufferDesc.vertexAttribs  = vertexFormat.attributes;
        }
        vertexBuffer = renderer->CreateBuffer(vertexBufferDesc, vertices.data());

        // Create index buffer (contains both 16 and 32 bit indices)
        LLGL::BufferDescriptor indexBufferDesc;
        {
            indexBufferDesc.size       = indices.size() * sizeof(std::uint16_t);
            indexBufferDesc.bindFlags  = LLGL::BindFlags::IndexBuffer;
        }
        indexBuffer = renderer->CreateBuffer(indexBufferDesc, indices.data());

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

                if (indexCount16 > 0)
                {
                    commands->SetIndexBuffer(*indexBuffer, LLGL::Format::R16UInt, indexOffset16);
                    commands->DrawIndexed(indexCount16, 0);
                }

                if (indexCount32 > 0)
                {
                    commands->SetIndexBuffer(*indexBuffer, LLGL::Format::R32UInt, indexOffset32);
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
