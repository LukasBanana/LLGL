/*
 * Test_Compute.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/LLGL.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Trap.h>
#include <Gauss/Gauss.h>
#include <vector>

// Fill an array list of 4D-vectors for testing purposes
static std::vector<Gs::Vector4f> GetTestVector(std::size_t size)
{
    std::vector<Gs::Vector4f> vec(size);

    for (unsigned int i = 0; i < size; ++i)
    {
        auto x = static_cast<float>(i + 1);
        vec[i] = Gs::Vector4f(1, x, 1.0f / x, 0.1f * x);
    }

    return vec;
}

int main()
{
    try
    {
        LLGL::Log::RegisterCallbackStd();

        // Setup profiler and debugger
        auto debugger = std::make_shared<LLGL::RenderingDebugger>();

        // Load render system module
        LLGL::RenderSystemDescriptor rendererDesc = "OpenGL";
        {
            rendererDesc.debugger = debugger.get();
        }
        auto renderer = LLGL::RenderSystem::Load(rendererDesc);

        // Create swap-chain
        LLGL::SwapChainDescriptor swapChainDesc;
        swapChainDesc.resolution = { 800, 600 };

        /*auto swapChain = */renderer->CreateSwapChain(swapChainDesc);

        // Create command buffer
        auto commandQueue = renderer->GetCommandQueue();
        auto commands = renderer->CreateCommandBuffer();

        // Quit if compute shaders are not supported
        const auto& renderCaps = renderer->GetRenderingCaps();
        LLGL_VERIFY(renderCaps.features.hasComputeShaders);

        // Create storage buffer
        static const unsigned int vecSize = 128;
        auto vec = GetTestVector(vecSize);

        LLGL::BufferDescriptor storageBufferDesc;
        {
            storageBufferDesc.size              = sizeof(Gs::Vector4f)*vecSize;
            storageBufferDesc.bindFlags         = LLGL::BindFlags::Storage;
            storageBufferDesc.cpuAccessFlags    = LLGL::CPUAccessFlags::Read;
            storageBufferDesc.miscFlags         = LLGL::MiscFlags::DynamicUsage;
        }
        auto storageBuffer = renderer->CreateBuffer(storageBufferDesc, vec.data());

        // Load shader
        auto computeShader = renderer->CreateShader({ LLGL::ShaderType::Compute, "Shaders/ComputeShader.glsl" });

        // Create timer query
        LLGL::QueryHeapDescriptor queryDesc;
        {
            queryDesc.type = LLGL::QueryType::TimeElapsed;
        }
        auto timerQuery = renderer->CreateQueryHeap(queryDesc);

        // Create pipeline layout
        auto pipelineLayout = renderer->CreatePipelineLayout(LLGL::Parse("rwbuffer(OutputBuffer@0):comp"));

        // Create compute pipeline
        LLGL::ComputePipelineDescriptor pipelineDesc;
        {
            pipelineDesc.pipelineLayout = pipelineLayout;
            pipelineDesc.computeShader  = computeShader;
        }
        auto pipeline = renderer->CreatePipelineState(pipelineDesc);

        if (auto report = pipeline->GetReport())
        {
            if (report->HasErrors())
                LLGL_THROW_RUNTIME_ERROR(report->GetText());
        }

        // Set resources
        commands->Begin();
        {
            commands->SetResource(0, *storageBuffer);
            commands->SetPipelineState(*pipeline);

            // Dispatch compute shader (with 1*1*1 work groups only) and measure elapsed time with timer query
            commands->BeginQuery(*timerQuery);
            {
                commands->Dispatch(1, 1, 1);
            }
            commands->EndQuery(*timerQuery);
        }
        commands->End();
        commandQueue->Submit(*commands);

        // Show elapsed time from timer query
        std::uint64_t result = 0;
        while (!commandQueue->QueryResult(*timerQuery, 0, 1, &result, sizeof(result)))
        {
            /* wait until the result is available */
        }
        LLGL::Log::Printf("compute shader duration: %f ms\n", static_cast<double>(result) / 1000000);

        // Wait until the GPU has completed all work, to be sure we can evaluate the storage buffer
        renderer->GetCommandQueue()->WaitIdle();

        // Evaluate compute shader
        if (auto mappedBuffer = renderer->MapBuffer(*storageBuffer, LLGL::CPUAccess::ReadOnly))
        {
            // Show result
            auto* vecBuffer = reinterpret_cast<const Gs::Vector4f*>(mappedBuffer);
            LLGL::Log::Printf(
                "compute shader output: average vector = ( %f | %f | %f | %f )\n",
                vecBuffer[0].x, vecBuffer[0].y, vecBuffer[0].z, vecBuffer[0].w
            );
        }
        renderer->UnmapBuffer(*storageBuffer);
    }
    catch (const std::exception& e)
    {
        LLGL::Log::Errorf("%s\n", e.what());
    }

    #ifdef _WIN32
    system("pause");
    #endif

    return 0;
}
