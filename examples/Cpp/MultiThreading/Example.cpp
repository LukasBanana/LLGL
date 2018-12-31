/*
 * Example.cpp (Example_MultiThreading)
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <ExampleBase.h>
#include <thread>
#include <mutex>


class Example_MultiThreading : public ExampleBase
{

    LLGL::ShaderProgram*        shaderProgram       = nullptr;
    LLGL::Buffer*               vertexBuffer        = nullptr;
    LLGL::Buffer*               indexBuffer         = nullptr;
    LLGL::PipelineLayout*       pipelineLayout      = nullptr;
    LLGL::CommandBuffer*        primaryCmdBuffer    = nullptr;
    
    std::uint32_t               numIndices          = 0;
    std::mutex                  logMutex;
    
    struct Bundle
    {
        LLGL::GraphicsPipeline* pipeline            = nullptr;
        LLGL::Buffer*           constantBuffer      = nullptr;
        LLGL::ResourceHeap*     resourceHeap        = nullptr;
        LLGL::CommandBuffer*    secondaryCmdBuffer  = nullptr;
        Gs::Matrix4f            wvpMatrix;
    }
    bundle[2];

public:

    Example_MultiThreading() :
        ExampleBase { L"LLGL Example: MultiThreading" }
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
        pipelineLayout = renderer->CreatePipelineLayout(LLGL::PipelineLayoutDesc("cbuffer(0):vert"));

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
            pipelineDesc.shaderProgram              = shaderProgram;
            pipelineDesc.pipelineLayout             = pipelineLayout;
            //pipelineDesc.rasterizer.multiSampling   = LLGL::MultiSamplingDescriptor(8);

            // Enable depth test and writing
            pipelineDesc.depth.testEnabled          = true;
            pipelineDesc.depth.writeEnabled         = true;

            // Enable back-face culling
            pipelineDesc.rasterizer.cullMode        = LLGL::CullMode::Back;
        }

        // Create first graphics pipeline
        bundle[0].pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);

        // Create second graphics pipeline
        {
            auto& targetDesc = pipelineDesc.blend.targets[0];
            targetDesc.blendEnabled     = true;
            targetDesc.dstColor         = LLGL::BlendOp::One;
            targetDesc.srcColor         = LLGL::BlendOp::One;
            targetDesc.colorArithmetic  = LLGL::BlendArithmetic::Subtract;
        }
        bundle[1].pipeline = renderer->CreateGraphicsPipeline(pipelineDesc);
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
            cmdBuffer.SetGraphicsPipeline(*bundle.pipeline);
            cmdBuffer.SetGraphicsResourceHeap(*bundle.resourceHeap, 0);
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
                for (auto& bdl : bundle)
                    cmdBuffer.Execute(*bdl.secondaryCmdBuffer);
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
        
        // Encode primary command buffer
        EncodePrimaryCommandBuffer("mainThread");
        
        // Wait for worker threads to finish
        for (auto& worker : workerThread)
        {
            if (worker.joinable())
                worker.join();
        }
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
        rotation += 0.001f;
        
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
        commandQueue->Submit(*primaryCmdBuffer);
        context->Present();
    }

    void OnDrawFrame() override
    {
        UpdateScene();
        DrawScene();
    }

};

LLGL_IMPLEMENT_EXAMPLE(Example_MultiThreading);



