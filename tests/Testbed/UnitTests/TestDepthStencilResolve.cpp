/*
 * TestDepthStencilResolve.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <Gauss/ProjectionMatrix4.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <algorithm>
#include <cmath>
#include <vector>


/*
Verifies RenderTargetDescriptor::depthStencilResolveAttachment: a multi-sampled depth buffer is resolved into a
single-sampled texture at the end of the render pass.

The oracle is a second render of the identical scene into a plain, single-sampled depth target. A depth-stencil
resolve takes sample 0 rather than averaging, and sample 0 sits only a fraction of a pixel from the pixel center,
so on the smooth interior of a surface the resolved depth must agree with the non-multi-sampled depth to well
within the tolerance below. Pixels along geometry edges may legitimately disagree -- there, whether sample 0 is
covered at all differs from whether the pixel center is -- so a small fraction of mismatches is permitted.

This is what distinguishes a working resolve from a plausible-looking failure: a resolve that never executed
leaves the resolve target at its cleared far value, which matches the background everywhere and the geometry
nowhere. That is caught by both the interior-pixel check and the mismatch-fraction check, neither of which a
constant image can pass.

The multi-sampled attachments deliberately carry a color attachment and its color resolve target as well, so the
attachment layout matches the real (XR) use case: the depth resolve descriptor is indexed after all color resolve
targets, and an off-by-one there would bind the wrong image.

Skipped on backends/devices without RenderingFeatures::hasDepthStencilResolve. Only Vulkan reports it.
*/
DEF_TEST( DepthStencilResolve )
{
    if (!caps.features.hasDepthStencilResolve)
        return TestResult::Skipped;

    if (shaders[VSSolid] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    /* Pick a sample count both the color and depth buffers support */
    const std::uint32_t maxSamples = std::min<std::uint32_t>(caps.limits.maxColorBufferSamples, caps.limits.maxDepthBufferSamples);
    const std::uint32_t numSamples = std::min<std::uint32_t>(4u, maxSamples);
    if (numSamples < 2)
        return TestResult::Skipped;

    const Extent2D  resolution  = opt.resolution;
    const Extent3D  extent3D    = { resolution.width, resolution.height, 1 };
    const std::size_t numTexels = static_cast<std::size_t>(resolution.width) * resolution.height;

    /* --- Reference: plain single-sampled depth target --- */
    TextureDescriptor refDepthDesc;
    {
        refDepthDesc.debugName  = "dsResolve.refDepth";
        refDepthDesc.type       = TextureType::Texture2D;
        refDepthDesc.format     = Format::D32Float;
        refDepthDesc.extent     = extent3D;
        refDepthDesc.mipLevels  = 1;
        refDepthDesc.bindFlags  = BindFlags::DepthStencilAttachment;
    }
    CREATE_TEXTURE(refDepthTex, refDepthDesc, "dsResolve.refDepth{d32}", nullptr);

    RenderTargetDescriptor refTargetDesc;
    {
        refTargetDesc.debugName                 = "dsResolve.refTarget";
        refTargetDesc.resolution                = resolution;
        refTargetDesc.depthStencilAttachment    = refDepthTex;
    }
    CREATE_RENDER_TARGET(refTarget, refTargetDesc, "dsResolve.refTarget");

    /* --- Multi-sampled target with color and depth resolve --- */
    TextureDescriptor msColorDesc;
    {
        msColorDesc.debugName   = "dsResolve.msColor";
        msColorDesc.type        = TextureType::Texture2DMS;
        msColorDesc.format      = Format::RGBA8UNorm;
        msColorDesc.extent      = extent3D;
        msColorDesc.mipLevels   = 1;
        msColorDesc.samples     = numSamples;
        msColorDesc.bindFlags   = BindFlags::ColorAttachment;
        msColorDesc.miscFlags   = MiscFlags::FixedSamples | MiscFlags::NoInitialData;
    }
    CREATE_TEXTURE(msColorTex, msColorDesc, "dsResolve.msColor{rgba8,ms}", nullptr);

    TextureDescriptor msDepthDesc;
    {
        msDepthDesc.debugName   = "dsResolve.msDepth";
        msDepthDesc.type        = TextureType::Texture2DMS;
        msDepthDesc.format      = Format::D32Float;
        msDepthDesc.extent      = extent3D;
        msDepthDesc.mipLevels   = 1;
        msDepthDesc.samples     = numSamples;
        msDepthDesc.bindFlags   = BindFlags::DepthStencilAttachment;
        msDepthDesc.miscFlags   = MiscFlags::FixedSamples | MiscFlags::NoInitialData;
    }
    CREATE_TEXTURE(msDepthTex, msDepthDesc, "dsResolve.msDepth{d32,ms}", nullptr);

    TextureDescriptor resolveColorDesc;
    {
        resolveColorDesc.debugName  = "dsResolve.resolveColor";
        resolveColorDesc.type       = TextureType::Texture2D;
        resolveColorDesc.format     = Format::RGBA8UNorm;
        resolveColorDesc.extent     = extent3D;
        resolveColorDesc.mipLevels  = 1;
        resolveColorDesc.bindFlags  = BindFlags::ColorAttachment;
    }
    CREATE_TEXTURE(resolveColorTex, resolveColorDesc, "dsResolve.resolveColor{rgba8}", nullptr);

    /* The depth resolve target is what this test reads back, hence CopySrc */
    TextureDescriptor resolveDepthDesc;
    {
        resolveDepthDesc.debugName  = "dsResolve.resolveDepth";
        resolveDepthDesc.type       = TextureType::Texture2D;
        resolveDepthDesc.format     = Format::D32Float;
        resolveDepthDesc.extent     = extent3D;
        resolveDepthDesc.mipLevels  = 1;
        resolveDepthDesc.bindFlags  = BindFlags::DepthStencilAttachment | BindFlags::CopySrc;
    }
    CREATE_TEXTURE(resolveDepthTex, resolveDepthDesc, "dsResolve.resolveDepth{d32}", nullptr);

    RenderTargetDescriptor msTargetDesc;
    {
        msTargetDesc.debugName                      = "dsResolve.msTarget";
        msTargetDesc.resolution                     = resolution;
        msTargetDesc.samples                        = numSamples;
        msTargetDesc.colorAttachments[0]            = msColorTex;
        msTargetDesc.resolveAttachments[0]          = resolveColorTex;
        msTargetDesc.depthStencilAttachment         = msDepthTex;
        msTargetDesc.depthStencilResolveAttachment  = resolveDepthTex;
    }
    CREATE_RENDER_TARGET(msTarget, msTargetDesc, "dsResolve.msTarget");

    /* --- Pipelines: depth-only rasterization, as in TestDepthBuffer --- */
    auto MakePSO = [&](RenderTarget* target, bool multiSampled) -> GraphicsPipelineDescriptor
    {
        GraphicsPipelineDescriptor psoDesc;
        psoDesc.pipelineLayout                  = layouts[PipelineSolid];
        psoDesc.renderPass                      = target->GetRenderPass();
        psoDesc.inputVertexAttribs              = vertexFormats[VertFmtStd].attributes;
        psoDesc.vertexShader                    = shaders[VSSolid];
        psoDesc.depth.testEnabled               = true;
        psoDesc.depth.writeEnabled              = true;
        psoDesc.rasterizer.cullMode             = CullMode::Back;
        psoDesc.rasterizer.multiSampleEnabled   = multiSampled; // Required for the PSO to inherit the render pass' sample count
        psoDesc.blend.targets[0].colorMask      = 0; // No fragment shader; rasterize for depth only
        return psoDesc;
    };

    GraphicsPipelineDescriptor refPSODesc = MakePSO(refTarget, false);
    CREATE_GRAPHICS_PSO(refPSO, refPSODesc, "dsResolve.refPSO");

    GraphicsPipelineDescriptor msPSODesc = MakePSO(msTarget, true);
    CREATE_GRAPHICS_PSO(msPSO, msPSODesc, "dsResolve.msPSO");

    /* --- Identical scene for both passes --- */
    sceneConstants = SceneConstants{};

    sceneConstants.wMatrix.LoadIdentity();
    Gs::Translate(sceneConstants.wMatrix, Gs::Vector3f{ 0, 0, 2 });
    Gs::RotateFree(sceneConstants.wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(20.0f));

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    auto DrawScene = [&](RenderTarget* target, PipelineState* pso, long clearFlags)
    {
        cmdBuffer->BeginRenderPass(*target);
        {
            cmdBuffer->Clear(clearFlags);
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetViewport(resolution);
            cmdBuffer->SetVertexBuffer(*meshBuffer);
            cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, models[ModelCube].indexBufferOffset);
            cmdBuffer->SetResource(0, *sceneCbuffer);
            cmdBuffer->DrawIndexed(models[ModelCube].numIndices, 0);
        }
        cmdBuffer->EndRenderPass();
    };

    BEGIN();
    {
        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
        DrawScene(refTarget, refPSO, ClearFlags::Depth);
        DrawScene(msTarget, msPSO, ClearFlags::ColorDepth);
    }
    END();

    /* --- Read both depth buffers back --- */
    auto ReadDepth = [&](Texture* tex, std::vector<float>& outDepth)
    {
        outDepth.assign(numTexels, -1.0f);
        MutableImageView dstView;
        {
            dstView.format      = ImageFormat::Depth;
            dstView.dataType    = DataType::Float32;
            dstView.data        = outDepth.data();
            dstView.dataSize    = sizeof(float) * outDepth.size();
        }
        renderer->ReadTexture(*tex, TextureRegion{ Offset3D{}, extent3D }, dstView);
    };

    std::vector<float> refDepth, resolvedDepth;
    ReadDepth(refDepthTex, refDepth);
    ReadDepth(resolveDepthTex, resolvedDepth);

    TestResult result = TestResult::Passed;

    /*
    The resolved depth must contain a range of values. A resolve that never ran leaves the cleared far value
    everywhere, which this catches before any comparison against the reference.
    */
    const auto resolvedMinMax = std::minmax_element(resolvedDepth.begin(), resolvedDepth.end());
    const float resolvedMin = *resolvedMinMax.first;
    const float resolvedMax = *resolvedMinMax.second;
    if (resolvedMax - resolvedMin < 0.001f)
    {
        Log::Errorf(
            "DepthStencilResolve: resolved depth is effectively constant (min=%f, max=%f); the resolve likely did not run\n",
            resolvedMin, resolvedMax
        );
        result = TestResult::FailedMismatch;
    }

    /* Interior of the geometry: sample 0 and the pixel center lie on the same surface, so these must agree */
    const std::size_t centerIndex = (resolution.height / 2) * resolution.width + (resolution.width / 2);
    const float centerRef       = refDepth[centerIndex];
    const float centerResolved  = resolvedDepth[centerIndex];
    constexpr float centerTolerance = 0.001f;
    if (std::abs(centerRef - centerResolved) > centerTolerance)
    {
        Log::Errorf(
            "DepthStencilResolve: center depth mismatch: resolved=%f, reference=%f (tolerance %f)\n",
            centerResolved, centerRef, centerTolerance
        );
        result = TestResult::FailedMismatch;
    }

    /*
    Whole-image agreement. Edge pixels may legitimately differ (sample 0 and the pixel center can fall on
    opposite sides of a silhouette), so allow a small fraction of mismatches but no more.
    */
    constexpr float perTexelTolerance   = 0.001f;
    constexpr float maxMismatchFraction = 0.05f;

    std::size_t numMismatches = 0;
    for_range(i, numTexels)
    {
        if (std::abs(refDepth[i] - resolvedDepth[i]) > perTexelTolerance)
            ++numMismatches;
    }

    const float mismatchFraction = static_cast<float>(numMismatches) / static_cast<float>(numTexels);
    if (mismatchFraction > maxMismatchFraction)
    {
        Log::Errorf(
            "DepthStencilResolve: %zu of %zu texels (%.2f%%) differ from the non-multi-sampled reference by more than %f\n",
            numMismatches, numTexels, mismatchFraction * 100.0f, perTexelTolerance
        );
        result = TestResult::FailedMismatch;
    }

    if (opt.verbose)
    {
        Log::Printf(
            "DepthStencilResolve: samples=%u, resolved depth range [%f, %f], center %f vs reference %f, %.2f%% edge mismatches\n",
            numSamples, resolvedMin, resolvedMax, centerResolved, centerRef, mismatchFraction * 100.0f
        );
    }

    /* Cleanup */
    renderer->Release(*msPSO);
    renderer->Release(*refPSO);
    renderer->Release(*msTarget);
    renderer->Release(*refTarget);
    renderer->Release(*resolveDepthTex);
    renderer->Release(*resolveColorTex);
    renderer->Release(*msDepthTex);
    renderer->Release(*msColorTex);
    renderer->Release(*refDepthTex);

    return result;
}
