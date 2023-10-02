/*
 * TestBufferWriteAndRead.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( BufferWriteAndRead )
{
    // Create small buffer with initial data
    const std::uint32_t buf1Initial[4] = { 0xFF, 0x42, 0xCC, 0x80 };
    static_assert(sizeof(buf1Initial) == 16, "sizeof(buf1Initial) must be 16");

    BufferDescriptor buf1Desc;
    {
        buf1Desc.size       = 16;
        buf1Desc.bindFlags  = BindFlags::VertexBuffer;
    }
    CREATE_BUFFER(buf1, buf1Desc, "buf1{size=16}", buf1Initial);

    // Create larger buffer without initial data
    BufferDescriptor buf2Desc;
    {
        buf2Desc.size       = 1024;
        buf2Desc.bindFlags  = BindFlags::IndexBuffer;
    }
    CREATE_BUFFER(buf2, buf2Desc, "buf2{size=1024}", nullptr);

    // Write to buffer
    for (std::uint64_t buf2Off = 0; buf2Off + sizeof(buf1Initial) < buf2Desc.size; buf2Off += sizeof(buf1Initial))
        renderer->WriteBuffer(*buf2, buf2Off, buf1Initial, sizeof(buf1Initial));

    // Read from both buffers and compare
    std::uint32_t buf1Data[4] = {};
    std::uint32_t buf2Data[4] = {};
    for (std::uint64_t buf2Off = 0; buf2Off + sizeof(buf2Data) < buf2Desc.size; buf2Off += sizeof(buf2Data))
    {
        // Clear previous data
        ::memset(buf1Data, 0, sizeof(buf1Data));
        ::memset(buf2Data, 0, sizeof(buf2Data));

        // Read buffers
        renderer->ReadBuffer(*buf1, 0, buf1Data, sizeof(buf1Data));
        renderer->ReadBuffer(*buf2, buf2Off, buf2Data, sizeof(buf2Data));

        // Compare data, it must match
        if (::memcmp(buf1Data, buf1Initial, sizeof(buf1Initial)) != 0)
        {
            Log::Errorf(
                "Mismatch between data of buffer 1 [0x%08X, 0x%08X, 0x%08X, 0x%08X] and initial data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
                buf1Data[0], buf1Data[1], buf1Data[2], buf1Data[3],
                buf1Initial[0], buf1Initial[1], buf1Initial[2], buf1Initial[3]
            );
            return TestResult::FailedMismatch;
        }
        if (::memcmp(buf1Data, buf2Data, sizeof(buf2Data)) != 0)
        {
            Log::Errorf(
                "Mismatch between data of buffer 1 [0x%08X, 0x%08X, 0x%08X, 0x%08X] and buffer 2 (offset = %" PRIu64 ") [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
                buf1Data[0], buf1Data[1], buf1Data[2], buf1Data[3],
                buf2Off,
                buf2Data[0], buf2Data[1], buf2Data[2], buf2Data[3]
            );
            return TestResult::FailedMismatch;
        }
    }

    // Delete old buffers
    renderer->Release(*buf1);
    renderer->Release(*buf2);

    return TestResult::Passed;
}

