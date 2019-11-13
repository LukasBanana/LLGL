/*
 * Example.cpp (Example_Queries)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <chrono>


class Example_Queries : public ExampleBase
{

    LLGL::ShaderProgram*    shaderProgram           = nullptr;

    LLGL::PipelineState*    occlusionPipeline       = nullptr;
    LLGL::PipelineState*    scenePipeline           = nullptr;

    LLGL::PipelineLayout*   pipelineLayout          = nullptr;
    LLGL::ResourceHeap*     resourceHeap            = nullptr;

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
        ExampleBase { L"LLGL Example: Query" }
    {
        // Create all graphics objects
        auto vertexFormat = CreateBuffers();
        shaderProgram = LoadStandardShaderProgram({ vertexFormat });
        CreatePipelines();
        CreateQueries();
        CreateResourceHeaps();

        // Show info
        std::cout << "press SPACE KEY to enable/disable animation of occluder" << std::endl;
    }

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
        vertexFormat.SetStride(sizeof(TexturedVertex));

        // Load models
        auto vertices = LoadObjModel("../../Media/Models/Pyramid.obj");
        model0.numVertices = static_cast<std::uint32_t>(vertices.size());

        // Create vertex and constant buffer
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
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
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.pipelineLayout                 = pipelineLayout;

            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;

            pipelineDesc.blend.targets[0].colorMask     = LLGL::ColorRGBAb{ false };
        }
        occlusionPipeline = renderer->CreatePipelineState(pipelineDesc);

        // Create graphics pipeline for scene rendering
        {
            pipelineDesc.blend.targets[0].blendEnabled  = true;
            pipelineDesc.blend.targets[0].colorMask     = LLGL::ColorRGBAb{ true };
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
            result = statistics.inputAssemblyPrimitives;
        }
        else
        {
            while (!commandQueue->QueryResult(*query, 0, 1, &result, sizeof(result))) { /* wait */ }
        }

        return result;
    }

    void PrintQueryResults()
    {
        // Query pipeline statistics results
        LLGL::QueryPipelineStatistics stats;
        while (!commandQueue->QueryResult(*geometryQuery, 0, 1, &stats, sizeof(stats)))
        {
            /* wait */
        }

        // Query timing results
        std::uint64_t elapsedTime = 0;
        while (!commandQueue->QueryResult(*timerQuery, 0, 1, &elapsedTime, sizeof(elapsedTime)))
        {
            /* wait */
        }

        // Print result
        std::cout << "input assembly: " << stats.inputAssemblyPrimitives;
        std::cout << ", vertex invocations: " << stats.vertexShaderInvocations;
        std::cout << ", fragment invocations: " << stats.fragmentShaderInvocations;
        std::cout << ", timing: " << static_cast<double>(elapsedTime)/1000000.0 << " ms";
        std::cout << "        \r";
        std::flush(std::cout);
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

        if (input->KeyDown(LLGL::Key::Space))
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
        commands->SetResourceHeap(*resourceHeap);

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
        commands->Clear(LLGL::ClearFlags::ColorDepth);

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

                // Start with qeometry query
                commands->BeginQuery(*geometryQuery);
                {
                    commands->SetViewport(LLGL::Viewport{ { 0, 0 }, context->GetResolution() });

                    commands->BeginRenderPass(*context);
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

        // Present result on the screen
        context->Present();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_Queries);



