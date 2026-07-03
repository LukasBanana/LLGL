/*
* TestByteBuffer.cpp
*
* Copyright (c) 2015 Lukas Hermanns. All rights reserved.
* Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
*/

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Scale.h>


/*
Test creation and transformation of ByteAddressBuffer in D3D.
*/
DEF_TEST( ByteBuffer )
{
    if (!caps.features.hasComputeShaders)
        return TestResult::Skipped;

    //TODO: implement for Metal, Vulkan, and OpenGL
    if (renderer->GetRendererID() != RendererID::Direct3D11 && renderer->GetRendererID() != RendererID::Direct3D12)
        return TestResult::Skipped;

    if (shaders[CSByteBuffer] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    TestResult result = TestResult::Passed;

    // Create PSO layouts
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(
        Parse(
            "buffer(inByteBufferA@0):comp,"
            "rwbuffer(outByteBufferB@1):comp,"
            "rwbuffer(outByteBufferC@2):comp,"
        )
    );

    // Create compute PSOs
    ComputePipelineDescriptor psoDesc;
    {
        psoDesc.computeShader   = shaders[CSByteBuffer];
        psoDesc.pipelineLayout  = psoLayout;
    }
    CREATE_COMPUTE_PSO(pso, psoDesc, "ByteBuffer.PSO");

    // Create byte buffers and ensure their size is a multiple of 4
    std::string inStringBuffer = "This is a simple string buffer that is meant to be transformed by a compute shader!";
    inStringBuffer.resize(((inStringBuffer.size() + 3) / 4) * 4);

    std::vector<std::uint32_t> arbitraryByteBufferA;
    std::vector<std::uint32_t> arbitraryByteBufferB;

    arbitraryByteBufferA.resize(inStringBuffer.size() / 4);
    arbitraryByteBufferB.resize(inStringBuffer.size() / 4);

    std::memcpy(arbitraryByteBufferA.data(), inStringBuffer.c_str(), inStringBuffer.size());
    for_range(i, arbitraryByteBufferA.size())
        arbitraryByteBufferB[i] = arbitraryByteBufferA[i] ^ 0xDEADBEEF;

    // Create GPU buffers
    BufferDescriptor inBufADesc;
    {
        inBufADesc.size         = arbitraryByteBufferA.size() * sizeof(std::uint32_t);
        inBufADesc.bindFlags    = BindFlags::Sampled;
    }
    CREATE_BUFFER(inByteBufferA, inBufADesc, "inByteBufferA", arbitraryByteBufferA.data());

    BufferDescriptor outBufBDesc;
    {
        outBufBDesc.size        = arbitraryByteBufferB.size() * sizeof(std::uint32_t);
        outBufBDesc.bindFlags   = BindFlags::Storage;
    }
    CREATE_BUFFER(outByteBufferB, outBufBDesc, "outByteBufferB", nullptr);

    BufferDescriptor outBufCDesc;
    {
        outBufCDesc.size        = inStringBuffer.size();
        outBufCDesc.bindFlags   = BindFlags::Sampled | BindFlags::Storage;
    }
    CREATE_BUFFER(outByteBufferC, outBufCDesc, "outByteBufferC", inStringBuffer.data());

    constexpr std::uint32_t kLocalThreadCount = 64;

    const std::uint32_t numThreadGroups = (static_cast<std::uint32_t>(inStringBuffer.size()) + kLocalThreadCount - 1) / kLocalThreadCount;

    // Dispatch compute kernels
    BEGIN();
    {
        cmdBuffer->SetPipelineState(*pso);
        cmdBuffer->SetResource(0, *inByteBufferA);
        cmdBuffer->SetResource(1, *outByteBufferB);
        cmdBuffer->SetResource(2, *outByteBufferC);
        cmdBuffer->Dispatch(numThreadGroups, 1, 1);
    }
    END();

    // Evaluate readback result
    std::vector<char> bufDataFeedback;
    bufDataFeedback.resize(arbitraryByteBufferB.size() * sizeof(std::uint32_t));

    renderer->ReadBuffer(*outByteBufferB, 0, bufDataFeedback.data(), bufDataFeedback.size());

    if (::memcmp(arbitraryByteBufferB.data(), bufDataFeedback.data(), bufDataFeedback.size()) != 0)
    {
        const std::string expectedDataStr = TestbedContext::FormatByteArray(bufDataFeedback.data(), bufDataFeedback.size());
        const std::string actualDataStr = TestbedContext::FormatByteArray(arbitraryByteBufferB.data(), arbitraryByteBufferB.size() * sizeof(std::uint32_t));
        Log::Errorf(
            Log::ColorFlags::StdError,
            "Mismatch between data of buffer %s and expected data:\n"
            " -> Expected: [%s]\n"
            " -> Actual:   [%s]\n",
            outByteBufferB_Name, expectedDataStr.c_str(), actualDataStr.c_str()
        );
        result = TestResult::FailedMismatch;
    }

    // Evaluate string transformation
    std::string ucaseStringBuffer;
    ucaseStringBuffer.resize(inStringBuffer.size());
    for_range(i, inStringBuffer.size())
    {
        if (inStringBuffer[i] >= 'a' && inStringBuffer[i] <= 'z')
            ucaseStringBuffer[i] = inStringBuffer[i] - ('a' - 'A');
        else
            ucaseStringBuffer[i] = inStringBuffer[i];
    }

    std::string bufStringFeedback;
    bufStringFeedback.resize(inStringBuffer.size());
    renderer->ReadBuffer(*outByteBufferC, 0, &bufStringFeedback[0], bufStringFeedback.size());

    if (bufStringFeedback != ucaseStringBuffer)
    {
        Log::Printf(
            Log::ColorFlags::StdError,
            "Mismatch between transformed string buffer and expected upper case string:\n"
            " -> Expected:  \"%s\"\n"
            " -> Actual:    \"%s\"\n",
            ucaseStringBuffer.c_str(), bufStringFeedback.c_str()
        );
        result = TestResult::FailedMismatch;
    }
    else if (opt.sanityCheck)
    {
        Log::Printf(
            Log::ColorFlags::StdAnnotation,
            "Transformed string buffer:\n"
            " -> Input:  \"%s\"\n"
            " -> Output: \"%s\"\n",
            inStringBuffer.c_str(), ucaseStringBuffer.c_str()
        );
    }

    // Clear resources
    SAFE_RELEASE(inByteBufferA);
    SAFE_RELEASE(outByteBufferB);
    SAFE_RELEASE(outByteBufferC);
    SAFE_RELEASE(pso);
    SAFE_RELEASE(psoLayout);

    return result;
}

