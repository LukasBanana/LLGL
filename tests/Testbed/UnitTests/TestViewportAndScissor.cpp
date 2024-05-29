/*
 * TestViewportAndScissor.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


DEF_TEST( ViewportAndScissor )
{
    if (shaders[VSSolid] == nullptr || shaders[PSSolid] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Test data
    const Scissor scissor0{ Offset2D{ 100, 150 }, Extent2D{ 500, 300 } };
    const Viewport viewport0{ 50.0f, 130.0f, 600.0f, 280.0f };

    const Scissor lowerBoundScissor // this scissor rectangle is reduced by the viewport
    {
        std::max(scissor0.x, static_cast<std::int32_t>(viewport0.x + 0.5f)),
        std::max(scissor0.y, static_cast<std::int32_t>(viewport0.y + 0.5f)),
        std::max(0, std::min(scissor0.x + scissor0.width,  static_cast<std::int32_t>(viewport0.x + viewport0.width  + 0.5f)) - scissor0.x),
        std::max(0, std::min(scissor0.y + scissor0.height, static_cast<std::int32_t>(viewport0.y + viewport0.height + 0.5f)) - scissor0.y)
    };

    const Gs::Vector4f colors[] =
    {
        Gs::Vector4f{ 0.6f, 0.2f, 0.2f, 1.0f }, // red
        Gs::Vector4f{ 0.2f, 0.6f, 0.2f, 1.0f }, // green
        Gs::Vector4f{ 0.6f, 0.2f, 0.7f, 1.0f }, // pink
        Gs::Vector4f{ 0.2f, 0.2f, 0.6f, 1.0f }, // blue
        Gs::Vector4f{ 0.7f, 0.8f, 0.2f, 1.0f }, // yellow
    };

    // Create graphics PSOs with and without scissor tests
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout                  = layouts[PipelineSolid];
        psoDesc.renderPass                      = swapChain->GetRenderPass();
        psoDesc.vertexShader                    = shaders[VSSolid];
        psoDesc.fragmentShader                  = shaders[PSSolid];
        psoDesc.depth.testEnabled               = true;
        psoDesc.depth.writeEnabled              = true;
        psoDesc.rasterizer.cullMode             = CullMode::Back;
    }
    CREATE_GRAPHICS_PSO(psoNoScissor, psoDesc, "psoNoScissor");
    {
        psoDesc.rasterizer.scissorTestEnabled   = true;
    }
    CREATE_GRAPHICS_PSO(psoDynamicScissor, psoDesc, "psoDynamicScissor");
    {
        psoDesc.viewports.push_back(viewport0);
        psoDesc.scissors.push_back(scissor0);
    }
    CREATE_GRAPHICS_PSO(psoStaticScissor, psoDesc, "psoStaticScissor");

    PipelineState* psoList[] = { psoNoScissor, psoStaticScissor, psoNoScissor, psoDynamicScissor, psoNoScissor };
    constexpr auto psoCount = sizeof(psoList)/sizeof(psoList[0]);

    for (PipelineState* pso : psoList)
    {
        if (const Report* report = pso->GetReport())
        {
            if (report->HasErrors())
            {
                Log::Errorf("PSO creation failed:\n%s", report->GetText());
                return TestResult::FailedErrors;
            }
        }
    }

    // Update scene constants
    sceneConstants = SceneConstants{};

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -2 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    sceneConstants.wMatrix.LoadIdentity();
    Gs::Scale(sceneConstants.wMatrix, Gs::Vector3f{ 10, 10, 1 });

    // Render scene
    Texture* readbackTex[psoCount] = {};

    const IndexedTriangleMesh& mesh = models[ModelRect];

    cmdBuffer->Begin();
    {
        // Graphics can be set inside and outside a render pass, so test binding this PSO outside the render pass
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);

        for_range(i, psoCount)
        {
            cmdBuffer->SetPipelineState(*psoList[i]);

            sceneConstants.solidColor = colors[i];
            cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));

            cmdBuffer->BeginRenderPass(*swapChain);
            {
                if (psoList[i] == psoDynamicScissor)
                {
                    cmdBuffer->SetViewport(viewport0);
                    cmdBuffer->SetScissor(scissor0);
                }
                else if (psoList[i] == psoNoScissor)
                    cmdBuffer->SetViewport(swapChain->GetResolution());

                // Draw scene
                cmdBuffer->Clear(ClearFlags::ColorDepth);
                cmdBuffer->SetResource(0, *sceneCbuffer);
                cmdBuffer->DrawIndexed(mesh.numIndices, 0);

                // Capture framebuffer
                readbackTex[i] = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
            }
            cmdBuffer->EndRenderPass();
        }
    }
    cmdBuffer->End();

    auto EvaluateCapturePoint = [this](Texture* capture, std::int32_t x, std::int32_t y, const Gs::Vector4f& expectedColor, const char* colorBufferName) -> TestResult
    {
        // Read texture at pixel location
        float actualColor[4] = { -1.0f, -1.0f, -1.0f, -1.0f };
        MutableImageView dstImageView;
        {
            dstImageView.format     = ImageFormat::RGBA;
            dstImageView.dataType   = DataType::Float32;
            dstImageView.data       = actualColor;
            dstImageView.dataSize   = sizeof(actualColor);
        }
        renderer->ReadTexture(*capture, TextureRegion{ Offset3D{ x, y, 0 }, Extent3D{ 1, 1, 1 } }, dstImageView);

        // Compare to expected pixel color
        constexpr float tolerance = 0.01f;
        if (std::abs(actualColor[0] - expectedColor[0]) > tolerance ||
            std::abs(actualColor[1] - expectedColor[1]) > tolerance ||
            std::abs(actualColor[2] - expectedColor[2]) > tolerance ||
            std::abs(actualColor[3] - expectedColor[3]) > tolerance)
        {
            Log::Errorf(
                "Mismatch in %s at location (%d, %d):\n"
                " => expected color (%f, %f, %f, %f)\n"
                " => actual color   (%f, %f, %f, %f)\n",
                colorBufferName, x, y,
                expectedColor[0], expectedColor[1], expectedColor[2], expectedColor[3],
                actualColor[0], actualColor[1], actualColor[2], actualColor[3]
            );
            return TestResult::FailedMismatch;
        }

        return TestResult::Passed;
    };

    auto EvaluateAllFixedCapturePoints = [this, &EvaluateCapturePoint, &lowerBoundScissor](Texture* capture, bool wasScissorEnabled, const Gs::Vector4f& solidColor, const char* colorBufferName) -> TestResult
    {
        struct SampleConfig
        {
            bool            inside; // Is sample location inside scissor rectangle?
            std::int32_t    posX;
            std::int32_t    posY;
        };

        const Scissor& rect = lowerBoundScissor;

        const SampleConfig samples[] =
        {
            SampleConfig{ false, rect.x - 1, rect.y     },
            SampleConfig{ false, rect.x    , rect.y - 1 },
            SampleConfig{ true,  rect.x    , rect.y     }, // inside left-top corner

            SampleConfig{ false, rect.x + rect.width - 1, rect.y + rect.height     },
            SampleConfig{ false, rect.x + rect.width    , rect.y + rect.height - 1 },
            SampleConfig{ true,  rect.x + rect.width - 1, rect.y + rect.height - 1 }, // inside right-bottom corner
        };

        for (const SampleConfig& sample : samples)
        {
            const bool isSampleInsideScissor = (sample.inside || !wasScissorEnabled);
            const Gs::Vector4f expectedColor = (isSampleInsideScissor ? solidColor : Gs::Vector4f{ 0.0f, 0.0f, 0.0f, 0.0f });
            TestResult result = EvaluateCapturePoint(capture, sample.posX, sample.posY, expectedColor, colorBufferName);
            if (result != TestResult::Passed)
                return result;
        }

        return TestResult::Passed;
    };

    // Match entire color buffer and create delta heat map
    TestResult result = TestResult::Passed;

    const char* frameNames[] =
    {
        "0_Default", "1_Static", "2_Default", "3_Dynamic", "4_Default"
    };

    for_range(i, psoCount)
    {
        const std::string colorBufferName = "ViewportAndScissor_" + std::string(frameNames[i]);

        // Evaluate at fixed points
        const bool wasScissorEnabled = (psoList[i] != psoNoScissor);
        TestResult intermediateResult = EvaluateAllFixedCapturePoints(readbackTex[i], wasScissorEnabled, colors[i], colorBufferName.c_str());
        if (intermediateResult != TestResult::Passed)
        {
            result = intermediateResult;
            if (!opt.greedy)
                break;
        }

        // Save capture
        SaveCapture(readbackTex[i], colorBufferName);

        const DiffResult diff = DiffImages(colorBufferName);

        // Evaluate readback result
        intermediateResult = diff.Evaluate("viewport and scissor", frame);
        if (intermediateResult != TestResult::Passed)
            result = intermediateResult;
    }

    // Clear resources
    renderer->Release(*psoNoScissor);
    renderer->Release(*psoDynamicScissor);
    renderer->Release(*psoStaticScissor);

    return result;
}

