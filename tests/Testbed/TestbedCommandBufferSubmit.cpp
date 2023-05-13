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
    constexpr unsigned maxNumCmdBuffers = 2;
    static CommandBuffer* multiSubmitCmdBuffers[maxNumCmdBuffers] = {};

    const unsigned numCmdBuffers = swapChain->GetNumSwapBuffers();

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

        for_range(swapBufferIndex, maxNumCmdBuffers)
        {
            auto* cmdBuf = renderer->CreateCommandBuffer(cmdBufferDesc);

            cmdBuf->Begin();
            {
                cmdBuf->BeginRenderPass(*swapChain, nullptr, 0, nullptr, swapBufferIndex % numCmdBuffers);
                cmdBuf->Clear(ClearFlags::Color, clearValues[swapBufferIndex % numClearValues]);
                cmdBuf->EndRenderPass();
            }
            cmdBuf->End();

            multiSubmitCmdBuffers[swapBufferIndex] = cmdBuf;
        }
    }

    constexpr unsigned numSubmissions = 16;
    if (frame < numSubmissions)
    {
        // Select the correct command buffer for the current swap-chain index
        unsigned swapBufferIndex = (numCmdBuffers == 1 ? frame % maxNumCmdBuffers : swapChain->GetCurrentSwapIndex());
        if (swapBufferIndex >= maxNumCmdBuffers)
        {
            Log::Errorf("Not enough command buffers (%u) for swap-chain size (%u)\n", maxNumCmdBuffers, numCmdBuffers);
            return TestResult::FailedErrors;
        }

        // Submit command buffers several times
        cmdQueue->Submit(*multiSubmitCmdBuffers[swapBufferIndex]);

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


