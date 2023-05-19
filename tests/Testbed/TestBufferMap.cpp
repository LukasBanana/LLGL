/*
 * TestContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( BufferMap )
{
    // Create small buffer with initial data and read access
    const std::uint32_t buf1Initial[4] = { 0x12, 0x34, 0x56, 0x78 };
    static_assert(sizeof(buf1Initial) == 16, "sizeof(buf1Initial) must be 16");

    BufferDescriptor buf1Desc;
    {
        buf1Desc.size           = 16;
        buf1Desc.cpuAccessFlags = CPUAccessFlags::Read;
    }
    CREATE_BUFFER(buf1, buf1Desc, "buf1{size=16,r}", buf1Initial);

    // Create small buffer with initial data and write access
    BufferDescriptor buf2Desc;
    {
        buf2Desc.size           = 16;
        buf2Desc.cpuAccessFlags = CPUAccessFlags::Write;
    }
    CREATE_BUFFER(buf2, buf2Desc, "buf2{size=16,w}", nullptr);

    // Create larger buffer without initial data and write access
    BufferDescriptor buf3Desc;
    {
        buf3Desc.size           = 2048;
        buf3Desc.cpuAccessFlags = CPUAccessFlags::ReadWrite;
    }
    CREATE_BUFFER(buf3, buf3Desc, "buf3{size=2048,rw}", nullptr);

    // Map buf1 into CPU memory space
    if (void* buf1Data = renderer->MapBuffer(*buf1, CPUAccess::ReadOnly))
    {
        auto* buf1DataU32 = reinterpret_cast<std::uint32_t*>(buf1Data);
        const int result = ::memcmp(buf1DataU32, buf1Initial, sizeof(buf1Initial));
        renderer->UnmapBuffer(*buf1);
        if (result != 0)
        {
            Log::Errorf(
                "Mismatch between data of CPU mapped buffer 1 [0x%08X, 0x%08X, 0x%08X, 0x%08X] and initial data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
                buf1DataU32[0], buf1DataU32[1], buf1DataU32[2], buf1DataU32[3],
                buf1Initial[0], buf1Initial[1], buf1Initial[2], buf1Initial[3]
            );
            return TestResult::FailedMismatch;
        }
    }
    else
    {
        Log::Errorf("Failed to map buffer 1 into CPU memory space for reading\n");
        return TestResult::FailedErrors;
    }

    // Map buf2 into CPU memory space and write to
    if (void* buf2Data = renderer->MapBuffer(*buf2, CPUAccess::WriteOnly))
    {
        // Write memory into buf2
        auto* buf2DataU32 = reinterpret_cast<std::uint32_t*>(buf2Data);
        ::memcpy(buf2DataU32, buf1Initial, sizeof(buf1Initial));
        renderer->UnmapBuffer(*buf2);

        // Read buffer and compare data
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
    }
    else
    {
        Log::Errorf("Failed to map buffer 2 into CPU memory space for writing (WriteOnly)\n");
        return TestResult::FailedErrors;
    }

    // Map buf3 into CPU memory space and write to
    for (std::uint64_t buf3Off = 0; buf3Off + sizeof(buf1Initial) < buf3Desc.size; buf3Off += sizeof(buf1Initial))
    {
        if (void* buf3Data = renderer->MapBuffer(*buf3, CPUAccess::WriteOnly, buf3Off, sizeof(buf1Initial)))
        {
            // Write data to buffer
            ::memcpy(buf3Data, buf1Initial, sizeof(buf1Initial));
            renderer->UnmapBuffer(*buf3);
        }
        else
        {
            Log::Errorf("Failed to map buffer 3 (offset = %" PRIu64 ") into CPU memory space for writing (WriteOnly)\n", buf3Off);
            return TestResult::FailedErrors;
        }
    }

    // Map buf3 into CPU memory space and read from
    for (std::uint64_t buf3Off = 0; buf3Off + sizeof(buf1Initial) < buf3Desc.size; buf3Off += sizeof(buf1Initial))
    {
        if (void* buf3Data = renderer->MapBuffer(*buf3, CPUAccess::ReadOnly, buf3Off, sizeof(buf1Initial)))
        {
            // Read data from buffer
            auto* buf3DataU32 = reinterpret_cast<std::uint32_t*>(buf3Data);
            int result = ::memcmp(buf3DataU32, buf1Initial, sizeof(buf1Initial));
            renderer->UnmapBuffer(*buf3);
            if (result != 0)
            {
                Log::Errorf(
                    "Mismatch between data of buffer 3 (offset = %" PRIu64 ") [0x%08X, 0x%08X, 0x%08X, 0x%08X] and initial data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
                    buf3Off,
                    buf3DataU32[0], buf3DataU32[1], buf3DataU32[2], buf3DataU32[3],
                    buf1Initial[0], buf1Initial[1], buf1Initial[2], buf1Initial[3]
                );
                return TestResult::FailedMismatch;
            }
        }
        else
        {
            Log::Errorf("Failed to map buffer 3 (offset = %" PRIu64 ") into CPU memory space for reading (ReadOnly)\n", buf3Off);
            return TestResult::FailedErrors;
        }
    }

    // Delete old buffers
    renderer->Release(*buf1);
    renderer->Release(*buf2);
    renderer->Release(*buf3);

    return TestResult::Passed;
}


