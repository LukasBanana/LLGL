/*
 * Example.cpp (Example_MultiThreading)
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <ExampleBase.h>
#include <LLGL/Utils/ForRange.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <stdio.h>


// Enables/disables the use of two secondary command buffers
#define ENABLE_SECONDARY_COMMAND_BUFFERS 1

class Measure
{

        using Clock     = std::chrono::system_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Ticks     = std::chrono::milliseconds;

    public:

        // Interval (in milliseconds) to the next measurement result.
        Measure(std::uint64_t interval = 1000, const std::string& title = "Average Time") :
            interval_ { interval },
            title_    { title    }
        {
        }

        void Start()
        {
            // Start timer
            timer_.Start();
        }

        void Stop()
        {
            // Take sample
            elapsed_ += timer_.Stop();
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
                double averageTime = static_cast<double>(elapsed_);
                averageTime /= static_cast<double>(timer_.GetFrequency());
                averageTime *= 1000000.0;
                averageTime /= static_cast<double>(samples_);

                printf("%s: %.6f  microseconds         \r", title_.c_str(), averageTime);
                fflush(stdout);

                samples_ = 0;
                elapsed_ = 0;
            }
        }

    private:

        Stopwatch       timer_;
        std::uint64_t   interval_           = 0;
        TimePoint       intervalStartTime_;
        std::uint64_t   samples_            = 0;
        std::uint64_t   elapsed_            = 0;
        std::string     title_;

};

constexpr std::uint32_t maxNumSwapBuffers = 3;

class Example_MultiThreading : public ExampleBase
{

    ShaderPipeline              shaderPipeline;
    LLGL::Buffer*               vertexBuffer                        = nullptr;
    LLGL::Buffer*               indexBuffer                         = nullptr;
    LLGL::PipelineLayout*       pipelineLayout                      = nullptr;
    LLGL::CommandBuffer*        primaryCmdBuffer[maxNumSwapBuffers] = {};

    std::uint32_t               numIndices                          = 0;
    std::mutex                  logMutex;

    Measure                     measure;

    struct Bundle
    {
        LLGL::PipelineState*    pipeline                            = nullptr;
        LLGL::Buffer*           constantBuffer                      = nullptr;
        LLGL::ResourceHeap*     resourceHeap                        = nullptr;
        LLGL::CommandBuffer*    secondaryCmdBuffer                  = nullptr;

        struct Matrices
        {
            Gs::Matrix4f        wvpMatrix;
            Gs::Matrix4f        wMatrix;
        }
        matrices;
    }
    bundle[2];

public:

    Example_MultiThreading() :
        ExampleBase { "LLGL Example: MultiThreading" }
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
        vertexFormat.AppendAttribute({ "normal",   LLGL::Format::RGB32Float });
        vertexFormat.AppendAttribute({ "texCoord", LLGL::Format::RG32Float  });

        // Generate data for mesh buffers
        auto indices = GenerateTexturedCubeTriangleIndices();
        auto vertices = GenerateTexturedCubeVertices();
        numIndices = static_cast<std::uint32_t>(indices.size());

        // Create buffers for a simple 3D cube model
        vertexBuffer = CreateVertexBuffer(vertices, vertexFormat);
        indexBuffer = CreateIndexBuffer(indices, LLGL::Format::R32UInt);

        for (auto& bdl : bundle)
            bdl.constantBuffer = CreateConstantBuffer(bdl.matrices);

        return vertexFormat;
    }

    void LoadShaders(const LLGL::VertexFormat& vertexFormat)
    {
        // Load shader program
        shaderPipeline = LoadStandardShaderPipeline({ vertexFormat });
    }

    void CreatePipelines()
    {
        // Create pipeline layout
        pipelineLayout = renderer->CreatePipelineLayout(LLGL::Parse("heap{cbuffer(Scene@1):vert}"));

        // Create resource view heap
        for (auto& bdl : bundle)
            bdl.resourceHeap = renderer->CreateResourceHeap(pipelineLayout, { bdl.constantBuffer });

        // Setup graphics pipeline descriptors
        LLGL::GraphicsPipelineDescriptor pipelineDesc;
        {
            // Set references to shader program, and pipeline layout
            pipelineDesc.vertexShader                   = shaderPipeline.vs;
            pipelineDesc.fragmentShader                 = shaderPipeline.ps;
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
        printf("%s\n", text.c_str());
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

    void EncodePrimaryCommandBuffer(LLGL::CommandBuffer& cmdBuffer, std::uint32_t swapBufferIndex, const std::string& threadName = "")
    {
        // Print thread start
        if (!threadName.empty())
            PrintThreadsafe(logMutex, "Enter thread: " + threadName);

        // Encode command buffer
        cmdBuffer.Begin();
        {
            // Set hardware buffers to draw the model
            cmdBuffer.SetVertexBuffer(*vertexBuffer);
            cmdBuffer.SetIndexBuffer(*indexBuffer);

            // Set the swap-chain as the initial render target
            cmdBuffer.BeginRenderPass(*swapChain, nullptr, 0, nullptr, swapBufferIndex);
            {
                // Clear color- and depth buffers, and set viewport
                cmdBuffer.Clear(LLGL::ClearFlags::ColorDepth, { backgroundColor });
                cmdBuffer.SetViewport(swapChain->GetResolution());

                // Draw scene with secondary command buffers
                #if ENABLE_SECONDARY_COMMAND_BUFFERS

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
        if (!threadName.empty())
            PrintThreadsafe(logMutex, "Leave thread: " + threadName);
    }

    void CreateCommandBuffers()
    {
        // Create primary command buffer
        LLGL::CommandBufferDescriptor cmdBufferDesc;
        {
            cmdBufferDesc.flags = LLGL::CommandBufferFlags::MultiSubmit;
        }
        for_range(i, swapChain->GetNumSwapBuffers())
            primaryCmdBuffer[i] = renderer->CreateCommandBuffer(cmdBufferDesc);

        #if ENABLE_SECONDARY_COMMAND_BUFFERS

        // Create secondary command buffers
        cmdBufferDesc.flags = (LLGL::CommandBufferFlags::Secondary | LLGL::CommandBufferFlags::MultiSubmit);

        // Start encoding secondary command buffers in parallel
        std::thread workerThread[2];

        for_range(i, 2)
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

        // Secondary command buffers must have finished encoding before we can use them in a primary command buffer
        for (auto& worker : workerThread)
        {
            if (worker.joinable())
                worker.join();
        }

        #endif // /ENABLE_SECONDARY_COMMAND_BUFFERS

        // Encode primary command buffer
        for_range(i, swapChain->GetNumSwapBuffers())
            EncodePrimaryCommandBuffer(*primaryCmdBuffer[i], i, "mainThread");
    }

    void Transform(Bundle::Matrices& matrices, const Gs::Vector3f& pos, const Gs::Vector3f& axis, float angle)
    {
        matrices.wMatrix.LoadIdentity();
        Gs::Translate(matrices.wMatrix, pos);
        Gs::RotateFree(matrices.wMatrix, axis.Normalized(), angle);
        matrices.wvpMatrix = projection * matrices.wMatrix;
    }

    void UpdateCommandBuffers()
    {
        std::thread workerThread[2];

        for_range(i, 2)
        {
            // Start worker thread to encode secondary command buffer
            workerThread[i] = std::thread(
                Example_MultiThreading::EncodeSecondaryCommandBuffer,
                std::ref(bundle[i]),
                numIndices,
                std::ref(logMutex),
                "workerThread[" + std::to_string(i) + "]"
            );
        }

        // Wait for worker threads to finish
        for (auto& worker : workerThread)
        {
            if (worker.joinable())
                worker.join();
        }

        for_range(i, swapChain->GetNumSwapBuffers())
            EncodePrimaryCommandBuffer(*primaryCmdBuffer[i], i, "mainThread");
    }

    void UpdateScene()
    {
        if (input.KeyDown(LLGL::Key::Tab))
            UpdateCommandBuffers();

        // Animate rotation
        static float rotation;
        rotation += 0.01f;

        // Update scene matrices
        Transform(bundle[0].matrices, { -1, 0, 8 }, { +1, 1, 1 }, -rotation);
        Transform(bundle[1].matrices, { +1, 0, 8 }, { -1, 1, 1 }, +rotation);

        // Update constant buffer
        for (auto& bdl : bundle)
        {
            renderer->WriteBuffer(
                *bdl.constantBuffer,
                0,
                &(bdl.matrices),
                sizeof(bdl.matrices)
            );
        }
    }

    void DrawScene()
    {
        // Submit primary command buffer and present result
        measure.Start();
        commandQueue->Submit(*primaryCmdBuffer[swapChain->GetCurrentSwapIndex()]);
        measure.Stop();
    }

    void OnResize(const LLGL::Extent2D& /*resoluion*/) override
    {
        // Encode primary command buffer again to update resolution in constant buffer
        for_range(i, swapChain->GetNumSwapBuffers())
            EncodePrimaryCommandBuffer(*primaryCmdBuffer[i], i);
    }

    void OnDrawFrame() override
    {
        UpdateScene();
        DrawScene();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_MultiThreading);



