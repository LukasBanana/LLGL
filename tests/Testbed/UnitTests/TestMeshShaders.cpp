/*
 * TestMeshShaders.cpp
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
Renders a simple triangle span with a mesh PSO. No vertex buffer input.
*/
DEF_TEST( MeshShaders )
{
    static TestResult result = TestResult::Passed;
    static PipelineLayout* psoLayout;
    static PipelineState* pso;

    // Ignore for backends that don't support tier-1 command buffers
    if (!caps.features.hasMeshShaders)
        return TestResult::Skipped;

    CommandBufferTier1* cmdBufferTier1 = CastTo<CommandBufferTier1>(cmdBuffer);

    if (frame == 0)
    {
        if (cmdBufferTier1 == nullptr)
        {
            Log::Errorf("Missing tier-1 command buffer for backend\n");
            return TestResult::FailedErrors;
        }
        if (shaders[MSMeshlet] == nullptr || shaders[PSMeshlet] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        psoLayout = renderer->CreatePipelineLayout(LLGL::Parse("float(aspectRatio),uint(numMeshlets)"));

        MeshPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout  = psoLayout;
            psoDesc.renderPass      = swapChain->GetRenderPass();
            psoDesc.meshShader      = shaders[MSMeshlet];
            psoDesc.fragmentShader  = shaders[PSMeshlet];
        }
        CREATE_MESH_PSO_EXT(pso, psoDesc, "psoMeshShaders");
    }

    // Render scene
    constexpr unsigned numFrames = 3;

    Texture* readbackTex = nullptr;

    const float aspectRatio = 1.0f / GetAspectRatio();
    const std::uint32_t numMeshlets = frame + 1;

    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::ColorDepth);
            cmdBuffer->SetViewport(opt.resolution);

            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetUniforms(0, &aspectRatio, sizeof(aspectRatio));
            cmdBuffer->SetUniforms(1, &numMeshlets, sizeof(numMeshlets));

            cmdBufferTier1->DrawMesh(numMeshlets, 1, 1);

            // Capture framebuffer in last iteration
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    END();

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "MeshShaders_Frame" + std::to_string(frame);

    SaveCapture(readbackTex, colorBufferName);
    const DiffResult diff = DiffImages(colorBufferName);

    // Evaluate readback result
    TestResult intermediateResult = diff.Evaluate("mesh shaders", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    renderer->Release(*psoLayout);
    renderer->Release(*pso);

    return result;
}

