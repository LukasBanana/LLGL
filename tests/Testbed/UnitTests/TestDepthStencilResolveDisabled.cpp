/*
 * TestDepthStencilResolveDisabled.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <algorithm>
#include <cmath>
#include <vector>


/*
Verifies that ResolveMode::Disabled for the depth attachment leaves RenderTargetDescriptor::depthStencilResolveAttachment
untouched at the end of a multi-sampled render pass, and that the default render pass resolves the same depth as an
explicit ResolveMode::Optimal render pass.

The resolve target is first cleared to a sentinel value via a separate single-sampled render target.
Skipped on devices without RenderingFeatures::hasDepthStencilResolve.
*/
DEF_TEST( DepthStencilResolveDisabled )
{
    if (!caps.features.hasDepthStencilResolve)
        return TestResult::Skipped;

    if (shaders[VSSolid] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    const std::uint32_t maxSamples = std::min<std::uint32_t>(caps.limits.maxColorBufferSamples, caps.limits.maxDepthBufferSamples);
    const std::uint32_t numSamples = std::min<std::uint32_t>(4u, maxSamples);
    if (numSamples < 2)
        return TestResult::Skipped;

    const Extent2D  resolution  = opt.resolution;
    const Extent3D  extent3D    = { resolution.width, resolution.height, 1 };
    const std::size_t numTexels = static_cast<std::size_t>(resolution.width) * resolution.height;

    /* Multi-sampled target with color and depth resolve, matching the attachment layout of TestDepthStencilResolve */
    TextureDescriptor msColorDesc;
    {
        msColorDesc.type        = TextureType::Texture2DMS;
        msColorDesc.format      = Format::RGBA8UNorm;
        msColorDesc.extent      = extent3D;
        msColorDesc.mipLevels   = 1;
        msColorDesc.samples     = numSamples;
        msColorDesc.bindFlags   = BindFlags::ColorAttachment;
        msColorDesc.miscFlags   = MiscFlags::FixedSamples | MiscFlags::NoInitialData;
    }
    CREATE_TEXTURE(msColorTex, msColorDesc, "dsResolveDisabled.msColor{rgba8,ms}", nullptr);

    TextureDescriptor msDepthDesc = msColorDesc;
    {
        msDepthDesc.format      = Format::D32Float;
        msDepthDesc.bindFlags   = BindFlags::DepthStencilAttachment;
    }
    CREATE_TEXTURE(msDepthTex, msDepthDesc, "dsResolveDisabled.msDepth{d32,ms}", nullptr);

    TextureDescriptor resolveColorDesc;
    {
        resolveColorDesc.type       = TextureType::Texture2D;
        resolveColorDesc.format     = Format::RGBA8UNorm;
        resolveColorDesc.extent     = extent3D;
        resolveColorDesc.mipLevels  = 1;
        resolveColorDesc.bindFlags  = BindFlags::ColorAttachment;
    }
    CREATE_TEXTURE(resolveColorTex, resolveColorDesc, "dsResolveDisabled.resolveColor{rgba8}", nullptr);

    TextureDescriptor resolveDepthDesc = resolveColorDesc;
    {
        resolveDepthDesc.format     = Format::D32Float;
        resolveDepthDesc.bindFlags  = BindFlags::DepthStencilAttachment | BindFlags::CopySrc;
    }
    CREATE_TEXTURE(resolveDepthTex, resolveDepthDesc, "dsResolveDisabled.resolveDepth{d32}", nullptr);

    RenderTargetDescriptor msTargetDesc;
    {
        msTargetDesc.resolution                     = resolution;
        msTargetDesc.samples                        = numSamples;
        msTargetDesc.colorAttachments[0]            = msColorTex;
        msTargetDesc.resolveAttachments[0]          = resolveColorTex;
        msTargetDesc.depthStencilAttachment         = msDepthTex;
        msTargetDesc.depthStencilResolveAttachment  = resolveDepthTex;
    }
    CREATE_RENDER_TARGET(msTarget, msTargetDesc, "dsResolveDisabled.msTarget");

    /* Single-sampled target to clear the resolve destination with a sentinel value */
    RenderTargetDescriptor sentinelTargetDesc;
    {
        sentinelTargetDesc.resolution               = resolution;
        sentinelTargetDesc.depthStencilAttachment   = resolveDepthTex;
    }
    CREATE_RENDER_TARGET(sentinelTarget, sentinelTargetDesc, "dsResolveDisabled.sentinelTarget");

    RenderPassDescriptor optimalPassDesc;
    {
        optimalPassDesc.samples                         = numSamples;
        optimalPassDesc.colorAttachments[0].format      = Format::RGBA8UNorm;
        optimalPassDesc.colorAttachments[0].storeOp     = AttachmentStoreOp::Store;
        optimalPassDesc.colorAttachments[0].resolveMode = ResolveMode::Optimal;
        optimalPassDesc.depthAttachment.format          = Format::D32Float;
        optimalPassDesc.depthAttachment.storeOp         = AttachmentStoreOp::Store;
        optimalPassDesc.depthAttachment.resolveMode     = ResolveMode::Optimal;
    }
    RenderPass* optimalPass = renderer->CreateRenderPass(optimalPassDesc);

    RenderPassDescriptor disabledPassDesc = optimalPassDesc;
    {
        disabledPassDesc.colorAttachments[0].resolveMode    = ResolveMode::Disabled;
        disabledPassDesc.depthAttachment.resolveMode        = ResolveMode::Disabled;
    }
    RenderPass* disabledPass = renderer->CreateRenderPass(disabledPassDesc);

    /* Depth-only rasterization of a rotated cube, as in TestDepthStencilResolve */
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout                  = layouts[PipelineSolid];
        psoDesc.renderPass                      = optimalPass;
        psoDesc.inputVertexAttribs              = vertexFormats[VertFmtStd].attributes;
        psoDesc.vertexShader                    = shaders[VSSolid];
        psoDesc.depth.testEnabled               = true;
        psoDesc.depth.writeEnabled              = true;
        psoDesc.rasterizer.cullMode             = CullMode::Back;
        psoDesc.rasterizer.multiSampleEnabled   = true;
        psoDesc.blend.targets[0].colorMask      = 0;
    }
    CREATE_GRAPHICS_PSO(msPSO, psoDesc, "dsResolveDisabled.msPSO");

    sceneConstants = SceneConstants{};

    sceneConstants.wMatrix.LoadIdentity();
    Gs::Translate(sceneConstants.wMatrix, Gs::Vector3f{ 0, 0, 2 });
    Gs::RotateFree(sceneConstants.wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(20.0f));

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    auto DrawScene = [&](const char* debugLabel, const RenderPass* renderPass)
    {
        BEGIN();
        {
            cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
            cmdBuffer->PushDebugGroup(debugLabel);
            {
                cmdBuffer->BeginRenderPass(*msTarget, renderPass);
                {
                    cmdBuffer->Clear(ClearFlags::ColorDepth);
                    cmdBuffer->SetPipelineState(*msPSO);
                    cmdBuffer->SetViewport(resolution);
                    cmdBuffer->SetVertexBuffer(*meshBuffer);
                    cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, models[ModelCube].indexBufferOffset);
                    cmdBuffer->SetResource(0, *sceneCbuffer);
                    cmdBuffer->DrawIndexed(models[ModelCube].numIndices, 0);
                }
                cmdBuffer->EndRenderPass();
            }
            cmdBuffer->PopDebugGroup();
        }
        END();
    };

    auto ReadDepth = [&](std::vector<float>& outDepth)
    {
        outDepth.assign(numTexels, -1.0f);
        MutableImageView dstView;
        {
            dstView.format      = ImageFormat::Depth;
            dstView.dataType    = DataType::Float32;
            dstView.data        = outDepth.data();
            dstView.dataSize    = sizeof(float) * outDepth.size();
        }
        renderer->ReadTexture(*resolveDepthTex, TextureRegion{ Offset3D{}, extent3D }, dstView);
    };

    auto IsDepthConstant = [](const std::vector<float>& depth, float value) -> bool
    {
        for (float d : depth)
        {
            if (std::abs(d - value) > 0.001f)
                return false;
        }
        return true;
    };

    TestResult result = TestResult::Passed;

    /* Resolve with explicit Optimal mode as reference */
    std::vector<float> optimalDepth, resolvedDepth;
    DrawScene("OptimalDepthResolve", optimalPass);
    ReadDepth(optimalDepth);

    /* Initialize resolve destination with sentinel value */
    ClearValue sentinel;
    sentinel.depth = 0.25f;
    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*sentinelTarget);
        cmdBuffer->Clear(ClearFlags::Depth, sentinel);
        cmdBuffer->EndRenderPass();
    }
    END();
    ReadDepth(resolvedDepth);
    if (!IsDepthConstant(resolvedDepth, sentinel.depth))
    {
        Log::Errorf("DepthStencilResolveDisabled: failed to initialize the resolve destination\n");
        result = TestResult::FailedMismatch;
    }

    /* Disabled resolve must preserve the sentinel value */
    DrawScene("DisabledDepthResolve", disabledPass);
    ReadDepth(resolvedDepth);
    if (!IsDepthConstant(resolvedDepth, sentinel.depth))
    {
        Log::Errorf("DepthStencilResolveDisabled: disabled resolve modified the destination\n");
        result = TestResult::FailedMismatch;
    }

    /* Default render pass must resolve the same as explicit Optimal mode */
    DrawScene("DefaultDepthResolve", nullptr);
    ReadDepth(resolvedDepth);
    for_range(index, numTexels)
    {
        if (std::abs(optimalDepth[index] - resolvedDepth[index]) > 0.000001f)
        {
            Log::Errorf("DepthStencilResolveDisabled: default and explicit Optimal resolves differ at texel %zu\n", index);
            result = TestResult::FailedMismatch;
            break;
        }
    }

    /* Cleanup */
    renderer->Release(*msPSO);
    renderer->Release(*disabledPass);
    renderer->Release(*optimalPass);
    renderer->Release(*sentinelTarget);
    renderer->Release(*msTarget);
    renderer->Release(*resolveDepthTex);
    renderer->Release(*resolveColorTex);
    renderer->Release(*msDepthTex);
    renderer->Release(*msColorTex);

    return result;
}

