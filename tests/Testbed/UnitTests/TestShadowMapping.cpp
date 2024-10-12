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


struct ShadowMapResources
{
    Texture*        tex;
    RenderTarget*   target;
    PipelineState*  pso;
};

struct ShadowMapConfig
{
    Format          format;
    std::uint32_t   width;
    std::uint32_t   height;
    bool            slow;
};

struct ViewportConfig
{
    Viewport    viewport;
    float       cubeRotation;
    ColorRGBf   bgColor;
};

struct ShadowSceneConstants
{
    Gs::Matrix4f vpMatrix;
    Gs::Matrix4f wMatrix;
    Gs::Matrix4f vpShadowMatrix;
    Gs::Vector4f solidColor     = { 1, 1, 1, 1 };
    Gs::Vector4f lightVec       = { 0, 0, -1, 0 };
};

DEF_TEST( ShadowMapping )
{
    static TestResult result = TestResult::Passed;
    static PipelineLayout* psoLayout;
    static PipelineState* psoScene;
    static Sampler* shadowSampler;
    static Buffer* shadowCbuffer;
    static RenderPass* loadContentRenderPass;

    constexpr ShadowMapConfig frameConfigs[] =
    {
        ShadowMapConfig{ Format::D32Float,           128,  128, false },
        ShadowMapConfig{ Format::D32Float,           256,  256, true  },
        ShadowMapConfig{ Format::D32Float,           512,  512, true  },
        ShadowMapConfig{ Format::D24UNormS8UInt,     256,  256, false },
        ShadowMapConfig{ Format::D24UNormS8UInt,    1024, 1024, true  },
        ShadowMapConfig{ Format::D16UNorm,           256,  256, true  },
        ShadowMapConfig{ Format::D16UNorm,           300,  280, false },
        ShadowMapConfig{ Format::D32FloatS8X24UInt,  256,  256, false },
    };

    if (frame == 0)
    {
        result = TestResult::Passed;

        if (shaders[VSShadowMap] == nullptr || shaders[VSShadowedScene] == nullptr || shaders[PSShadowedScene] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        // Create scene rendering render pass
        RenderPassDescriptor passDesc;
        {
            passDesc.colorAttachments[0].format     = swapChain->GetColorFormat();
            passDesc.colorAttachments[0].loadOp     = AttachmentLoadOp::Load; // Default swap-chain render pass uses undefined loading op
            passDesc.colorAttachments[0].storeOp    = AttachmentStoreOp::Store;
            passDesc.depthAttachment.format         = swapChain->GetDepthStencilFormat();
            passDesc.depthAttachment.loadOp         = AttachmentLoadOp::Undefined; // Don't care about previous framebuffer depth, it'll be cleared anyway
            passDesc.depthAttachment.storeOp        = AttachmentStoreOp::Store;

            passDesc.stencilAttachment.format       = swapChain->GetDepthStencilFormat(); //TODO: currently required for Metal backend
        }
        loadContentRenderPass = renderer->CreateRenderPass(passDesc);

        // Create scene rendering PSO
        psoLayout = renderer->CreatePipelineLayout(
            Parse(
                HasCombinedSamplers()
                    ?   "cbuffer(Scene@1):vert:frag,texture(shadowMap@2):frag,sampler(2):frag"
                    :   "cbuffer(Scene@1):vert:frag,texture(shadowMap@2):frag,sampler(3):frag"
            )
        );

        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout      = psoLayout;
            psoDesc.renderPass          = loadContentRenderPass;
            psoDesc.vertexShader        = shaders[VSShadowedScene];
            psoDesc.fragmentShader      = shaders[PSShadowedScene];
            psoDesc.depth.testEnabled   = true;
            psoDesc.depth.writeEnabled  = true;
            psoDesc.rasterizer.cullMode = CullMode::Back;
        }
        CREATE_GRAPHICS_PSO_EXT(psoScene, psoDesc, "psoShadowScene");

        // Create scene constant buffer
        BufferDescriptor bufDesc;
        {
            bufDesc.debugName   = "shadowCbuffer";
            bufDesc.size        = sizeof(ShadowSceneConstants);
            bufDesc.bindFlags   = BindFlags::ConstantBuffer;
        }
        shadowCbuffer = renderer->CreateBuffer(bufDesc, &sceneCbuffer);

        // Create shadow sampler state
        SamplerDescriptor samplerDesc;
        {
            samplerDesc.addressModeU    = SamplerAddressMode::Border;
            samplerDesc.addressModeV    = SamplerAddressMode::Border;
            samplerDesc.addressModeW    = LLGL::SamplerAddressMode::Border;
            samplerDesc.borderColor[0]  = 1.0f;
            samplerDesc.borderColor[1]  = 1.0f;
            samplerDesc.borderColor[2]  = 1.0f;
            samplerDesc.borderColor[3]  = 1.0f;
            samplerDesc.compareEnabled  = true;
            samplerDesc.mipMapEnabled   = false;
        }
        shadowSampler = renderer->CreateSampler(samplerDesc);
    }

    auto CreateShadowMapResources = [this](ShadowMapResources& resources, const Extent2D& resolution, Format format) -> TestResult
    {
        TestResult result = TestResult::Passed;

        // Create shadow map texture
        TextureDescriptor shadowMapDesc;
        {
            shadowMapDesc.type          = TextureType::Texture2D;
            shadowMapDesc.bindFlags     = BindFlags::Sampled | BindFlags::DepthStencilAttachment;
            shadowMapDesc.format        = format;
            shadowMapDesc.extent        = { resolution.width, resolution.height, 1u };
            shadowMapDesc.mipLevels     = 1;
        }
        const std::string name = std::string("shadowTex-") + ToString(format);
        result = CreateTexture(shadowMapDesc, name.c_str(), &resources.tex);
        if (result != TestResult::Passed)
            return result;

        // Create shadow map render target
        RenderTargetDescriptor rtDesc;
        {
            rtDesc.resolution               = resolution;
            rtDesc.depthStencilAttachment   = resources.tex;
        }
        const std::string rtName = std::string("shadowTarget-") + ToString(format);
        result = CreateRenderTarget(rtDesc, rtName.c_str(), &resources.target);
        if (result != TestResult::Passed)
            return result;

        // Create shadow map PSO
        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout                      = layouts[PipelineSolid];
            psoDesc.renderPass                          = resources.target->GetRenderPass();
            psoDesc.vertexShader                        = shaders[VSShadowMap];
            psoDesc.viewports                           = { Viewport{ resolution } };
            psoDesc.depth.testEnabled                   = true;
            psoDesc.depth.writeEnabled                  = true;
            psoDesc.rasterizer.cullMode                 = CullMode::Back;
            psoDesc.rasterizer.depthBias.constantFactor = 4.0f;
            psoDesc.rasterizer.depthBias.slopeFactor    = 1.5f;
            psoDesc.blend.targets[0].colorMask          = 0x0;
        }
        CREATE_GRAPHICS_PSO_EXT(resources.pso, psoDesc, "psoShadowMap");

        return TestResult::Passed;
    };

    constexpr unsigned numFrames = (sizeof(frameConfigs)/sizeof(frameConfigs[0]));
    const ShadowMapConfig& cfg = frameConfigs[frame % numFrames];

    // Skip every other frame on fast test
    if (opt.fastTest && cfg.slow)
        return (frame + 1 < numFrames ? TestResult::ContinueSkipFrame : result);

    const std::string colorBufferName = "ShadowMapping_" + std::string(ToString(cfg.format)) + "_" + std::to_string(cfg.width) + "x" + std::to_string(cfg.height);

    const std::uint64_t t0 = Timer::Tick();

    if (opt.verbose && !opt.showTiming)
        Log::Printf("Testing %s\n", colorBufferName.c_str());

    // Create shadow map resources for current frame
    ShadowMapResources resources;
    TestResult resourcesResult = CreateShadowMapResources(resources, Extent2D{ cfg.width, cfg.height }, cfg.format);
    if (resourcesResult != TestResult::Passed)
    {
        result = resourcesResult;
        return (opt.greedy ? TestResult::Continue : resourcesResult);
    }

    // Update scene constants
    ShadowSceneConstants sceneConstants;

    // View projection
    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    // Light projection
    Gs::Matrix4f lightTransform;
    lightTransform.LoadIdentity();
    Gs::Translate(lightTransform, Gs::Vector3f{ 2, 2, 1 });
    Gs::RotateFree(lightTransform, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(15.0f));
    Gs::RotateFree(lightTransform, Gs::Vector3f{ 1, 0, 0 }, Gs::Deg2Rad(-15.0f));
    lightTransform.MakeInverse();

    Gs::Matrix4f lightProj;
    LoadProjectionMatrix(lightProj, 1.0f, 0.1f, 50.0f, 70.0f);

    sceneConstants.vpShadowMatrix.LoadIdentity();
    sceneConstants.vpShadowMatrix = lightProj * lightTransform;

    auto TransformWorldMatrix = [](Gs::Matrix4f& wMatrix, float x, float y, float z, float scale, float turn)
    {
        wMatrix.LoadIdentity();
        Gs::Translate(wMatrix, Gs::Vector3f{ x, y, z });
        Gs::RotateFree(wMatrix, Gs::Vector3f{ 1, 1, 1 }.Normalized(), Gs::Deg2Rad(turn));
        Gs::Scale(wMatrix, Gs::Vector3f{ scale });
    };

    auto DrawTriangleMesh = [this, &sceneConstants](const IndexedTriangleMesh& mesh)
    {
        cmdBuffer->UpdateBuffer(*shadowCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
        cmdBuffer->DrawIndexed(mesh.numIndices, 0);
    };

    auto DrawScene = [this, &TransformWorldMatrix, &DrawTriangleMesh, &sceneConstants](const ColorRGBf& bgColor, float cubeRotation)
    {
        // Draw background
        cmdBuffer->PushDebugGroup("Background Plane");
        sceneConstants.solidColor = { bgColor.r, bgColor.g, bgColor.b, 1.0f };
        TransformWorldMatrix(sceneConstants.wMatrix, 0.0f, 0.0f, 6.0f, 10.0f, 0.0f);
        DrawTriangleMesh(models[ModelRect]);
        cmdBuffer->PopDebugGroup();

        for (int y = 0; y < 2; ++y)
        {
            for (int x = 0; x < 2; ++x)
            {
                const std::string meshLabel = "Cube(" + std::to_string(x) + "," + std::to_string(y) + ")";
                cmdBuffer->PushDebugGroup(meshLabel.c_str());
                TransformWorldMatrix(
                    sceneConstants.wMatrix,
                    Gs::Lerp(-2.0f, +2.0f, static_cast<float>(x)),
                    Gs::Lerp(-2.0f, +2.0f, static_cast<float>(y)),
                    6.0f, 0.5f, 0.0f
                );
                DrawTriangleMesh(models[ModelCube]);
                cmdBuffer->PopDebugGroup();
            }
        }

        // Draw box in the front
        cmdBuffer->PushDebugGroup("Front Cube");
        sceneConstants.solidColor = { 1.0f, 1.0f, 0.5f, 1.0f };
        TransformWorldMatrix(sceneConstants.wMatrix, 1.0f, 1.0f, 3.0f, 0.5f, cubeRotation);
        DrawTriangleMesh(models[ModelCube]);
        cmdBuffer->PopDebugGroup();
    };

    // Render scene
    Texture* readbackTex = nullptr;

    const Extent2D      halfRes = { opt.resolution.width/2, opt.resolution.height/2 };
    const std::int32_t  halfResX = static_cast<std::int32_t>(halfRes.width);
    const std::int32_t  halfResY = static_cast<std::int32_t>(halfRes.height);

    const ViewportConfig viewportConfigs[4] =
    {
        ViewportConfig{ Viewport{ Offset2D{        0,        0 }, halfRes },  0.0f, ColorRGBf{ 1.0f, 1.0f, 1.0f } },
        ViewportConfig{ Viewport{ Offset2D{ halfResX,        0 }, halfRes }, 35.0f, ColorRGBf{ 0.8f, 0.6f, 0.6f } },
        ViewportConfig{ Viewport{ Offset2D{ halfResX, halfResY }, halfRes }, 55.0f, ColorRGBf{ 0.6f, 0.8f, 0.6f } },
        ViewportConfig{ Viewport{ Offset2D{        0, halfResY }, halfRes }, 80.0f, ColorRGBf{ 0.6f, 0.6f, 0.8f } },
    };

    cmdBuffer->Begin();
    {
        // Graphics can be set inside and outside a render pass, so test binding this PSO outside the render pass
        cmdBuffer->SetVertexBuffer(*meshBuffer);

        for (std::size_t i = 0; i < sizeof(viewportConfigs)/sizeof(viewportConfigs[0]); ++i)
        {
            const std::string viewportLabel = "Viewport[" + std::to_string(i) + "]";
            cmdBuffer->PushDebugGroup(viewportLabel.c_str());

            // Render shadow map
            //cmdBuffer->SetPipelineState(*resources.pso); //TODO: does not bind the correct shader pipeline in GL outside a render pass

            cmdBuffer->PushDebugGroup("ShadowMap");
            cmdBuffer->BeginRenderPass(*resources.target);
            {
                // Draw scene
                cmdBuffer->Clear(ClearFlags::Depth);
                cmdBuffer->SetPipelineState(*resources.pso); //TODO: move outside render pass (not working with GL)
                cmdBuffer->SetResource(0, *shadowCbuffer);
                DrawScene(viewportConfigs[i].bgColor, viewportConfigs[i].cubeRotation);
            }
            cmdBuffer->EndRenderPass();
            cmdBuffer->PopDebugGroup();

            // Render scene and use custom render pass to preserve framebuffer content (AttachmentLoadOp::Load)
            cmdBuffer->PushDebugGroup("SwapChain");
            //cmdBuffer->SetPipelineState(*psoScene); //TODO: does not bind the correct shader pipeline in GL outside a render pass
            cmdBuffer->BeginRenderPass(*swapChain, (i > 0 ? loadContentRenderPass : nullptr));
            {
                // Draw scene
                cmdBuffer->Clear(ClearFlags::Depth); //NOTE: can be replaced by render pass clear
                cmdBuffer->SetPipelineState(*psoScene); //TODO: move outside render pass
                cmdBuffer->SetViewport(viewportConfigs[i].viewport);
                cmdBuffer->SetResource(0, *shadowCbuffer);
                cmdBuffer->SetResource(1, *resources.tex);
                cmdBuffer->SetResource(2, *shadowSampler);
                DrawScene(viewportConfigs[i].bgColor, viewportConfigs[i].cubeRotation);
            }
            cmdBuffer->EndRenderPass();
            cmdBuffer->PopDebugGroup();

            cmdBuffer->PopDebugGroup();
        }

        // Capture framebuffer (must be inside a render pass)
        cmdBuffer->BeginRenderPass(*swapChain);
        {
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    cmdQueue->WaitIdle();

    // Clear intermediate resources
    renderer->Release(*resources.pso);
    renderer->Release(*resources.target);
    renderer->Release(*resources.tex);

    if (opt.showTiming)
    {
        const std::uint64_t t1 = Timer::Tick();
        Log::Printf("Testing %s (%f ms)\n", colorBufferName.c_str(), TestbedContext::ToMillisecs(t0, t1));
    }

    // Match entire color buffer and create delta heat map
    SaveCapture(readbackTex, colorBufferName);

    constexpr int threshold = 13; // All tests differ by at least 11 between GL and D3D
    constexpr int tolerance = 45; // D16UNorm tests differ by (diff = 73; count = 41) between GL and D3D, so tolerate at least 45 pixels that are out of bounds
    const DiffResult diff = DiffImages(colorBufferName, threshold, tolerance);

    // Evaluate readback result
    TestResult intermediateResult = diff.Evaluate("shadow mapping", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    renderer->Release(*psoScene);
    renderer->Release(*psoLayout);
    renderer->Release(*shadowCbuffer);
    renderer->Release(*shadowSampler);
    renderer->Release(*loadContentRenderPass);

    return result;
}

