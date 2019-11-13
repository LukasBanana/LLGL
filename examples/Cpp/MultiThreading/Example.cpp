/*
 * Example.cpp (Example_MultiThreading)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <iomanip>


// Enables/disables the use of two secondary command buffers
//#define ENABLE_SECONDARY_COMMAND_BUFFERS

class Measure
{

        using Clock     = std::chrono::system_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Ticks     = std::chrono::milliseconds;

    public:

        // Interval (in milliseconds) to the next measurement result.
        Measure(std::uint64_t interval = 1000, const std::string& title = "Average Time") :
            timer_    { LLGL::Timer::Create() },
            interval_ { interval              },
            title_    { title                 }
        {
        }

        void Start()
        {
            // Start timer
            timer_->Start();
        }

        void Stop()
        {
            // Take sample
            elapsed_ += timer_->Stop();
            ++samples_;

            // Check if average elapsed time can be printed again
            auto end = Clock::now();
            auto diff = std::chrono::duration_cast<Ticks>(end - intervalStartTime_);

            if (diff.count() >= static_cast<long long>(interval_))
            {
                Print();
                intervalStartTime_ = Clock::now();
            }
        }

    private:

        void Print()
        {
            if (samples_ > 0)
            {
                auto averageTime = static_cast<double>(elapsed_);
                averageTime /= static_cast<double>(timer_->GetFrequency());
                averageTime *= 1000000.0;
                averageTime /= static_cast<double>(samples_);

                std::cout << title_ << ": ";
                std::cout << std::fixed << std::setprecision(6) << averageTime << " microseconds";
                std::cout << "         \r";
                std::flush(std::cout);

                samples_ = 0;
                elapsed_ = 0;
            }
        }

    private:

        std::unique_ptr<LLGL::Timer>    timer_;
        std::uint64_t                   interval_           = 0;
        TimePoint                       intervalStartTime_;
        std::uint64_t                   samples_            = 0;
        std::uint64_t                   elapsed_            = 0;
        std::string                     title_;

};

class Example_MultiThreading : public ExampleBase
{

    LLGL::ShaderProgram*        shaderProgram       = nullptr;
    LLGL::Buffer*               vertexBuffer        = nullptr;
    LLGL::Buffer*               indexBuffer         = nullptr;
    LLGL::PipelineLayout*       pipelineLayout      = nullptr;
    LLGL::CommandBuffer*        primaryCmdBuffer    = nullptr;

    std::uint32_t               numIndices          = 0;
    std::mutex                  logMutex;

    Measure                     measure;

    struct Bundle
    {
        LLGL::PipelineState*    pipeline            = nullptr;
        LLGL::Buffer*           constantBuffer      = nullptr;
        LLGL::ResourceHeap*     resourceHeap        = nullptr;
        LLGL::CommandBuffer*    secondaryCmdBuffer  = nullptr;
        Gs::Matrix4f            wvpMatrix;
    }
    bundle[2];

public:

    Example_MultiThreading() :
        ExampleBase { L"LLGL Example: MultiThreading", { 800, 600 }, 8, true, false }
    {
        auto vertexFormat = CreateBuffers();
        LoadShaders(vertexFormat);
        CreatePipelines();
        CreateCommandBuffers();
    }

private:

    LLGL::VertexFormat CreateBuffers()
    {
        // Specify vertex format
        LLGL::VertexFormat vertexFormat;
        vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });

        // Generate data for mesh buffers
        auto indices = GenerateTexturedCubeTriangleIndices();
        auto vertices = GenerateTexturedCubeVertices();
        numIndices = static_cast<std::uint32_t>(indices.size());

        // Create buffers for a simple 3D cube model
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        indexBuffer = CreateIndexBuffer(indices, LLGL::Format::R32UInt);

        for (auto& bdl : bundle)
            bdl.constantBuffer = CreateConstantBuffer(bdl.wvpMatrix);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        shaderProgram = LoadStandardShaderProgram({ vertexFormat });
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(Scene@1):vert"));

        // Create resource view heap
        for (auto& bdl : bundle)
        {
            LLGL::ResourceHeapDescriptor resourceHeapDesc;
            {
                resourceHeapDesc.pipelineLayout = pipelineLayout;
                resourceHeapDesc.resourceViews  = { bdl.constantBuffer };
            }
            bdl.resourceHeap = renderer->CreateResourceHeap(resourceHeapDesc);
        }

        // Setup graphics pipeline descriptors
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            // Set references to shader program, and pipeline layout
            pipelineDesc.shaderProgram                  = shaderProgram;
            pipelineDesc.pipelineLayout                 = pipelineLayout;
            pipelineDesc.rasterizer.multiSampleEnabled  = (GetSampleCount() > 1);

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled              = true;
            pipelineDesc.depth.writeEnabled             = true;

            // Enable back-face culling
            pipelineDesc.rasterizer.cullMode            = LLGL::CullMode::Back;
        }

        // Create first graphics pipeline
        bundle[0].pipeline = renderer->CreatePipelineState(pipelineDesc);

        // Create second graphics pipeline
        {
            auto& targetDesc = pipelineDesc.blend.targets[0];
            targetDesc.blendEnabled     = true;
            targetDesc.dstColor         = LLGL::BlendOp::One;
            targetDesc.srcColor         = LLGL::BlendOp::One;
            targetDesc.colorArithmetic  = LLGL::BlendArithmetic::Subtract;
        }
        bundle[1].pipeline = renderer->CreatePipelineState(pipelineDesc);
    }

    static void PrintThreadsafe(std::mutex& mtx, const std::string& text)
    {
        std::lock_guard<std::mutex> guard { mtx };
        std::cout << text << std::endl;
    }

    static void EncodeSecondaryCommandBuffer(
        Bundle&         bundle,
        std::uint32_t   numIndices,
        std::mutex&     mtx,
        std::string     threadName)
    {
        // Print thread start
        PrintThreadsafe(mtx, "Enter thread: " + threadName);

        // Encode command buffer
        auto& cmdBuffer = *bundle.secondaryCmdBuffer;

        cmdBuffer.Begin();
        {
            cmdBuffer.SetPipelineState(*bundle.pipeline);
            cmdBuffer.SetResourceHeap(*bundle.resourceHeap);
            cmdBuffer.DrawIndexed(numIndices, 0);
        }
        cmdBuffer.End();

        // Print thread end
        PrintThreadsafe(mtx, "Leave thread: " + threadName);
    }

    void EncodePrimaryCommandBuffer(const std::string& threadName)
    {
        // Print thread start
        PrintThreadsafe(logMutex, "Enter thread: " + threadName);

        // Encode command buffer
        auto& cmdBuffer = *primaryCmdBuffer;
        cmdBuffer.Begin();
        {
            // Initialize clear color
            cmdBuffer.SetClearColor(backgroundColor);

            // Set hardware buffers to draw the model
            cmdBuffer.SetVertexBuffer(*vertexBuffer);
            cmdBuffer.SetIndexBuffer(*indexBuffer);

            // Set the render context as the initial render target
            cmdBuffer.BeginRenderPass(*context);
            {
                // Clear color- and depth buffers, and set viewport
                cmdBuffer.Clear(LLGL::ClearFlags::ColorDepth);
                cmdBuffer.SetViewport(context->GetVideoMode().resolution);

                // Draw scene with secondary command buffers
                #ifdef ENABLE_SECONDARY_COMMAND_BUFFERS

                for (auto& bdl : bundle)
                    cmdBuffer.Execute(*bdl.secondaryCmdBuffer);

                #else // ENABLE_SECONDARY_COMMAND_BUFFERS

                for (auto& bdl : bundle)
                {
                    cmdBuffer.SetPipelineState(*bdl.pipeline);
                    cmdBuffer.SetResourceHeap(*bdl.resourceHeap);
                    cmdBuffer.DrawIndexed(numIndices, 0);
                }

                #endif // /ENABLE_SECONDARY_COMMAND_BUFFERS
            }
            cmdBuffer.EndRenderPass();
        }
        cmdBuffer.End();

        // Print thread end
        PrintThreadsafe(logMutex, "Leave thread: " + threadName);
    }

    void CreateCommandBuffers()
    {
        // Create primary command buffer
        LLGL::CommandBufferDescriptor cmdBufferDesc;
        {
            cmdBufferDesc.flags = LLGL::CommandBufferFlags::MultiSubmit;
        }
        primaryCmdBuffer = renderer->CreateCommandBuffer(cmdBufferDesc);

        #ifdef ENABLE_SECONDARY_COMMAND_BUFFERS

        // Create secondary command buffers
        cmdBufferDesc.flags = (LLGL::CommandBufferFlags::DeferredSubmit | LLGL::CommandBufferFlags::MultiSubmit);

        // Start encoding secondary command buffers in parallel
        std::thread workerThread[2];

        for (int i = 0; i < 2; ++i)
        {
            // Create secondary command buffer
            bundle[i].secondaryCmdBuffer = renderer->CreateCommandBuffer(cmdBufferDesc);

            // Start worker thread to encode secondary command buffer
            workerThread[i] = std::thread(
                Example_MultiThreading::EncodeSecondaryCommandBuffer,
                std::ref(bundle[i]),
                numIndices,
                std::ref(logMutex),
                "workerThread[" + std::to_string(i) + "]"
            );
        }

        #endif // /ENABLE_SECONDARY_COMMAND_BUFFERS

        // Encode primary command buffer
        EncodePrimaryCommandBuffer("mainThread");

        #ifdef ENABLE_SECONDARY_COMMAND_BUFFERS

        // Wait for worker threads to finish
        for (auto& worker : workerThread)
        {
            if (worker.joinable())
                worker.join();
        }

        #endif // /ENABLE_SECONDARY_COMMAND_BUFFERS
    }

    void Transform(Gs::Matrix4f& matrix, const Gs::Vector3f& pos, float angle)
    {
        matrix.LoadIdentity();
        Gs::Translate(matrix, pos);
        Gs::RotateFree(matrix, Gs::Vector3f(1, 1, 1).Normalized(), angle);
        matrix = projection * matrix;
    }

    void UpdateScene()
    {
        // Animate rotation
        static float rotation;
        rotation += 0.01f;

        // Update scene matrices
        Transform(bundle[0].wvpMatrix, { -1, 0, 8 }, -rotation);
        Transform(bundle[1].wvpMatrix, { +1, 0, 8 }, +rotation);

        // Update constant buffer
        for (auto& bdl : bundle)
        {
            renderer->WriteBuffer(
                *bdl.constantBuffer,
                0,
                &(bdl.wvpMatrix),
                sizeof(Gs::Matrix4f)
            );
        }
    }

    void DrawScene()
    {
        // Submit primary command buffer and present result
        measure.Start();
        commandQueue->Submit(*primaryCmdBuffer);
        measure.Stop();
        context->Present();
    }

    void OnDrawFrame() override
    {
        UpdateScene();
        DrawScene();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_MultiThreading);



