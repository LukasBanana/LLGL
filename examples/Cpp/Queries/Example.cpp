/*
 * Example.cpp (Example_Queries)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <chrono>
#include <thread>


class Example_Queries : public ExampleBase
{

    ShaderPipeline          shaderPipeline;

    LLGL::PipelineState*    occlusionPipeline       = nullptr;
    LLGL::PipelineState*    scenePipeline           = nullptr;

    LLGL::PipelineLayout*   pipelineLayout          = nullptr;

    LLGL::Buffer*           vertexBuffer            = nullptr;
    LLGL::Buffer*           constantBuffer          = nullptr;

    LLGL::QueryHeap*        occlusionQuery          = nullptr;
    LLGL::QueryHeap*        geometryQuery           = nullptr;
    LLGL::QueryHeap*        timerQuery              = nullptr;

    Gs::Matrix4f            modelTransform[2];
    bool                    animEnabled             = true;

    using Clock     = std::chrono::system_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Ticks     = std::chrono::milliseconds;

    TimePoint               prevPrintTime           = Clock::now();
    const long long         printRefreshRate        = 100;

    struct Model
    {
        std::uint32_t numVertices = 0;
        std::uint32_t firstVertex = 0;
    };

    Model                   model0;

    struct Settings
    {
        Gs::Matrix4f        wvpMatrix;
        Gs::Matrix4f        wMatrix;
        LLGL::ColorRGBAf    color;
    }
    settings;

public:

    Example_Queries() :
        ExampleBase { "LLGL Example: Query" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderPipeline = LoadStandardShaderPipeline({ vertexFormat });
        CreatePipelines();
        CreateQueries();

        // Show info
        LLGL::Log::Printf("press SPACE KEY to enable/disable animation of occluder\n");
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
        vertexFormat.SetStride(sizeof(TexturedVertex));

        // Load models
        auto vertices = LoadObjModel("Pyramid.obj");
        model0.numVertices = static_cast<std::uint32_t>(vertices.size());

        // Create vertex and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        constantBuffer = CreateConstantBuffer(settings);

        return vertexFormat;
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(LLGL::Parse("cbuffer(1):vert:frag"));

        // Create graphics pipeline for occlusion query
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            pipelineDesc.vertexShader                   = shaderPipeline.vs;
            pipelineDesc.fragmentShader                 = shaderPipeline.ps;
            pipelineDesc.pipelineLayout                 = pipelineLayout;

            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;

            pipelineDesc.blend.targets[0].colorMask     = 0x0;
        }
        occlusionPipeline = renderer->CreatePipelineState(pipelineDesc);

        // Create graphics pipeline for scene rendering
        {
            pipelineDesc.blend.targets[0].blendEnabled  = true;
            pipelineDesc.blend.targets[0].colorMask     = 0xF;
        }
        scenePipeline = renderer->CreatePipelineState(pipelineDesc);
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

        // Create query to measure GPU timing
        {
            queryDesc.type              = LLGL::QueryType::TimeElapsed;
            queryDesc.renderCondition   = false;
        }
        timerQuery = renderer->CreateQueryHeap(queryDesc);
    }

    std::uint64_t GetAndSyncQueryResult(LLGL::QueryHeap* query)
    {
        #ifdef __APPLE__
        return 0; //TODO: QueryResult not implemented on macOS/iOS
        #endif

        // Wait until query result is available and return result
        std::uint64_t result = 0;

        if (query->GetType() == LLGL::QueryType::PipelineStatistics)
        {
            LLGL::QueryPipelineStatistics statistics;
            while (!commandQueue->QueryResult(*query, 0, 1, &statistics, sizeof(statistics)))
            {
                // Wait and return control to other threads
                std::this_thread::yield();
            }
            result = statistics.inputAssemblyPrimitives;
        }
        else
        {
            while (!commandQueue->QueryResult(*query, 0, 1, &result, sizeof(result)))
            {
                // Wait and return control to other threads
                std::this_thread::yield();
            }
        }

        return result;
    }

    void PrintQueryResults()
    {
        #ifdef __APPLE__
        return; //TODO: QueryResult not implemented on macOS/iOS
        #endif

        // Query pipeline statistics results
        LLGL::QueryPipelineStatistics stats;
        while (!commandQueue->QueryResult(*geometryQuery, 0, 1, &stats, sizeof(stats)))
        {
            // Wait and return control to other threads
            std::this_thread::yield();
        }

        // Query timing results
        std::uint64_t elapsedTime = 0;
        while (!commandQueue->QueryResult(*timerQuery, 0, 1, &elapsedTime, sizeof(elapsedTime)))
        {
            // Wait and return control to other threads
            std::this_thread::yield();
        }

        // Print result
        LLGL::Log::Printf(
            "input assembly: %u, vertex invocations: %u, fragment invocations: %u, timing: %f ms        \r",
            static_cast<unsigned>(stats.inputAssemblyPrimitives),
            static_cast<unsigned>(stats.vertexShaderInvocations),
            static_cast<unsigned>(stats.fragmentShaderInvocations),
            static_cast<double>(elapsedTime)/1000000.0
        );
        ::fflush(stdout);
    }

    void SetBoxTransformAndColor(const Gs::Matrix4f& matrix, const LLGL::ColorRGBAf& color)
    {
        settings.wvpMatrix  = projection;
        settings.wvpMatrix *= matrix;
        settings.wMatrix    = matrix;
        settings.color      = color;
        commands->UpdateBuffer(*constantBuffer, 0, &settings, sizeof(settings));
    }

private:

    void DrawModel(const Model& mdl)
    {
        commands->Draw(mdl.numVertices, mdl.firstVertex);
    }

    void UpdateScene()
    {
        // Update matrices in constant buffer
        static float anim0, anim1;

        if (input.KeyDown(LLGL::Key::Space))
            animEnabled = !animEnabled;

        if (animEnabled)
            anim0 += 0.01f;

        anim1 += 0.01f;

        modelTransform[0].LoadIdentity();
        Gs::RotateFree(modelTransform[0], { 0, 1, 0 }, Gs::Deg2Rad(std::sin(anim0)*15.0f));
        Gs::Translate(modelTransform[0], { 0, 0, 5 });
        Gs::RotateFree(modelTransform[0], { 0, 1, 0 }, anim0*3);

        modelTransform[1].LoadIdentity();
        Gs::Translate(modelTransform[1], { 0, 0, 10 });
        Gs::RotateFree(modelTransform[1], { 0, 1, 0 }, anim1*-1.5f);
    }

    void RenderBoundingBoxes()
    {
        // Clear depth buffer
        commands->Clear(LLGL::ClearFlags::Depth);

        // Set resources
        commands->SetPipelineState(*occlusionPipeline);
        commands->SetResource(0, *constantBuffer);

        // Draw occluder box
        SetBoxTransformAndColor(modelTransform[0], { 1, 1, 1 });
        DrawModel(model0);

        // Draw box for occlusion query
        SetBoxTransformAndColor(modelTransform[1], { 1, 1, 1 });
        commands->BeginQuery(*occlusionQuery);
        {
            DrawModel(model0);
        }
        commands->EndQuery(*occlusionQuery);
    }

    void RenderScene()
    {
        // Clear color and depth buffers
        commands->Clear(LLGL::ClearFlags::ColorDepth, backgroundColor);

        // Set resources
        commands->SetPipelineState(*scenePipeline);

        // Draw scene
        SetBoxTransformAndColor(modelTransform[1], { 0, 1, 0 });
        commands->BeginRenderCondition(*occlusionQuery);
        {
            DrawModel(model0);
        }
        commands->EndRenderCondition();

        // Draw scene
        SetBoxTransformAndColor(modelTransform[0], { 1, 1, 1, 0.5f });
        DrawModel(model0);
    }

    void OnDrawFrame() override
    {
        UpdateScene();

        commands->Begin();
        {
            // Measure GPU performance
            commands->BeginQuery(*timerQuery);
            {
                // Set buffers
                commands->SetVertexBuffer(*vertexBuffer);

                // Start with geometry query
                commands->BeginQuery(*geometryQuery);
                {
                    commands->SetViewport(swapChain->GetResolution());

                    commands->BeginRenderPass(*swapChain);
                    {
                        RenderBoundingBoxes();
                        RenderScene();
                    }
                    commands->EndRenderPass();
                }
                commands->EndQuery(*geometryQuery);
            }
            commands->EndQuery(*timerQuery);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Print query results every couple of milliseconds
        auto currentTime = Clock::now();
        if (std::chrono::duration_cast<Ticks>(currentTime - prevPrintTime).count() >= printRefreshRate)
        {
            prevPrintTime = currentTime;
            PrintQueryResults();
        }
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Queries);



