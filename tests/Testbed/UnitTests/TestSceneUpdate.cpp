/*
 * TestSceneUpdate.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


/*
Renders a scene (segmented cube) with various different rotations.
The primary command buffer is encoded in several iterations and immediately submitted to the command queue.
Only the last iteration takes a framebuffer capture to ensure the buffer updates are encoded correctly
and not erroneously overridden by faulty CPU/GPU synchronization.
*/
DEF_TEST( SceneUpdate )
{
    static TestResult result = TestResult::Passed;
    static PipelineState* pso;

    if (frame == 0)
    {
        if (shaders[VSSolid] == nullptr || shaders[PSSolid] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout      = layouts[PipelineSolid];
            psoDesc.renderPass          = swapChain->GetRenderPass();
            psoDesc.vertexShader        = shaders[VSSolid];
            psoDesc.fragmentShader      = shaders[PSSolid];
            psoDesc.depth.testEnabled   = true;
            psoDesc.depth.writeEnabled  = true;
            psoDesc.rasterizer.cullMode = CullMode::Back;
        }
        CREATE_GRAPHICS_PSO_EXT(pso, psoDesc, "psoSceneUpdate");
    }

    // Skip every other frame on fast test
    if (opt.fastTest && (frame % 2 == 0))
        return TestResult::ContinueSkipFrame;

    // Update scene constants
    sceneConstants = SceneConstants{};

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    auto TransformWorldMatrix = [](Gs::Matrix4f& wMatrix, float pos, float scale, float turn)
    {
        wMatrix.LoadIdentity();
        Gs::Translate(wMatrix, Gs::Vector3f{ 0, pos, 2.0f });
        Gs::RotateFree(wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(turn));
        Gs::Scale(wMatrix, Gs::Vector3f{ 1, scale, 1 });
    };

    // Render scene
    constexpr unsigned numFrames = 10;
    constexpr unsigned numSceneIterations = 3;

    Texture* readbackTex = nullptr;

    const IndexedTriangleMesh& mesh = models[ModelCube];

    const float semiRandomRotations[2] = { -10.0f, -5.0f };

    // Render the scene several times before taking the frame capture for comparison.
    // This ensures that the buffer updates (sceneCbuffer) are encoded correctly and CPU/GPU synchronization works as intended.
    for_range(i, numSceneIterations)
    {
        const bool isLastIteration = (i + 1 == for_range_end(i));

        // First render the object in a semi-random rotation and use the frame dependent rotation in the last iteration
        const float rotation =
        (
            isLastIteration
                ? static_cast<float>(frame) * 90.0f / static_cast<float>(numFrames - 1)
                : semiRandomRotations[i % (sizeof(semiRandomRotations)/sizeof(semiRandomRotations[0]))]
        );

        cmdBuffer->Begin();
        {
            // Graphics can be set inside and outside a render pass, so test binding this PSO outside the render pass
            cmdBuffer->SetVertexBuffer(*meshBuffer);
            cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
            cmdBuffer->SetPipelineState(*pso);

            cmdBuffer->BeginRenderPass(*swapChain);
            {
                // Draw scene
                cmdBuffer->Clear(ClearFlags::ColorDepth);
                cmdBuffer->SetViewport(opt.resolution);
                cmdBuffer->SetResource(0, *sceneCbuffer);

                // Draw top part
                sceneConstants.solidColor = { 1.0f, 0.7f, 0.6f, 1.0f }; // red
                TransformWorldMatrix(sceneConstants.wMatrix, 0.5f, 0.5f, rotation);
                cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
                cmdBuffer->DrawIndexed(mesh.numIndices, 0);

                // Draw middle part
                sceneConstants.solidColor = { 0.5f, 1.0f, 0.4f, 1.0f }; // green
                TransformWorldMatrix(sceneConstants.wMatrix, -0.25f, 0.25f, rotation);
                cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
                cmdBuffer->DrawIndexed(mesh.numIndices, 0);

                // Draw bottom part
                sceneConstants.solidColor = { 0.3f, 0.7f, 1.0f, 1.0f }; // blue
                TransformWorldMatrix(sceneConstants.wMatrix, -0.75f, 0.25f, rotation);
                cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
                cmdBuffer->DrawIndexed(mesh.numIndices, 0);

                // Capture framebuffer in last iteration
                if (isLastIteration)
                    readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
            }
            cmdBuffer->EndRenderPass();
        }
        cmdBuffer->End();
    }

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "SceneUpdate_Frame" + std::to_string(frame);

    SaveCapture(readbackTex, colorBufferName);

    const DiffResult diff = DiffImages(colorBufferName);

    // Evaluate readback result
    TestResult intermediateResult = diff.Evaluate("scene update", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    renderer->Release(*pso);

    return result;
}

