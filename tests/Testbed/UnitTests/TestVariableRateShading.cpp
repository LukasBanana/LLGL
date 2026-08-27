/*
 * TestShadowMapping.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ColorRGB.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>
#include <Gauss/ProjectionMatrix4.h>


// Renders textured geometry with different shading rates whereas the last one has shading rates disabled, to ensure the PSO automatically resets the shading rate.
// The API states that SetShadingRate() must only be called with bound PSOs that have shading rate enabled via `RasterizerDescriptor::shadingRateEnabled`.
DEF_TEST( VariableRateShading )
{
    if (!caps.features.hasVariableRateShading)
    {
        if (opt.verbose)
            Log::Printf("Variable rate shading not supported by renderer backend -> skip test\n");
        return TestResult::Skipped;
    }

    CommandBufferTier1* cmdBufferTier1 = CastTo<CommandBufferTier1>(cmdBuffer);
    if (cmdBufferTier1 == nullptr)
    {
        Log::Printf(Log::ColorFlags::StdError, "Variable rate shading is reported as supported but CommandBufferTier1 is not available!");
        return TestResult::FailedErrors;
    }

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = layouts[PipelineTextured];
        psoDesc.inputVertexAttribs  = vertexFormats[VertFmtStd].attributes;
        psoDesc.vertexShader        = shaders[VSTextured];
        psoDesc.fragmentShader      = shaders[PSTextured];
        psoDesc.depth.testEnabled   = true;
        psoDesc.depth.writeEnabled  = true;
        psoDesc.rasterizer.cullMode = CullMode::Back;
    }
    CREATE_GRAPHICS_PSO(psoVRSDisabled, psoDesc, "pso(VRS=OFF)");
    {
        psoDesc.rasterizer.shadingRateEnabled = true;
    }
    CREATE_GRAPHICS_PSO(psoVRSEnabled, psoDesc, "pso(VRS=ON)");

    // Update scene constants
    sceneConstants = {};

    sceneConstants.vpMatrix = projection;

    sceneConstants.wMatrix.LoadIdentity();
    Gs::Translate(sceneConstants.wMatrix, Gs::Vector3f{ 0, 0, 2.5f });
    Gs::RotateFree(sceneConstants.wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(25.0f));
    Gs::Scale(sceneConstants.wMatrix, Gs::Vector3f{ 0.5f });

    // Helper functions for drawing the scene
    auto BindPipeline = [this](PipelineState* pso)
    {
        cmdBuffer->SetPipelineState(*pso);
        cmdBuffer->SetResource(0, *sceneCbuffer);
        cmdBuffer->SetResource(1, *textures[TexturePaintingB]);
        cmdBuffer->SetResource(2, *samplers[SamplerLinear]);
    };

    auto DrawTriangleMesh = [this](const IndexedTriangleMesh& mesh)
    {
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
        cmdBuffer->DrawIndexed(mesh.numIndices, 0);
    };

    auto DrawScene = [this, &DrawTriangleMesh, cmdBufferTier1](
        const Viewport& viewport, bool isVRSEnabled = false, ShadingRate shadingRate = ShadingRate::Size1x1)
    {
        // Only call SetShadingRate() if the current PSO enbales VRS. Otherwise, the PSO needs to make sure it's disabled.
        if (isVRSEnabled)
            cmdBufferTier1->SetShadingRate(shadingRate);

        cmdBuffer->SetViewport(viewport);

        DrawTriangleMesh(models[ModelCube]);
    };

    // Render scene
    Texture* readbackTex = nullptr;

    const Extent2D      halfRes = { opt.resolution.width/2, opt.resolution.height/2 };
    const std::int32_t  halfResX = static_cast<std::int32_t>(halfRes.width);
    const std::int32_t  halfResY = static_cast<std::int32_t>(halfRes.height);

    const ShadingRate shadingRatesToTest[3] =
    {
        ShadingRate::Size2x2,
        ShadingRate::Size4x2,
        ShadingRate::Size4x4
    };

    const Viewport viewports[4] =
    {
        Viewport{ Offset2D{        0,        0 }, halfRes }, // Left-top
        Viewport{ Offset2D{ halfResX,        0 }, halfRes }, // Right-top
        Viewport{ Offset2D{ halfResX, halfResY }, halfRes }, // Right-bottom
        Viewport{ Offset2D{        0, halfResY }, halfRes }, // Left-bottom
    };

    BEGIN();
    {
        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
        cmdBuffer->SetVertexBuffer(*meshBuffer);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->Clear(ClearFlags::ColorDepth);

            // Render scene with VRS enabled
            BindPipeline(psoVRSEnabled);
            for_range(i, 3)
                DrawScene(viewports[i], true, shadingRatesToTest[i]);

            // Render scene without VRS
            BindPipeline(psoVRSDisabled);
            DrawScene(viewports[3]);

            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    END();

    // Match entire color buffer and create delta heat map
    const char* colorBufferName = "VariableRateShading";
    SaveCapture(readbackTex, colorBufferName);

    constexpr int threshold = 5;
    const DiffResult diff = DiffImages(colorBufferName, threshold);

    // Evaluate readback result
    TestResult result = diff.Evaluate("variable rate shading");

    // Clear resources
    renderer->Release(*psoVRSDisabled);
    renderer->Release(*psoVRSEnabled);

    return result;
}

