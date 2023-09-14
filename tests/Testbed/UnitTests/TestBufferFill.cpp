/*
 * TestBufferFill.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( BufferFill )
{
    // Create small buffer with initial data and read access
    const std::uint32_t fillData[4] = { 0x12345678, 0xFF00FF00, 0xCC20EF90, 0x80706050 };
    static_assert(sizeof(fillData) == 16, "sizeof(fillData) must be 16");

    const std::uint32_t fillData0Only[4] = { fillData[0], fillData[0], fillData[0], fillData[0] };

    BufferDescriptor buf1Desc;
    {
        buf1Desc.size       = sizeof(fillData);
        buf1Desc.bindFlags  = BindFlags::CopyDst;
    }
    CREATE_BUFFER(buf1, buf1Desc, "buf1{size=16}", nullptr);

    BufferDescriptor buf2Desc;
    {
        buf2Desc.size       = 2048;
        buf2Desc.bindFlags  = BindFlags::CopyDst;
    }
    CREATE_BUFFER(buf2, buf2Desc, "buf2{size=2048}", nullptr);

    // Fill buf1
    cmdBuffer->Begin();
    {
        cmdBuffer->FillBuffer(*buf1, 0, fillData[0], buf1Desc.size);
    }
    cmdBuffer->End();

    // Read buf1 feedback data
    std::uint32_t buf1DataFeedback[4] = {};

    renderer->ReadBuffer(*buf1, 0, buf1DataFeedback, sizeof(buf1DataFeedback));
    if (::memcmp(buf1DataFeedback, fillData0Only, sizeof(fillData0Only)) != 0)
    {
        Log::Errorf(
            "Mismatch between data of buffer 1 feedback data [0x%08X, 0x%08X, 0x%08X, 0x%08X] and fill data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
            buf1DataFeedback[0], buf1DataFeedback[1], buf1DataFeedback[2], buf1DataFeedback[3],
            fillData0Only[0], fillData0Only[1], fillData0Only[2], fillData0Only[3]
        );
        return TestResult::FailedMismatch;
    }

    // Fill buf2
    cmdBuffer->Begin();
    {
        for (std::uint64_t buf2Off = 0; buf2Off + sizeof(std::uint32_t) < buf2Desc.size; buf2Off += sizeof(std::uint32_t))
            cmdBuffer->FillBuffer(*buf2, buf2Off, fillData[(buf2Off / sizeof(std::uint32_t)) % 4], sizeof(std::uint32_t));
    }
    cmdBuffer->End();

    // Read buf2 feedback data
    std::uint32_t buf2DataFeedback[4] = {};

    static_assert(sizeof(buf2DataFeedback) == sizeof(fillData), "'buf2DataFeedback' must be same size as 'fillData'");

    for (std::uint64_t buf2Off = 0; buf2Off + sizeof(fillData) < buf2Desc.size; buf2Off += sizeof(fillData))
    {
        // Reset previous data
        ::memset(buf2DataFeedback, 0, sizeof(buf2DataFeedback));

        renderer->ReadBuffer(*buf2, buf2Off, buf2DataFeedback, sizeof(buf2DataFeedback));
        if (::memcmp(buf2DataFeedback, fillData, sizeof(buf2DataFeedback)) != 0)
        {
            Log::Errorf(
                "Mismatch between data of buffer 2 feedback data (offset = %" PRIu64 ") [0x%08X, 0x%08X, 0x%08X, 0x%08X] and fill data [0x%08X, 0x%08X, 0x%08X, 0x%08X]\n",
                buf2Off,
                buf2DataFeedback[0], buf2DataFeedback[1], buf2DataFeedback[2], buf2DataFeedback[3],
                fillData[0], fillData[1], fillData[2], fillData[3]
            );
            return TestResult::FailedMismatch;
        }
    }

    // Delete old buffers
    renderer->Release(*buf1);
    renderer->Release(*buf2);

    return TestResult::Passed;
}

