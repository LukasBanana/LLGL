/*
 * main.cpp (Tutorial04_Query)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <tutorial.h>


class Tutorial04 : public Tutorial
{

    LLGL::ShaderProgram*    shaderProgram           = nullptr;

    LLGL::GraphicsPipeline* occlusionPipeline       = nullptr;
    LLGL::GraphicsPipeline* scenePipeline           = nullptr;

    LLGL::PipelineLayout*   pipelineLayout          = nullptr;
    LLGL::ResourceHeap*     resourceHeap            = nullptr;

    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           indexBuffer             = nullptr;
    LLGL::Buffer*           constantBuffer          = nullptr;

    LLGL::QueryHeap*            occlusionQuery          = nullptr;
    LLGL::QueryHeap*            geometryQuery           = nullptr;

    struct Settings
    {
        Gs::Matrix4f        wvpMatrix;
        LLGL::ColorRGBAf    color;
    }
    settings;

public:

    Tutorial04() :
        Tutorial { L"LLGL Tutorial 04: Query" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram({ vertexFormat });
        CreatePipelines();
        CreateQueries();
        CreateResourceHeaps();
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

        // Create vertex, index, and constant buffer
        vertexBuffer = CreateVertexBuffer(GenerateCubeVertices(), vertexFormat);
        indexBuffer = CreateIndexBuffer(GenerateCubeTriangleIndices(), LLGL::DataType::UInt32);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(0):vert:frag"));

        // Create graphics pipeline for occlusion query
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.pipelineLayout             = pipelineLayout;

            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);

            LLGL::BlendTargetDescriptor blendDesc;
            {
                blendDesc.colorMask = LLGL::ColorRGBAb(false);
            }
            pipelineDesc.blend.targets.push_back(blendDesc);
        }
        occlusionPipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Create graphics pipeline for scene rendering
        {
            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.depth.writeEnabled         = true;
            pipelineDesc.blend.targets[0].colorMask = LLGL::ColorRGBAb(true);
        }
        scenePipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
    }

    void CreateQueries()
    {
        // Create query to determine if any samples passed the depth test (occlusion query)
        LLGL::QueryHeapDescriptor queryDesc;
        {
            queryDesc.type              = LLGL::QueryType::AnySamplesPassed;
            queryDesc.renderCondition   = true;
        }
        occlusionQuery = renderer->CreateQueryHeap(queryDesc);

        // Create query to determine number of primitives that are sent to the rasterizer
        {
            queryDesc.type              = LLGL::QueryType::PipelineStatistics;
            queryDesc.renderCondition   = false;
        }
        geometryQuery = renderer->CreateQueryHeap(queryDesc);
    }

    void CreateResourceHeaps()
    {
        // Create resource heap for constant buffer
        LLGL::ResourceHeapDescriptor heapDesc;
        {
            heapDesc.pipelineLayout = pipelineLayout;
            heapDesc.resourceViews  = { constantBuffer };
        }
        resourceHeap = renderer->CreateResourceHeap(heapDesc);
    }

    std::uint64_t GetAndSyncQueryResult(LLGL::QueryHeap* query)
    {
        // Wait until query result is available and return result
        std::uint64_t result = 0;

        if (query->GetType() == LLGL::QueryType::PipelineStatistics)
        {
            LLGL::QueryPipelineStatistics statistics;
            while (!commandQueue->QueryResult(*query, 0, 1, &statistics, sizeof(statistics))) { /* wait */ }
            result = statistics.numPrimitivesGenerated;
        }
        else
        {
            while (!commandQueue->QueryResult(*query, 0, 1, &result, sizeof(result))) { /* wait */ }
        }

        return result;
    }

    void PrintQueryResult()
    {
        std::cout << "primitives generated: " << GetAndSyncQueryResult(geometryQuery);
        std::cout << "                         \r";
        std::flush(std::cout);
    }

    void SetBoxColor(const LLGL::ColorRGBAf& color)
    {
        settings.color = color;
        UpdateBuffer(constantBuffer, settings, true);
    }

private:

    void OnDrawFrame() override
    {
        // Update matrices in constant buffer
        static float anim;
        anim += 0.01f;

        settings.wvpMatrix = projection;
        Gs::RotateFree(settings.wvpMatrix, { 0, 1, 0 }, Gs::Deg2Rad(std::sin(anim)*55.0f));
        Gs::Translate(settings.wvpMatrix, { 0, 0, 5 });
        Gs::RotateFree(settings.wvpMatrix, Gs::Vector3f(1).Normalized(), anim*3);

        commands->Begin();
        {
            // Set buffers
            commands->SetVertexBuffer(*vertexBuffer);
            commands->SetIndexBuffer(*indexBuffer);

            // Start with qeometry query
            commands->BeginQuery(*geometryQuery);
            {
                commands->SetViewport(LLGL::Viewport{ { 0, 0 }, context->GetResolution() });

                SetBoxColor({ 1, 1, 1 });
                commands->BeginRenderPass(*context);
                {
                    // Clear color and depth buffers
                    commands->Clear(LLGL::ClearFlags::ColorDepth);

                    // Draw box for occlusion query
                    commands->SetGraphicsPipeline(*occlusionPipeline);
                    commands->SetGraphicsResourceHeap(*resourceHeap);

                    commands->BeginQuery(*occlusionQuery);
                    {
                        commands->DrawIndexed(36, 0);
                    }
                    commands->EndQuery(*occlusionQuery);
                }
                commands->EndRenderPass();

                SetBoxColor({ 0, 1, 0 });
                commands->BeginRenderPass(*context);
                {
                    // Draw scene
                    commands->SetGraphicsPipeline(*scenePipeline);

                    commands->BeginRenderCondition(*occlusionQuery);
                    {
                        commands->DrawIndexed(36, 0);
                    }
                    commands->EndRenderCondition();
                }
                commands->EndRenderPass();
            }
            commands->EndQuery(*geometryQuery);
        }
        commands->End();
        commandQueue->Submit(*commands);

        PrintQueryResult();

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_TUTORIAL(Tutorial04);



