/*
 * TestBufferCopy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( BufferCopy )
{
    // Create small buffer with initial data and read access
    const std::uint32_t buf1Initial[4] = { 0x01, 0x45, 0x89, 0xCD };
    static_assert(sizeof(buf1Initial) == 16, "sizeof(buf1Initial) must be 16");

    BufferDescriptor buf1Desc;
    {
        buf1Desc.size       = sizeof(buf1Initial);
        buf1Desc.bindFlags  = BindFlags::CopySrc;
    }
    CREATE_BUFFER(buf1, buf1Desc, "buf1{size=16,src}", buf1Initial);

    // Create small buffer with initial data and write access
    BufferDescriptor buf2Desc;
    {
        buf2Desc.size       = sizeof(buf1Initial);
        buf2Desc.bindFlags  = BindFlags::CopySrc | BindFlags::CopyDst;
    }
    CREATE_BUFFER(buf2, buf2Desc, "buf2{size=16,src/dst}", nullptr);

    // Create larger buffer without initial data and write access
    BufferDescriptor buf3Desc;
    {
        buf3Desc.size       = 4096;
        buf3Desc.bindFlags  = BindFlags::CopyDst;
    }
    CREATE_BUFFER(buf3, buf3Desc, "buf3{size=4096,dst}", nullptr);

    // Copy buf1 into buf2
    cmdBuffer->Begin();
    {
        cmdBuffer->CopyBuffer(*buf2, 0, *buf1, 0, buf1Desc.size);
    }
    cmdBuffer->End();

    // Read buf2 feedback data
    std::uint32_t buf2DataFeedback[4] = {};
    renderer->ReadBuffer(*buf2, 0, buf2DataFeedback, sizeof(buf2DataFeedback));
    if (::memcmp(buf2DataFeedback, buf1Initial, sizeof(buf1Initial)) != 0)
    {
        Log::Errorf(
            "Mismatch between data of buffer 2 feedback data [0x%08X, 0x%08X, 0x%08X, 0x%08X] and initial data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
            buf2DataFeedback[0], buf2DataFeedback[1], buf2DataFeedback[2], buf2DataFeedback[3],
            buf1Initial[0], buf1Initial[1], buf1Initial[2], buf1Initial[3]
        );
        return TestResult::FailedMismatch;
    }

    // Copy buf2 into all all regions of buf3
    cmdBuffer->Begin();
    {
        for (std::uint64_t buf3Off = 0; buf3Off + buf2Desc.size < buf3Desc.size; buf3Off += buf2Desc.size)
            cmdBuffer->CopyBuffer(*buf3, buf3Off, *buf2, 0, buf2Desc.size);
    }
    cmdBuffer->End();

    // Read buf3 feedback data
    std::uint32_t buf3DataFeedback[4] = {};

    for (std::uint64_t buf3Off = 0; buf3Off + sizeof(buf3DataFeedback) < buf3Desc.size; buf3Off += sizeof(buf3DataFeedback))
    {
        // Clear previous data
        ::memset(buf3DataFeedback, 0, sizeof(buf3DataFeedback));

        // Read buf3
        renderer->ReadBuffer(*buf3, buf3Off, buf3DataFeedback, sizeof(buf3DataFeedback));
        if (::memcmp(buf3DataFeedback, buf1Initial, sizeof(buf1Initial)) != 0)
        {
            Log::Errorf(
                "Mismatch between data of buffer 3 feedback data (offset = %" PRIu64 ") [0x%08X, 0x%08X, 0x%08X, 0x%08X] and initial data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
                buf3Off,
                buf2DataFeedback[0], buf2DataFeedback[1], buf2DataFeedback[2], buf2DataFeedback[3],
                buf1Initial[0], buf1Initial[1], buf1Initial[2], buf1Initial[3]
            );
            return TestResult::FailedMismatch;
        }
    }

    // Delete old buffers
    renderer->Release(*buf1);
    renderer->Release(*buf2);
    renderer->Release(*buf3);

    return TestResult::Passed;
}

