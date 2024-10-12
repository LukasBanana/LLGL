/*
 * TestResourceCopy.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Scale.h>


/*
Copy buffer and textures resources forth and back. This is a combination of BufferToTextureCopy, TextureToBufferCopy, and TextureCopy tests.
This test only uses a single texture format. The aforementioned tests validate various different texture formats.
The last two frames (8 and 9) are expected to produce the same output.
*/
DEF_TEST( ResourceCopy )
{
    static TestResult       result              = TestResult::Passed;

    static PipelineState*   pso                 = nullptr;
    static Buffer*          contentBuffer       = nullptr;          // Content buffer which is copied into the textures
    static Texture*         dstTextures[2]      = {};               // Destination textures for display
    static int              dstTextureIndex     = 0;

    const std::uint64_t     contentBufferSize   = 4 * 512;          // Format = RGBA8UNorm
    const Extent3D          dstTextureSize      = { 64, 64, 1 };
  //const Extent3D          srcTexture0Size     = { 64, 64, 1 };    // 64 * 4 = 256 = Proper row alignment (especially for D3D12)
  //const Extent3D          srcTexture1Size     = { 50, 20, 1 };    // 50 * 4 = 200 = Improper row alignment

    constexpr unsigned maxNumFrames = 10;

    const LLGL::ColorRGBub expectedSrcColor0[maxNumFrames] =
    {
        LLGL::ColorRGBub{ 0xFF, 0xFF, 0xFF },
        LLGL::ColorRGBub{ 0x40, 0xD0, 0x50 },
        LLGL::ColorRGBub{ 0x50, 0x50, 0xD0 },
        LLGL::ColorRGBub{ 0x50, 0x50, 0xD0 },
        LLGL::ColorRGBub{ 0xD0, 0x50, 0x20 },
        LLGL::ColorRGBub{ 0x40, 0xD0, 0x50 },
        LLGL::ColorRGBub{ 0x50, 0x50, 0xD0 },
        LLGL::ColorRGBub{ 0x50, 0x50, 0xD0 },
        LLGL::ColorRGBub{ 0xD0, 0x50, 0x20 },
        LLGL::ColorRGBub{ 0xD0, 0x50, 0x20 },
    };

    auto ReleaseResources = [&]() -> void
    {
        renderer->Release(*pso);
        renderer->Release(*contentBuffer);
        renderer->Release(*dstTextures[0]);
        renderer->Release(*dstTextures[1]);
    };

    auto GenerateTextureContent = [&]() -> void
    {
        // Map content buffer for writing
        if (void* dst = renderer->MapBuffer(*contentBuffer, LLGL::CPUAccess::WriteDiscard))
        {
            // Write some initial data
            auto dstColors = reinterpret_cast<LLGL::ColorRGBAub*>(dst);
            for (int i = 0; i < 128; ++i)
            {
                dstColors[i] = LLGL::ColorRGBAub{ 0xD0, 0x50, 0x20, 0xFF }; // Red
            }
            renderer->UnmapBuffer(*contentBuffer);
        }

        // Encode copy commands
        cmdBuffer->Begin();
        {
            // Fill up content buffer (Note: swap endian)
            cmdBuffer->FillBuffer(*contentBuffer, /*Offset:*/ 128 * 4, /*Value:*/ 0xFF50D040, /*Size:*/ 128 * 4); // Green
            cmdBuffer->FillBuffer(*contentBuffer, /*Offset:*/ 256 * 4, /*Value:*/ 0xFFD05050, /*Size:*/ 256 * 4); // Blue

            // Copy content buffer to destination texture
            for (int y = 0; y < static_cast<int>(dstTextureSize.height); y += 8)
            {
                cmdBuffer->CopyTextureFromBuffer(
                    *dstTextures[0],
                    LLGL::TextureRegion
                    {
                        LLGL::Offset3D{ 0, y, 0 },
                        LLGL::Extent3D{ 64, 8, 1 }
                    },
                    *contentBuffer,
                    0
                );
            }

            // Duplicate destination texture context
            cmdBuffer->CopyTexture(*dstTextures[1], {}, *dstTextures[0], {}, dstTextureSize);
        }
        cmdBuffer->End();
    };

    auto ModifyTextureContent = [&](LLGL::ColorRGBAub& outSrcColor0) -> void
    {
        // Encode copy commands
        cmdBuffer->Begin();
        {
            // Modify texture by copying data between the two alternating destination textures
            cmdBuffer->CopyTexture(
                /*Destination:*/            *dstTextures[(dstTextureIndex + 1) % 2],
                /*DestinationLocation:*/    LLGL::TextureLocation{ LLGL::Offset3D{ 8, 8, 0 } },
                /*Source:*/                 *dstTextures[dstTextureIndex],
                /*SourceLocation:*/         LLGL::TextureLocation{ LLGL::Offset3D{ 12, 10, 0 } },
                /*Size:*/                   LLGL::Extent3D{ 32, 32, 1 }
            );

            // Store single pixel of texture back to content buffer to map texture memory to CPU space
            cmdBuffer->CopyBufferFromTexture(
                *contentBuffer,
                0,
                *dstTextures[(dstTextureIndex + 1) % 2],
                LLGL::TextureRegion
                {
                    LLGL::Offset3D{ 8, 8, 0 },
                    LLGL::Extent3D{ 1, 1, 1 }
                }
            );
        }
        cmdBuffer->End();

        // Map content buffer for reading
        if (const void* src = renderer->MapBuffer(*contentBuffer, LLGL::CPUAccess::ReadOnly))
        {
            outSrcColor0 = reinterpret_cast<const LLGL::ColorRGBAub*>(src)[0];
            renderer->UnmapBuffer(*contentBuffer);
        }

        // Move to next destination texture for display
        dstTextureIndex = (dstTextureIndex + 1) % 2;
    };

    if (frame == 0)
    {
        result = TestResult::Passed;

        if (shaders[VSTextured] == nullptr || shaders[PSTextured] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        // Create graphics PSO
        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.debugName       = "Test.ResourceCopy.PSO";
            psoDesc.pipelineLayout  = layouts[PipelineTextured];
            psoDesc.renderPass      = swapChain->GetRenderPass();
            psoDesc.vertexShader    = shaders[VSTextured];
            psoDesc.fragmentShader  = shaders[PSTextured];
        }
        CREATE_GRAPHICS_PSO_EXT(pso, psoDesc, nullptr);

        // Create content buffer with CPU read/write access but without binding flags since we don't bind it to any pipeline
        BufferDescriptor bufferDesc;
        {
            bufferDesc.debugName        = "Test.ResourceCopy.ContentBuffer";
            bufferDesc.size             = contentBufferSize;
            bufferDesc.bindFlags        = LLGL::BindFlags::CopySrc | LLGL::BindFlags::CopyDst; // Not used in a graphics or compute shader, only with copy commands
            bufferDesc.cpuAccessFlags   = LLGL::CPUAccessFlags::ReadWrite;
            bufferDesc.miscFlags        = LLGL::MiscFlags::NoInitialData;
        }
        contentBuffer = renderer->CreateBuffer(bufferDesc);

        // Create empty destination texture
        LLGL::TextureDescriptor texDesc;
        {
            texDesc.debugName   = "Test.ResourceCopy.DstTex0";
            texDesc.bindFlags   = LLGL::BindFlags::Sampled | LLGL::BindFlags::ColorAttachment | LLGL::BindFlags::CopyDst | LLGL::BindFlags::CopySrc;
            texDesc.miscFlags   = LLGL::MiscFlags::NoInitialData;
            texDesc.extent      = dstTextureSize;
        }
        dstTextures[0] = renderer->CreateTexture(texDesc);
        {
            texDesc.debugName   = "Test.ResourceCopy.DstTex1";
        }
        dstTextures[1] = renderer->CreateTexture(texDesc);

        // Initialize texture content
        GenerateTextureContent();
    }

    // Modify texture content each frame (after first one)
    LLGL::ColorRGBAub srcColor0;
    if (frame > 0)
        ModifyTextureContent(srcColor0);

    if (opt.sanityCheck)
        Log::Printf("SrcColor0 result: [%02X %02X %02X %02X]\n", srcColor0.r, srcColor0.g, srcColor0.b, srcColor0.a);

    if (srcColor0.r != expectedSrcColor0[frame].r ||
        srcColor0.g != expectedSrcColor0[frame].g ||
        srcColor0.b != expectedSrcColor0[frame].b ||
        srcColor0.a != 0xFF)
    {
        // Test failed: mismatch between source colors for specific pixel location
        Log::Errorf(
            "Mismatch between color at reference point (Frame %u):\n"
            " -> Expected: [%02X %02X %02X %02X]\n"
            " -> Actual:   [%02X %02X %02X %02X]\n",
            frame,
            expectedSrcColor0[frame].r, expectedSrcColor0[frame].g, expectedSrcColor0[frame].b, 0xFF,
            srcColor0.r, srcColor0.g, srcColor0.b, srcColor0.a
        );

        result = TestResult::FailedMismatch;
        if (!opt.greedy)
        {
            ReleaseResources();
            return result;
        }
    }

    // Each test must be consecutive here, so '--fast' option skips the remaining 5 tests instead of every other test
    const unsigned numFrames = (opt.fastTest ? 5 : maxNumFrames);

    // Initialize scene constants
    sceneConstants = SceneConstants{};

    const Extent2D resolution = swapChain->GetResolution();
    const float rectSize[2] = { 512.0f, 512.0f }; // must be multiple of 'dstTextureSize', otherwise one-pixel offset can occur depending on hardware

    Gs::Scale(sceneConstants.vpMatrix, Gs::Vector3f{ rectSize[0] / static_cast<float>(resolution.width), rectSize[1] / static_cast<float>(resolution.height), 1.0f });

    // Render scene
    Texture* readbackTex = nullptr;

    const IndexedTriangleMesh& mesh = models[ModelRect];

    cmdBuffer->Begin();
    {
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);

        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetViewport(swapChain->GetResolution());

            // Draw scene
            cmdBuffer->Clear(ClearFlags::ColorDepth);
            cmdBuffer->SetResource(0, *sceneCbuffer);
            cmdBuffer->SetResource(1, *dstTextures[dstTextureIndex]);
            cmdBuffer->SetResource(2, *samplers[SamplerNearestClamp]);
            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "ResourceCopy_Frame" + std::to_string(frame);

    SaveCapture(readbackTex, colorBufferName);

    const DiffResult diff = DiffImages(colorBufferName);

    // Evaluate readback result and tolerate 5 pixel that are beyond the threshold due to GPU differences with the reinterpretation of pixel formats
    TestResult intermediateResult = diff.Evaluate("uniforms", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    ReleaseResources();

    return result;
}

