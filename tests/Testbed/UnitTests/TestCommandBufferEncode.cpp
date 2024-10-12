/*
 * TestCommandBufferEncode.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>


DEF_TEST( CommandBufferEncode )
{
    if (shaders[VSClear] == nullptr || shaders[PSClear] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    enum TestCmdBuffers
    {
        TestCmdBufferPrimary0,
        TestCmdBufferPrimary1,

        TestCmdBufferSecondary0,
        TestCmdBufferSecondary1,

        TestCmdBuffer_Count,
    };

    static CommandBuffer* cmdBuffers[TestCmdBuffer_Count] = {};
    static Texture* framebufferResultTex[2] = {};
    static PipelineLayout* psoLayout;
    static PipelineState* pso;

    const ClearValue clearValues[] =
    {
        ClearValue{ 0.7f, 0.1f, 0.6f, 1 }, // Magenta
        ClearValue{ 0.9f, 0.8f, 0.1f, 1 }  // Yellow
    };

    const TextureRegion texRegion{ Offset3D{ 0, 0, 0 }, Extent3D{ 1, 1, 1 } };

    constexpr unsigned numPrimaryEncodings      = 9;
    constexpr unsigned numSecondaryEncodings    = 4;

    auto EncodeSecondaryCommandBuffer = [this](CommandBuffer& cmdBuf, const ClearValue& clearValue) -> void
    {
        // Draw a fullscreen triangle to clear the screen without using the Clear() function as this is only supported on a primary command buffer
        cmdBuf.Begin();
        {
            cmdBuf.SetPipelineState(*pso);
            cmdBuf.SetVertexBuffer(*this->meshBuffer);
            cmdBuf.SetUniforms(0, clearValue.color, sizeof(clearValue.color));
            cmdBuf.Draw(3, 0);
        }
        cmdBuf.End();
    };

    auto EncodePrimaryCommandBuffer = [this, &EncodeSecondaryCommandBuffer, numSecondaryEncodings, &clearValues, frame, &texRegion](
        CommandBuffer& cmdBuf, CommandBuffer& secondaryCmdBuf, unsigned cmdBufferIndex) -> void
    {
        if (opt.verbose)
            Log::Printf("Encoding primary command buffer [%u] in frame [%u] with secondary command buffers [", cmdBufferIndex, frame);

        // Encode secondary command buffer redundantly several times
        for_range(i, numSecondaryEncodings)
        {
            if (opt.verbose)
                Log::Printf("%u%s", i, (i + 1 == numSecondaryEncodings ? "]" : ", "));

            EncodeSecondaryCommandBuffer(secondaryCmdBuf, clearValues[cmdBufferIndex]);
        }

        if (opt.verbose)
            Log::Printf("\n");

        // Encode primary command buffer that executes the secondary command buffer and copies the result into the feedback texture
        cmdBuf.Begin();
        {
            cmdBuf.BeginRenderPass(*swapChain);
            {
                cmdBuf.SetViewport(swapChain->GetResolution());
                cmdBuf.Execute(secondaryCmdBuf);
                cmdBuf.CopyTextureFromFramebuffer(*framebufferResultTex[cmdBufferIndex], texRegion, Offset2D{ 0, 0 });
            }
            cmdBuf.EndRenderPass();
        }
        cmdBuf.End();
    };

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
            texDesc.bindFlags   = BindFlags::Sampled | BindFlags::CopyDst;
            texDesc.format      = swapChain->GetColorFormat();
        }
        framebufferResultTex[0] = renderer->CreateTexture(texDesc, &initialImage);
        framebufferResultTex[1] = renderer->CreateTexture(texDesc, &initialImage);

        // Create multi-submit command buffers
        for_range(i, TestCmdBuffer_Count)
        {
            CommandBufferDescriptor cmdBufferDesc;
            {
                cmdBufferDesc.flags             = (i >= TestCmdBufferSecondary0 ? CommandBufferFlags::Secondary : 0);
                cmdBufferDesc.numNativeBuffers  = (i % 2 == 0 ? 3 : 1);
                cmdBufferDesc.renderPass        = (i >= TestCmdBufferSecondary0 ? swapChain->GetRenderPass() : nullptr);
            }
            cmdBuffers[i] = renderer->CreateCommandBuffer(cmdBufferDesc);
        }

        // Create graphics PSO to draw a single pixel
        psoLayout = renderer->CreatePipelineLayout(Parse("float4(clearColor)"));

        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout      = psoLayout;
            psoDesc.renderPass          = swapChain->GetRenderPass();
            psoDesc.vertexShader        = shaders[VSClear];
            psoDesc.fragmentShader      = shaders[PSClear];
            //psoDesc.primitiveTopology   = PrimitiveTopology::PointList;
        }
        CREATE_GRAPHICS_PSO_EXT(pso, psoDesc, "psoCmdBufEncode");
    }

    // Re-record command buffers several times until we're ready to submit
    EncodePrimaryCommandBuffer(*cmdBuffers[TestCmdBufferPrimary0], *cmdBuffers[TestCmdBufferSecondary0], 0);
    EncodePrimaryCommandBuffer(*cmdBuffers[TestCmdBufferPrimary1], *cmdBuffers[TestCmdBufferSecondary1], 1);

    if (frame < numPrimaryEncodings)
        return TestResult::Continue;

    // Submit command buffers
    cmdQueue->Submit(*cmdBuffers[TestCmdBufferPrimary0]);
    cmdQueue->Submit(*cmdBuffers[TestCmdBufferPrimary1]);

    // Read framebuffer pixel value from intermediate texture
    for_range(i, 2)
    {
        std::uint8_t framebufferResult[4] = {};
        MutableImageView framebufferResultDesc;
        {
            framebufferResultDesc.data      = framebufferResult;
            framebufferResultDesc.dataSize  = sizeof(framebufferResult);
        }
        renderer->ReadTexture(*framebufferResultTex[i], texRegion, framebufferResultDesc);

        const std::uint8_t expectedResult[4] =
        {
            static_cast<std::uint8_t>(clearValues[i].color[0] * 255.0f),
            static_cast<std::uint8_t>(clearValues[i].color[1] * 255.0f),
            static_cast<std::uint8_t>(clearValues[i].color[2] * 255.0f),
            static_cast<std::uint8_t>(clearValues[i].color[3] * 255.0f),
        };

        if (::memcmp(framebufferResult, expectedResult, sizeof(framebufferResult)) != 0)
        {
            if (TestbedContext::IsRGBA8ubInThreshold(framebufferResult, expectedResult))
            {
                if (opt.verbose)
                {
                    Log::Printf(
                        "Negligible mismatch between framebuffer[%u] color [%02X %02X %02X %02X] and clear value [%02X %02X %02X %02X] within threshold\n",
                        i,
                        framebufferResult[0], framebufferResult[1], framebufferResult[2], framebufferResult[3],
                        expectedResult[0], expectedResult[1], expectedResult[2], expectedResult[3]
                    );
                }
            }
            else
            {
                Log::Errorf(
                    "Mismatch between framebuffer[%u] color [%02X %02X %02X %02X] and clear value [%02X %02X %02X %02X]\n",
                    i,
                    framebufferResult[0], framebufferResult[1], framebufferResult[2], framebufferResult[3],
                    expectedResult[0], expectedResult[1], expectedResult[2], expectedResult[3]
                );
                return TestResult::FailedMismatch;
            }
        }
    }

    // Delete old command buffers
    for (auto* cmdBuf : cmdBuffers)
        renderer->Release(*cmdBuf);
    for (auto* tex : framebufferResultTex)
        renderer->Release(*tex);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    return TestResult::Passed;
}


