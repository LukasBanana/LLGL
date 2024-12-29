/*
* TestSamplerBuffer.cpp
*
* Copyright (c) 2015 Lukas Hermanns. All rights reserved.
* Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
*/

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Scale.h>


/*
Copy entries between typed and structured buffers to test different resource types being bound correctly by a simplified PSO layout description.
E.g. use binding flags BindFlags::Sampled and let LLGL determine how to bind to a typed buffer (samplerBuffer in GLSL) and structured buffer (SSBO in GLSL).
*/
DEF_TEST( SamplerBuffer )
{
    //TODO: not supported for Vulkan and Metal yet
    if (renderer->GetRendererID() != RendererID::OpenGL &&
        renderer->GetRendererID() != RendererID::Direct3D11 &&
        renderer->GetRendererID() != RendererID::Direct3D12)
    {
        return TestResult::Skipped;
    }

    if (shaders[CSSamplerBuffer] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    static TestResult result = TestResult::Passed;
    constexpr unsigned numFrames = 3;

    // Create PSO layouts
    PipelineLayout* psoLayout = nullptr;

    switch (frame)
    {
        case 0:
            result = TestResult::Passed;
            psoLayout = renderer->CreatePipelineLayout(
                Parse(
                    "cbuffer(Config@4):comp,"
                    "buffer(inTypedBuffer@0):comp,"
                    "rwbuffer(outTypedBuffer@1):comp,"
                    "buffer(inStructBuffer@2):comp,"
                    "rwbuffer(outStructBuffer@3):comp,"
                )
            );
            break;

        case 1:
            psoLayout = renderer->CreatePipelineLayout(
                Parse(
                    "cbuffer(Config@4):comp,"
                    "buffer(inTypedBuffer@0):comp,"
                    "heap{"
                    "  rwbuffer(outTypedBuffer@1):comp,"
                    "  buffer(inStructBuffer@2):comp,"
                    "},"
                    "rwbuffer(outStructBuffer@3):comp,"
                )
            );
            break;

        case 2:
            psoLayout = renderer->CreatePipelineLayout(
                Parse(
                    "heap{"
                    "  cbuffer(Config@4):comp,"
                    "  buffer(inTypedBuffer@0):comp,"
                    "  rwbuffer(outTypedBuffer@1):comp,"
                    "  buffer(inStructBuffer@2):comp,"
                    "  rwbuffer(outStructBuffer@3):comp,"
                    "},"
                )
            );
            break;
    }

    // Create compute PSOs
    ComputePipelineDescriptor psoDesc;
    {
        psoDesc.computeShader   = shaders[CSSamplerBuffer];
        psoDesc.pipelineLayout  = psoLayout;
    }
    CREATE_COMPUTE_PSO(pso, psoDesc, "SamplerBuffer.PSO");

    // Create typed buffers
    constexpr std::uint32_t numEntries = 2;
    constexpr std::int32_t initialTypedValues[numEntries] = { 42, 600 };
    constexpr std::int32_t initialStructValues[numEntries*2] = { 60, -12, 99, 16 };

    BufferDescriptor typedBufDesc;
    {
        typedBufDesc.size       = sizeof(std::int32_t)*numEntries;
        typedBufDesc.bindFlags  = BindFlags::Sampled;
        typedBufDesc.format     = Format::R32SInt;
    }
    CREATE_BUFFER(inTypedBuffer, typedBufDesc, "inTypedBuffer", &initialTypedValues[0]);

    {
        typedBufDesc.bindFlags  = BindFlags::Storage | BindFlags::CopyDst;
    }
    CREATE_BUFFER(outTypedBuffer, typedBufDesc, "outTypedBuffer", nullptr);

    // Create structured buffers
    BufferDescriptor structBufDesc;
    {
        structBufDesc.size      = sizeof(std::int32_t)*2*numEntries;
        structBufDesc.bindFlags = BindFlags::Sampled;
        structBufDesc.stride    = sizeof(std::int32_t)*2;
    }
    CREATE_BUFFER(inStructBuffer, structBufDesc, "inStructBuffer", &initialStructValues[0]);

    {
        structBufDesc.bindFlags  = BindFlags::Storage | BindFlags::CopyDst;
    }
    CREATE_BUFFER(outStructBuffer, structBufDesc, "outStructBuffer", nullptr);

    // Create constant buffer
    const std::int32_t multipliers[3] = { 2, 3, 4 };
    BufferDescriptor cbufferDesc;
    {
        cbufferDesc.size        = sizeof(multipliers);
        cbufferDesc.bindFlags   = BindFlags::ConstantBuffer;
    }
    CREATE_BUFFER(configBuffer, cbufferDesc, "configBuffer", multipliers);

    // Create resource heaps
    ResourceHeap* resHeap = nullptr;
    switch (frame)
    {
        case 1:
            resHeap = renderer->CreateResourceHeap(psoLayout, { outTypedBuffer, inStructBuffer });
            break;

        case 2:
            resHeap = renderer->CreateResourceHeap(psoLayout, { configBuffer, inTypedBuffer, outTypedBuffer, inStructBuffer, outStructBuffer });
            break;
    }

    // Dispatch compute kernels
    cmdBuffer->Begin();
    {
        cmdBuffer->FillBuffer(*outTypedBuffer, 0, 0xDEADBEEF);
        cmdBuffer->FillBuffer(*outStructBuffer, 0, 0xDEADBEEF);
        cmdBuffer->SetPipelineState(*pso);

        switch (frame)
        {
            case 0:
                cmdBuffer->SetResource(0, *configBuffer);
                cmdBuffer->SetResource(1, *inTypedBuffer);
                cmdBuffer->SetResource(2, *outTypedBuffer);
                cmdBuffer->SetResource(3, *inStructBuffer);
                cmdBuffer->SetResource(4, *outStructBuffer);
                break;

            case 1:
                cmdBuffer->SetResource(0, *configBuffer);
                cmdBuffer->SetResource(1, *inTypedBuffer);
                cmdBuffer->SetResource(2, *outStructBuffer);
                cmdBuffer->SetResourceHeap(*resHeap);
                break;

            case 2:
                cmdBuffer->SetResourceHeap(*resHeap);
                break;
        }

        cmdBuffer->Dispatch(numEntries, 1, 1);
    }
    cmdBuffer->End();

    // Evaluate readback result
    if (opt.verbose)
        Log::Printf("Sampler buffer iteration %u\n", frame);

    std::int32_t typedResults[numEntries] = {};
    renderer->ReadBuffer(*outTypedBuffer, 0, typedResults, sizeof(typedResults));

    for_range(i, numEntries)
    {
        const std::int32_t expectedValue = initialTypedValues[i]*2;
        if (typedResults[i] != expectedValue)
        {
            Log::Errorf(
                Log::ColorFlags::StdError,
                "Mismatch between data[%u] of outTypedBuffer (%d) and expected value (%d) [iteration %u]\n",
                i, typedResults[i], expectedValue, frame
            );
            result = TestResult::FailedMismatch;
        }
        else if (opt.sanityCheck)
        {
            Log::Printf(
                Log::ColorFlags::StdAnnotation,
                "Sanity check for outTypedBuffer.%u (%d) [iteration %u]\n",
                i, typedResults[i], frame
            );
        }
    }

    std::int32_t structResults[numEntries*2] = {};
    renderer->ReadBuffer(*outStructBuffer, 0, structResults, sizeof(structResults));

    for_range(i, numEntries)
    {
        const std::int32_t expectedValueA = initialStructValues[i*2    ]*3;
        const std::int32_t expectedValueB = initialStructValues[i*2 + 1]*4;
        if (structResults[i*2    ] != expectedValueA ||
            structResults[i*2 + 1] != expectedValueB)
        {
            Log::Errorf(
                Log::ColorFlags::StdError,
                "Mismatch between data[%u] of outStructBuffer (a=%d, b=%d) and expected value (a=%d, b=%d) [iteration %u]\n",
                i, structResults[i*2], structResults[i*2 + 1], expectedValueA, expectedValueB, frame
            );
            result = TestResult::FailedMismatch;
        }
        else if (opt.sanityCheck)
        {
            Log::Printf(
                Log::ColorFlags::StdAnnotation,
                "Sanity check for outStructBuffer.%u (a = %d, b = %d) [iteration %u]\n",
                i, structResults[i*2], structResults[i*2 + 1], frame
            );
        }
    }

    // Clear resources
    renderer->Release(*inTypedBuffer);
    renderer->Release(*inStructBuffer);
    renderer->Release(*outTypedBuffer);
    renderer->Release(*outStructBuffer);
    renderer->Release(*configBuffer);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    if (frame + 1 < numFrames)
        return TestResult::Continue;

    return result;
}

