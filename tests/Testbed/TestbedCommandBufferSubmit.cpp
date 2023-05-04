/*
 * TestbedContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( CommandBufferSubmit )
{
    // Create multi-submit command buffers
    constexpr unsigned numCmdBuffers = 2;
    static CommandBuffer* multiSubmitCmdBuffers[numCmdBuffers] = {};

    if (frame == 0)
    {
        const ClearValue clearValues[] =
        {
            ClearValue{ 0.2f, 1.0f, 0.2f, 1 },
            ClearValue{ 0.2f, 0.4f, 0.8f, 1 }
        };
        constexpr unsigned numClearValues = sizeof(clearValues)/sizeof(clearValues[0]);

        CommandBufferDescriptor cmdBufferDesc;
        cmdBufferDesc.flags = CommandBufferFlags::MultiSubmit;

        for_range(i, numCmdBuffers)
        {
            auto* cmdBuf = renderer->CreateCommandBuffer(cmdBufferDesc);

            cmdBuf->Begin();
            {
                cmdBuf->BeginRenderPass(*swapChain);
                cmdBuf->Clear(ClearFlags::Color, clearValues[i % numClearValues]);
                cmdBuf->EndRenderPass();
            }
            cmdBuf->End();

            multiSubmitCmdBuffers[i] = cmdBuf;
        }
    }

    constexpr unsigned numSubmissions = 16;
    if (frame < numSubmissions)
    {
        // Submit command buffers several times
        cmdQueue->Submit(*multiSubmitCmdBuffers[frame % numCmdBuffers]);

        // Read swap chain color
        //TODO
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));

        return TestResult::Continue;
    }
    else
    {
        // Delete old command buffers
        for (auto* cmdBuf : multiSubmitCmdBuffers)
            renderer->Release(*cmdBuf);

        return TestResult::Passed;
    }
}


