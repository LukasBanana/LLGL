/*
 * TestPipelineCaching.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/ForRange.h>


// This is a performance regression test only

DEF_TEST( PipelineCaching )
{
    if (shaders[VSTextured] == nullptr || shaders[PSTextured] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    constexpr unsigned numPSOs = 10;

    // Initialize PSO descriptor
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = layouts[PipelineTextured];
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSTextured];
        psoDesc.fragmentShader      = shaders[PSTextured];
        psoDesc.depth.testEnabled   = true;
        psoDesc.depth.writeEnabled  = true;
        psoDesc.rasterizer.cullMode = CullMode::Back;
    }

    auto CreateTestPSO = [this, &psoDesc](PipelineCache* cache, double& outElapsedMS) -> PipelineState*
    {
        cmdQueue->WaitIdle();

        const std::uint64_t startTime = Timer::Tick();
        PipelineState* pso = renderer->CreatePipelineState(psoDesc, cache);
        const std::uint64_t endTime = Timer::Tick();

        outElapsedMS = (static_cast<double>(endTime - startTime) / static_cast<double>(Timer::Frequency())) * 1000.0;

        return pso;
    };

    // Create N PSOs without caching and immediate release
    double elapsedTime[numPSOs];

    const char* captions[] =
    {
        "Elapsed times for uncached temporary PSOs: ",
        "Elapsed times for uncached PSOs:           ",
        "Elapsed times for cached PSOs:             ",
    };

    auto PrintElapsedTimes = [this, &elapsedTime, numPSOs](const char* caption) -> void
    {
        if (this->opt.showTiming)
        {
            Log::Printf(caption);
            for_range(i, numPSOs)
            {
                if (i > 0)
                    Log::Printf(", ");
                Log::Printf("%.2f ms", elapsedTime[i]);
            }
            Log::Printf("\n");
        }
    };

    for_range(i, numPSOs)
    {
        PipelineState* tempPSO = CreateTestPSO(nullptr, elapsedTime[i]);
        renderer->Release(*tempPSO);
    }

    PrintElapsedTimes(captions[0]);

    // Create N PSOs without caching and delayed release
    PipelineState* pipelineStates[numPSOs] = {};

    for_range(i, numPSOs)
        pipelineStates[i] = CreateTestPSO(nullptr, elapsedTime[i]);

    PrintElapsedTimes(captions[1]);

    for_range(i, numPSOs)
        renderer->Release(*pipelineStates[i]);

    // Create N PSOs with caching
    PipelineCache* pipelineCache = renderer->CreatePipelineCache();

    for_range(i, numPSOs)
        pipelineStates[i] = CreateTestPSO(pipelineCache, elapsedTime[i]);

    PrintElapsedTimes(captions[2]);

    for_range(i, numPSOs)
        renderer->Release(*pipelineStates[i]);
    renderer->Release(*pipelineCache);

    return TestResult::Passed;
}


