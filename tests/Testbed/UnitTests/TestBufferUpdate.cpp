/*
 * TestBufferUpdate.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( BufferUpdate )
{
    TestResult result = TestResult::Passed;

    // Create small buffer with initial data and read access
    const std::uint32_t updateData[4] = { 0x12345678, 0xFF00FF00, 0xCC20EF90, 0x80706050 };
    static_assert(sizeof(updateData) == 16, "sizeof(updateData) must be 16");

    std::uint32_t readbackData[4] = {};
    static_assert(sizeof(readbackData) == sizeof(updateData), "'readbackData' must have same size as 'updateData'");

    BufferDescriptor buf1Desc;
    {
        buf1Desc.size       = 1024;
        buf1Desc.bindFlags  = BindFlags::ConstantBuffer;
    }
    CREATE_BUFFER(buf1, buf1Desc, "buf1{size=1024,cbuffer}", nullptr);

    BufferDescriptor buf2Desc;
    {
        buf2Desc.size       = 1024;
        buf2Desc.bindFlags  = BindFlags::ConstantBuffer;
        buf2Desc.miscFlags  = MiscFlags::DynamicUsage; // Create a dynamic buffer for frequent updates
    }
    CREATE_BUFFER(buf2, buf2Desc, "buf2{size=1024,dynamic}", nullptr);

    BufferDescriptor buf3Desc;
    {
        buf3Desc.size       = 2048;
        buf3Desc.bindFlags  = BindFlags::VertexBuffer | BindFlags::IndexBuffer;
    }
    CREATE_BUFFER(buf3, buf3Desc, "buf3{size=2048,vert/indx}", nullptr);

    BufferDescriptor buf4Desc;
    {
        buf4Desc.size       = 4096;
        buf4Desc.bindFlags  = BindFlags::Storage | BindFlags::Sampled;
    }
    CREATE_BUFFER_COND(caps.features.hasStorageBuffers, buf4, buf4Desc, "buf4{size=4096,r/w}", nullptr);

    // Perform same update tests on all buffers
    Buffer* buffers[] = { buf1, buf2, buf3, buf4 };
    const char* bufferNames[] = { buf1_Name, buf2_Name, buf3_Name, buf4_Name };
    const std::uint64_t bufferSizes[] = { buf1Desc.size, buf2Desc.size, buf3Desc.size, buf4Desc.size };

    for (int i = 0; i < sizeof(bufferSizes)/sizeof(bufferSizes[0]); ++i)
    {
        if (buffers[i] == nullptr)
            continue;

        // Fill buffer
        cmdBuffer->Begin();
        {
            for (std::uint64_t bufOff = 0; bufOff + sizeof(updateData) < bufferSizes[i]; bufOff += sizeof(updateData))
                cmdBuffer->UpdateBuffer(*buffers[i], bufOff, updateData, sizeof(updateData));
        }
        cmdBuffer->End();

        // Read feedback data
        for (std::uint64_t bufOff = 0; bufOff + sizeof(readbackData) < bufferSizes[i]; bufOff += sizeof(readbackData))
        {
            ::memset(readbackData, 0, sizeof(readbackData));
            renderer->ReadBuffer(*buffers[i], bufOff, readbackData, sizeof(readbackData));
            if (::memcmp(readbackData, updateData, sizeof(readbackData)) != 0)
            {
                Log::Errorf(
                    Log::ColorFlags::StdError,
                    "Mismatch between data of buffer [%d] \"%s\" readback data (offset = %" PRIu64 ") [0x%08X, 0x%08X, 0x%08X, 0x%08X] and update data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
                    i, bufferNames[i], bufOff,
                    readbackData[0], readbackData[1], readbackData[2], readbackData[3],
                    updateData[0], updateData[1], updateData[2], updateData[3]
                );
                result = TestResult::FailedMismatch;
                if (!opt.greedy)
                    return result;
            }
        }
    }

    // Test large buffer updates
    BufferDescriptor buf4LargeDesc;
    {
        buf4LargeDesc.size      = 1 << 18u; // Must be at least larger than 2^16 to ensure Vulkan backend cannot use vkCmdBufferUpdate()
        buf4LargeDesc.bindFlags = BindFlags::CopyDst | BindFlags::Storage;
    }
    CREATE_BUFFER(buf4Large, buf4LargeDesc, "buf4Large", nullptr);

    std::vector<std::uint32_t> buf4LargeData;
    buf4LargeData.resize(static_cast<std::size_t>(buf4LargeDesc.size) / sizeof(std::uint32_t), 0xF000BAAA); // Initialize with test value

    cmdBuffer->Begin();
    {
        cmdBuffer->UpdateBuffer(*buf4Large, 0, buf4LargeData.data(), buf4LargeData.size() * sizeof(std::uint32_t));
    }
    cmdBuffer->End();

    // Read feedback data
    std::vector<std::uint32_t> buf4LargeReadback;
    buf4LargeReadback.resize(buf4LargeData.size(), 0xDEADBEEF); // Initialize with different value

    renderer->ReadBuffer(*buf4Large, 0, buf4LargeReadback.data(), buf4LargeReadback.size() * sizeof(std::uint32_t));

    if (::memcmp(buf4LargeData.data(), buf4LargeReadback.data(), buf4LargeData.size() * sizeof(std::uint32_t)) != 0)
    {
        // Try to find start and end point of mismatch
        std::size_t start = 0;
        for_range(i, buf4LargeData.size())
        {
            if (buf4LargeData[i] != buf4LargeReadback[i])
            {
                start = i;
                break;
            }
        }

        std::size_t end = buf4LargeData.size();
        for_range_reverse(i, buf4LargeData.size())
        {
            if (buf4LargeData[i] != buf4LargeReadback[i])
            {
                end = i;
                break;
            }
        }

        const std::uint64_t startOffset = start*sizeof(std::uint32_t);
        const std::uint64_t endOffset = end*sizeof(std::uint32_t);

        Log::Errorf(
            Log::ColorFlags::StdError,
            "Mismatch between data of buffer \"%s\" readback data and update data within the range [%" PRIu64 ", %" PRIu64 ")\n"
            " -> Start range [%" PRIu64 "]: Expected 0x%08X, Actual 0x%08X\n"
            " -> End range   [%" PRIu64 "]: Expected 0x%08X, Actual 0x%08X\n",
            buf4Large_Name, startOffset, endOffset,
            startOffset, buf4LargeData[start], buf4LargeReadback[start],
            endOffset, buf4LargeData[end], buf4LargeReadback[end]
        );

        result = TestResult::FailedMismatch;
    }

    // Release resources
    SAFE_RELEASE(buf1);
    SAFE_RELEASE(buf2);
    SAFE_RELEASE(buf3);
    SAFE_RELEASE(buf4Large);

    return result;
}

