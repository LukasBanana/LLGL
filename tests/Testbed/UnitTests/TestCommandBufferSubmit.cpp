/*
 * TestCommandBufferSubmit.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"


DEF_TEST( CommandBufferSubmit )
{
    constexpr unsigned maxNumCmdBuffers = 2;

    static CommandBuffer* multiSubmitCmdBuffers[maxNumCmdBuffers] = {};
    static Texture* framebufferResultTex;

    const unsigned numCmdBuffers = swapChain->GetNumSwapBuffers();

    const ClearValue clearValues[] =
    {
        ClearValue{ 0.2f, 1.0f, 0.2f, 1 }, // Green
        ClearValue{ 0.2f, 0.4f, 0.8f, 1 }  // Blue
    };

    const TextureRegion texRegion{ Offset3D{ 0, 0, 0 }, Extent3D{ 1, 1, 1 } };

    if (frame == 0)
    {
        // Create 1x1 texture for framebuffer result (i.e. to read a single pixel)
        const std::uint8_t initialImageData[4] = { 0xDE, 0xAD, 0xBE, 0xEF };
        ImageView initialImage;
        {
            initialImage.data       = initialImageData;
            initialImage.dataSize   = sizeof(initialImageData);
        }
        TextureDescriptor texDesc;
        {
            texDesc.debugName   = "CommandBufferSubmit.Framebuffer";
            texDesc.bindFlags   = BindFlags::Sampled | BindFlags::CopyDst;
            texDesc.format      = swapChain->GetColorFormat();
        }
        framebufferResultTex = renderer->CreateTexture(texDesc, &initialImage);

        // Create multi-submit command buffers
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
                cmdBuf->CopyTextureFromFramebuffer(*framebufferResultTex, texRegion, Offset2D{ 0, 0 });
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

        // Read framebuffer pixel value from intermediate texture
        std::uint8_t framebufferResult[4] = {};
        MutableImageView framebufferResultDesc;
        {
            framebufferResultDesc.data      = framebufferResult;
            framebufferResultDesc.dataSize  = sizeof(framebufferResult);
        }
        renderer->ReadTexture(*framebufferResultTex, texRegion, framebufferResultDesc);

        const std::uint8_t expectedResult[4] =
        {
            static_cast<std::uint8_t>(clearValues[swapBufferIndex].color[0] * 255.0f),
            static_cast<std::uint8_t>(clearValues[swapBufferIndex].color[1] * 255.0f),
            static_cast<std::uint8_t>(clearValues[swapBufferIndex].color[2] * 255.0f),
            static_cast<std::uint8_t>(clearValues[swapBufferIndex].color[3] * 255.0f),
        };

        if (::memcmp(framebufferResult, expectedResult, sizeof(framebufferResult)) != 0)
        {
            Log::Errorf(
                "Mismatch between framebuffer[%u] color [%02X %02X %02X %02X] and clear value [%02X %02X %02X %02X]\n",
                swapBufferIndex,
                framebufferResult[0], framebufferResult[1], framebufferResult[2], framebufferResult[3],
                expectedResult[0], expectedResult[1], expectedResult[2], expectedResult[3]
            );
            return TestResult::FailedMismatch;
        }

        return TestResult::Continue;
    }
    else
    {
        // Delete old command buffers
        for (auto* cmdBuf : multiSubmitCmdBuffers)
            renderer->Release(*cmdBuf);
        renderer->Release(*framebufferResultTex);

        return TestResult::Passed;
    }
}


