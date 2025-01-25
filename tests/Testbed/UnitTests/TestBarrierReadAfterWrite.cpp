/*
 * BarrierReadAfterWrite.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>


/*
This test initializes only the first element of one or two buffers and then propagates this value to each next element.
The propagation is done via a compute shader that reads the value from a previous Dispatch() invocation and writes it to the new location.
The test must validate that the correct memory barriers are inserted between these invocations (e.g. UAV barriers in D3D12).
*/
DEF_TEST( BarrierReadAfterWrite )
{
    //TODO: not supported for Vulkan yet
    if (renderer->GetRendererID() == RendererID::Vulkan)
    {
        return TestResult::Skipped;
    }

    if (shaders[CSReadAfterWrite] == nullptr)
    {
        if (renderer->GetRendererID() == RendererID::Metal)
        {
            if (opt.verbose)
                Log::Printf("Read/write texture access not supported for this Metal device\n");
            return TestResult::Skipped;
        }
        else
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }
    }

    constexpr unsigned      numFrames       = 2;
    constexpr std::uint32_t numIterations   = 64;
    constexpr std::uint32_t propagateValue  = 123456789;

    struct Entry
    {
        std::uint32_t a, b;
    };

    const Entry propagateValueEntry = { propagateValue, propagateValue };

    struct Uniforms
    {
        std::uint32_t readPos;
        std::uint32_t writePos;
    }
    uniforms = { 0, 0 };

    // Create small buffer and texture resources with initial data
    BufferDescriptor buf1Desc;
    {
        buf1Desc.size       = sizeof(std::uint32_t) * (numIterations + 1);
        buf1Desc.format     = Format::R32UInt;
        buf1Desc.bindFlags  = BindFlags::Storage | BindFlags::CopyDst; // CopyDst for FillBuffer() command
    }
    CREATE_BUFFER(buf1, buf1Desc, "buf1<uint>", nullptr);

    BufferDescriptor buf2Desc;
    {
        buf2Desc.size       = sizeof(Entry) * (numIterations + 1);
        buf2Desc.bindFlags  = BindFlags::Storage | BindFlags::CopyDst; // CopyDst for FillBuffer() command
        buf2Desc.stride     = sizeof(Entry);
    }
    CREATE_BUFFER(buf2, buf2Desc, "buf2<struct>", nullptr);

    TextureDescriptor tex1Desc;
    {
        tex1Desc.type       = TextureType::Texture1D;
        tex1Desc.bindFlags  = BindFlags::Storage;
        tex1Desc.format     = Format::R32UInt;
        tex1Desc.extent     = Extent3D{ numIterations + 1, 1, 1 };
        tex1Desc.mipLevels  = 1;
    }
    CREATE_TEXTURE(tex1, tex1Desc, "tex1<uint>", nullptr);

    TextureDescriptor tex2Desc;
    {
        tex2Desc.type       = TextureType::Texture2D;
        tex2Desc.bindFlags  = BindFlags::Storage;
        tex2Desc.format     = Format::RG32UInt;
        tex2Desc.extent     = Extent3D{ numIterations + 1, 1, 1 };
        tex2Desc.mipLevels  = 1;
    }
    CREATE_TEXTURE(tex2, tex2Desc, "tex2<uint2>", nullptr);

    // Create compute PSO
    PipelineLayoutDescriptor psoLayoutDesc = Parse(
        "rwbuffer(buf1@1):comp,"
        "rwbuffer(buf2@2):comp,"
        "rwtexture(tex1@3):comp,"
        "rwtexture(tex2@4):comp,"
        "uint(readPos),"
        "uint(writePos),"
    );

    if (frame == 0)
        psoLayoutDesc.barrierFlags = BarrierFlags::Storage;

    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(psoLayoutDesc);

    ComputePipelineDescriptor psoDesc;
    {
        psoDesc.debugName       = (frame == 0 ? "ReadAfterWrite.PSO[ImplicitBarriers]" : "ReadAfterWrite.PSO[ExplicitBarriers]");
        psoDesc.pipelineLayout  = psoLayout;
        psoDesc.computeShader   = shaders[CSReadAfterWrite];
    }
    CREATE_COMPUTE_PSO(pso, psoDesc, nullptr);

    // Initialize first pixel of textures with propagate value
    TextureRegion firstPixelRegion;
    {
        firstPixelRegion.offset = Offset3D{ 0, 0, 0 };
        firstPixelRegion.extent = Extent3D{ 1, 1, 1 };
    }
    ImageView initialTex1Value;
    {
        initialTex1Value.format      = ImageFormat::R;
        initialTex1Value.dataType    = DataType::UInt32;
        initialTex1Value.data        = &propagateValue;
        initialTex1Value.dataSize    = sizeof(propagateValue);
    }
    renderer->WriteTexture(*tex1, firstPixelRegion, initialTex1Value);

    ImageView initialTex2Value;
    {
        initialTex2Value.format      = ImageFormat::RG;
        initialTex2Value.dataType    = DataType::UInt32;
        initialTex2Value.data        = &propagateValueEntry;
        initialTex2Value.dataSize    = sizeof(propagateValueEntry);
    }
    renderer->WriteTexture(*tex2, firstPixelRegion, initialTex2Value);

    // Run compute shader invocations
    cmdBuffer->Begin();
    {
        // Initialize first element of buffers with propagate value
        cmdBuffer->FillBuffer(*buf1, 0, 0x00000000);
        cmdBuffer->FillBuffer(*buf1, 0, propagateValue, sizeof(std::uint32_t));

        cmdBuffer->FillBuffer(*buf2, 0, 0x00000000);
        cmdBuffer->FillBuffer(*buf2, 0, propagateValue, sizeof(Entry));

        // Run compute shader for the given number of iterations
        cmdBuffer->SetPipelineState(*pso);

        cmdBuffer->SetResource(0, *buf1);
        cmdBuffer->SetResource(1, *buf2);
        cmdBuffer->SetResource(2, *tex1);
        cmdBuffer->SetResource(3, *tex2);

        for_range(iter, numIterations)
        {
            uniforms.readPos = iter;
            uniforms.writePos = iter + 1;
            cmdBuffer->SetUniforms(0, &uniforms, sizeof(uniforms));

            cmdBuffer->Dispatch(1, 1, 1);

            if (frame > 0)
            {
                // Use explicit barriers
                Buffer* buffers[] = { buf1, buf2 };
                Texture* textures[] = { tex1, tex2 };
                cmdBuffer->ResourceBarrier(2, buffers, 2, textures);
            }
        }
    }
    cmdBuffer->End();

    // Read back results
    TestResult result = TestResult::Passed;

    std::vector<std::uint32_t> expectedResults;
    expectedResults.resize((numIterations + 1)*(sizeof(Entry)/sizeof(std::uint32_t)), propagateValue);

    auto ValidatePropagatedValues = [this, &result, &expectedResults, frame](const char* name, const void* data, std::size_t dataSize) -> void
    {
        if (::memcmp(expectedResults.data(), data, dataSize) != 0)
        {
            const std::string expectedValuesStr = FormatByteArray(expectedResults.data(), dataSize);
            const std::string actualValuesStr = FormatByteArray(data, dataSize);
            Log::Errorf(
                Log::ColorFlags::StdError,
                "Mismatch between propagated values in %s and expected values [frame %u]:\n"
                " -> Expected: %s\n"
                " -> Actual:   %s\n",
                name, frame, expectedValuesStr.c_str(), actualValuesStr.c_str()
            );
            result = TestResult::FailedMismatch;
        }
        else if (opt.sanityCheck)
        {
            const std::string actualValuesStr = FormatByteArray(data, dataSize);
            Log::Printf(
                Log::ColorFlags::StdAnnotation,
                "Propagated values in %s as expected [frame %u]:\n%s\n",
                name, frame, actualValuesStr.c_str()
            );
        }
    };

    // Evaluate buffer results
    std::vector<std::uint32_t> buf1Results;
    buf1Results.resize(numIterations + 1);
    const std::size_t buf1ResultsSize = buf1Results.size() * sizeof(buf1Results[0]);
    renderer->ReadBuffer(*buf1, 0, buf1Results.data(), buf1ResultsSize);
    ValidatePropagatedValues(buf1_Name, buf1Results.data(), buf1ResultsSize);

    std::vector<Entry> buf2Results;
    buf2Results.resize(numIterations + 1);
    const std::size_t buf2ResultsSize = buf2Results.size() * sizeof(buf2Results[0]);
    renderer->ReadBuffer(*buf2, 0, buf2Results.data(), buf2ResultsSize);
    ValidatePropagatedValues(buf2_Name, buf2Results.data(), buf2ResultsSize);

    // Evaluate texture results
    const TextureRegion readbackTexRegion{ Offset3D{}, Extent3D{ numIterations + 1, 1, 1 } };

    std::vector<std::uint32_t> tex1Results;
    tex1Results.resize(numIterations + 1);
    MutableImageView tex1ResultsView;
    {
        tex1ResultsView.format      = ImageFormat::R;
        tex1ResultsView.dataType    = DataType::UInt32;
        tex1ResultsView.data        = tex1Results.data();
        tex1ResultsView.dataSize    = tex1Results.size() * sizeof(tex1Results[0]);
    }
    renderer->ReadTexture(*tex1, readbackTexRegion, tex1ResultsView);
    ValidatePropagatedValues(tex1_Name, tex1Results.data(), tex1ResultsView.dataSize);

    std::vector<Entry> tex2Results;
    tex2Results.resize(numIterations + 1);
    MutableImageView tex2ResultsView;
    {
        tex2ResultsView.format      = ImageFormat::RG;
        tex2ResultsView.dataType    = DataType::UInt32;
        tex2ResultsView.data        = tex2Results.data();
        tex2ResultsView.dataSize    = tex2Results.size() * sizeof(tex2Results[0]);
    }
    renderer->ReadTexture(*tex2, readbackTexRegion, tex2ResultsView);
    ValidatePropagatedValues(tex2_Name, tex2Results.data(), tex2ResultsView.dataSize);

    // Release resources
    renderer->Release(*buf1);
    renderer->Release(*buf2);
    renderer->Release(*tex1);
    renderer->Release(*tex2);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    if (result == TestResult::Passed && frame + 1 < numFrames)
        return TestResult::ContinueSkipFrame;

    return result;
}

