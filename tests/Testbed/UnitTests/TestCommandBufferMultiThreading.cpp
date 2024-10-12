/*
 * TestCommandBufferMultiThreading.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/Utility.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>
#include <thread>
#include <mutex>


class ThreadOrderInfo
{

    public:

        void Append(unsigned threadID)
        {
            std::lock_guard<std::mutex> guard{ threadIDListMutex_ };
            if (!threadIDList_.empty())
                threadIDList_ += ", ";
            threadIDList_ += std::to_string(threadID);
        }

        std::string Flush()
        {
            return std::move(threadIDList_);
        }

    private:

        std::string threadIDList_;
        std::mutex  threadIDListMutex_;

};

struct ThreadBundle
{
    CommandBuffer*  cmdBuffer;          // Unique
    RenderTarget*   renderTarget;       // Unique
    Buffer*         meshBuffer;         // Uniform
    Buffer*         sceneBuffer;        // Unique
    Texture*        colorMap;           // Uniform
    Sampler*        colorMapSampler;    // Uniform
};

DEF_TEST( CommandBufferMultiThreading )
{
    constexpr unsigned  numCmdBuffers   = 16;
    constexpr unsigned  numFrames       = 10;
    constexpr int       diffThreshold   = 30; // Diff threshold of 30, because sampling MIP-mapped textures is backend dependent

    if (shaders[VSTextured] == nullptr || shaders[PSTextured] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    static CommandBuffer*   cmdBuffers      [numCmdBuffers] = {};
    static Buffer*          sceneBuffers    [numCmdBuffers] = {};
    static RenderTarget*    renderTargets   [numCmdBuffers] = {};
    static Texture*         outputTextures  [numCmdBuffers] = {};
    static RenderPass*      renderPass;
    static PipelineState*   pso;
    static double           avgEncodingTime;
    static double           avgSubmissionTime;

    const Extent2D texSize{ 256, 256 };

    if (frame == 0)
    {
        // Reset time stats
        avgEncodingTime     = 0.0;
        avgSubmissionTime   = 0.0;

        // Create all command buffers
        for_range(i, numCmdBuffers)
        {
            cmdBuffers[i] = renderer->CreateCommandBuffer();
            sceneBuffers[i] = renderer->CreateBuffer(ConstantBufferDesc(sizeof(SceneConstants)));

            TextureDescriptor texDesc;
            {
                texDesc.extent.width    = texSize.width;
                texDesc.extent.height   = texSize.height;
                texDesc.mipLevels       = 1;
            }
            outputTextures[i] = renderer->CreateTexture(texDesc);

            RenderTargetDescriptor rtDesc;
            {
                rtDesc.resolution               = texSize;
                rtDesc.colorAttachments[0]      = outputTextures[i];
                rtDesc.depthStencilAttachment   = Format::D16UNorm;
            }
            renderTargets[i] = renderer->CreateRenderTarget(rtDesc);
        }

        // Create render pass that is compatible with all render targets
        RenderPassDescriptor rpDesc;
        {
            rpDesc.colorAttachments[0].format   = Format::RGBA8UNorm;
            rpDesc.colorAttachments[0].storeOp  = AttachmentStoreOp::Store;
            rpDesc.depthAttachment.format       = Format::D16UNorm;
        }
        renderPass = renderer->CreateRenderPass(rpDesc);

        // Create graphics PSO
        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout      = layouts[PipelineTextured];
            psoDesc.renderPass          = renderPass;
            psoDesc.vertexShader        = shaders[VSTextured];
            psoDesc.fragmentShader      = shaders[PSTextured];
            psoDesc.depth.testEnabled   = true;
            psoDesc.depth.writeEnabled  = true;
            psoDesc.rasterizer.cullMode = CullMode::Back;
        }
        CREATE_GRAPHICS_PSO_EXT(pso, psoDesc, "psoMultiThreading");
    }

    // Encode command buffers in parallel
    ThreadOrderInfo threadEnterOrder;
    ThreadOrderInfo threadExitOrder;

    auto CommandBufferRecordingWorker = [this, &threadEnterOrder, &threadExitOrder](
        unsigned threadID, const ThreadBundle& bundle, const IndexedTriangleMesh& mesh, Gs::Vector3f origin, float rotation)
    {
        threadEnterOrder.Append(threadID);

        // Initialie scene constants
        SceneConstants localSceneConstants;

        Gs::Matrix4f vMatrix;
        vMatrix.LoadIdentity();
        Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });

        vMatrix.MakeInverse();

        LoadProjectionMatrix(localSceneConstants.vpMatrix);
        localSceneConstants.vpMatrix *= vMatrix;

        localSceneConstants.wMatrix.LoadIdentity();
        Gs::Translate(localSceneConstants.wMatrix, origin);
        Gs::RotateFree(localSceneConstants.wMatrix, Gs::Vector3f{ 1 }.Normalized(), Gs::Deg2Rad(rotation));
        Gs::Scale(localSceneConstants.wMatrix, Gs::Vector3f{ 0.5f });

        // Record command buffer
        bundle.cmdBuffer->Begin();
        {
            bundle.cmdBuffer->SetVertexBuffer(*bundle.meshBuffer);
            bundle.cmdBuffer->SetIndexBuffer(*bundle.meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
            bundle.cmdBuffer->UpdateBuffer(*bundle.sceneBuffer, 0, &localSceneConstants, sizeof(localSceneConstants));
            bundle.cmdBuffer->BeginRenderPass(*bundle.renderTarget);
            {
                bundle.cmdBuffer->Clear(ClearFlags::ColorDepth);
                bundle.cmdBuffer->SetViewport(bundle.renderTarget->GetResolution());
                bundle.cmdBuffer->SetPipelineState(*pso);
                bundle.cmdBuffer->SetResource(0, *bundle.sceneBuffer);
                bundle.cmdBuffer->SetResource(1, *bundle.colorMap);
                bundle.cmdBuffer->SetResource(2, *bundle.colorMapSampler);
                bundle.cmdBuffer->DrawIndexed(mesh.numIndices, 0);
            }
            bundle.cmdBuffer->EndRenderPass();
        }
        bundle.cmdBuffer->End();

        threadExitOrder.Append(threadID);
    };

    // Launch worker threads
    std::thread workers[numCmdBuffers];
    ThreadBundle bundles[numCmdBuffers];

    const std::uint64_t startEncodingTime = Timer::Tick();

    for_range(i, numCmdBuffers)
    {
        // Initialize thread bundle
        ThreadBundle& bundle = bundles[i];
        {
            bundle.cmdBuffer    = cmdBuffers[i];
            bundle.renderTarget = renderTargets[i];
            bundle.meshBuffer   = meshBuffer;
            bundle.sceneBuffer  = sceneBuffers[i];
        }

        bundle.colorMap = textures[i % 2];
        if (bundle.colorMap == nullptr)
        {
            Log::Errorf("Missing texture for command buffer [%u]\n", i);
            return TestResult::FailedErrors;
        }

        bundle.colorMapSampler = samplers[SamplerLinearClamp];
        if (bundle.colorMapSampler == nullptr)
        {
            Log::Errorf("Missing sampler state for command buffer [%u]\n", i);
            return TestResult::FailedErrors;
        }

        // Generate origin of 3D model for worker
        const Gs::Vector3f origin{ 0.0f, 0.0f, 0.0f };
        float t = static_cast<float>(frame) * 0.025f;
        float rotation = t * 360.0f * static_cast<float>(i) / static_cast<float>(numCmdBuffers - 1);

        // Launch worker thread with bundle
        workers[i] = std::thread(CommandBufferRecordingWorker, static_cast<unsigned>(i), std::ref(bundle), std::ref(models[ModelCube]), origin, rotation);
    }

    // Wait until all workers have finished
    for_range(i, numCmdBuffers)
        workers[i].join();

    const std::uint64_t endEncodingTime = Timer::Tick();

    // Submit all encoded command buffers
    const std::uint64_t startSubmissionTime = Timer::Tick();

    for_range(i, numCmdBuffers)
        cmdQueue->Submit(*cmdBuffers[i]);

    // Wait until GPU is idle or we can't get a representative timing
    cmdQueue->WaitIdle();

    const std::uint64_t endSubmissionTime = Timer::Tick();

    // Track average time
    const double freq           = static_cast<double>(Timer::Frequency());
    const double encodingTime   = (static_cast<double>(endEncodingTime - startEncodingTime) / freq) * 1000.0;
    const double submissionTime = (static_cast<double>(endSubmissionTime - startSubmissionTime) / freq) * 1000.0;
    avgEncodingTime += encodingTime;
    avgSubmissionTime += submissionTime;

    // Print threading order info
    if (opt.showTiming)
    {
        const std::string frameNo = (frame < 10 ? "Frame  " : "Frame ") + std::to_string(frame);
        std::string enterOrder = threadEnterOrder.Flush();
        Log::Printf("Thread enter order: [%s] %s (Encoding:   %.4f ms)\n", frameNo.c_str(), enterOrder.c_str(), encodingTime);
        std::string exitOrder = threadExitOrder.Flush();
        Log::Printf("Thread exit order:  [%s] %s (Submission: %.4f ms)\n", frameNo.c_str(), exitOrder.c_str(), submissionTime);
    }

    if (frame < numFrames)
        return TestResult::Continue;

    avgEncodingTime /= numFrames;
    avgSubmissionTime /= numFrames;

    if (opt.showTiming)
        Log::Printf("Average timing: Encoding ( %.4f ms ), Submission ( %.4f ms )\n", avgEncodingTime, avgSubmissionTime);

    // Read result from render target textures
    std::vector<ColorRGBub> outputImage;
    outputImage.resize(texSize.width * texSize.height);

    MutableImageView dstImageView;
    {
        dstImageView.format     = ImageFormat::RGB;
        dstImageView.dataType   = DataType::UInt8;
        dstImageView.data       = outputImage.data();
        dstImageView.dataSize   = outputImage.size() * sizeof(ColorRGBub);
    }

    const TextureRegion texRegion{ Offset3D{}, Extent3D{ texSize.width, texSize.height, 1 } };

    TestResult result = TestResult::Passed;

    for_range(i, numCmdBuffers)
    {
        if (opt.fastTest && i % 2 == 1)
            continue;

        renderer->ReadTexture(*outputTextures[i], texRegion, dstImageView);

        const std::string outputImageName = "MultiThreading_Worker" + std::to_string(i);
        SaveColorImage(outputImage, texSize, outputImageName);

        const DiffResult diff = DiffImages(outputImageName, diffThreshold);

        TestResult intermediateResult = diff.Evaluate(outputImageName.c_str());
        if (intermediateResult != TestResult::Passed)
        {
            result = intermediateResult;
            if (!opt.greedy)
                break;
        }
    }

    // Release resources
    for_range(i, numCmdBuffers)
    {
        renderer->Release(*cmdBuffers[i]);
        renderer->Release(*sceneBuffers[i]);
        renderer->Release(*renderTargets[i]);
        renderer->Release(*outputTextures[i]);
    }

    renderer->Release(*pso);
    renderer->Release(*renderPass);

    return result;
}


