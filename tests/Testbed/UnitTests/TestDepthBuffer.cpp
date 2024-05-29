/*
 * TestDepthBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/ProjectionMatrix4.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>


DEF_TEST( DepthBuffer )
{
    if (shaders[VSSolid] == nullptr || shaders[PSSolid] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Create texture for readback with depth-only format (D32Float)
    TextureDescriptor texDesc;
    {
        texDesc.debugName       = "readbackTex";
        texDesc.format          = Format::D32Float;
        texDesc.extent.width    = opt.resolution.width;
        texDesc.extent.height   = opt.resolution.height;
        texDesc.bindFlags       = BindFlags::DepthStencilAttachment;
        texDesc.mipLevels       = 1;
    }
    Texture* readbackTex = renderer->CreateTexture(texDesc);

    // Create depth-only render target for scene
    RenderTargetDescriptor renderTargetDesc;
    {
        renderTargetDesc.debugName              = "renderTarget";
        renderTargetDesc.resolution             = opt.resolution;
        renderTargetDesc.depthStencilAttachment = readbackTex;
    }
    RenderTarget* renderTarget = renderer->CreateRenderTarget(renderTargetDesc);

    // Create PSO for rendering to the depth buffer
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout              = layouts[PipelineSolid];
        psoDesc.renderPass                  = renderTarget->GetRenderPass();
        psoDesc.vertexShader                = shaders[VSSolid];
        psoDesc.depth.testEnabled           = true;
        psoDesc.depth.writeEnabled          = true;
        psoDesc.rasterizer.cullMode         = CullMode::Back;
        psoDesc.blend.targets[0].colorMask  = 0; // Disable rasterize with colorMask=0 since we don't use a fragment shader
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoDepthBuf");

    if (const Report* report = pso->GetReport())
    {
        if (report->HasErrors())
        {
            Log::Errorf("PSO creation failed:\n%s", report->GetText());
            return TestResult::FailedErrors;
        }
    }

    // Update scene constants
    sceneConstants = SceneConstants{};

    sceneConstants.wMatrix.LoadIdentity();
    Gs::Translate(sceneConstants.wMatrix, Gs::Vector3f{ 0, 0, 2 });
    Gs::RotateFree(sceneConstants.wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(20.0f));

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    // Render scene
    cmdBuffer->Begin();
    {
        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
        cmdBuffer->BeginRenderPass(*renderTarget);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::Depth);
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetViewport(opt.resolution);
            cmdBuffer->SetVertexBuffer(*meshBuffer);
            cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, models[ModelCube].indexBufferOffset);
            cmdBuffer->SetResource(0, *sceneCbuffer);
            cmdBuffer->DrawIndexed(models[ModelCube].numIndices, 0);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Readback depth buffer and compare with expected result
    const Offset3D readbackTexPosition
    {
        static_cast<std::int32_t>(opt.resolution.width/2),
        static_cast<std::int32_t>(opt.resolution.height/2),
        0,
    };
    const TextureRegion readbackTexRegion{ readbackTexPosition, Extent3D{ 1, 1, 1 } };

    constexpr float invalidDepthValue = -1.0f;

    float readbackDepthValue = invalidDepthValue;

    MutableImageView dstImageView;
    {
        dstImageView.format     = ImageFormat::Depth;
        dstImageView.data       = &readbackDepthValue;
        dstImageView.dataSize   = sizeof(readbackDepthValue);
        dstImageView.dataType   = DataType::Float32;
    }
    renderer->ReadTexture(*readbackTex, readbackTexRegion, dstImageView);

    constexpr float expectedDepthValue = 0.975574434f;

    const float deltaDepthValue = std::abs(readbackDepthValue - expectedDepthValue);

    // Match entire depth and create delta heat map
    std::vector<float> readbackDepthBuffer;
    readbackDepthBuffer.resize(texDesc.extent.width * texDesc.extent.height, -1.0f);
    {
        dstImageView.format     = ImageFormat::Depth;
        dstImageView.data       = readbackDepthBuffer.data();
        dstImageView.dataSize   = sizeof(decltype(readbackDepthBuffer)::value_type) * readbackDepthBuffer.size();
        dstImageView.dataType   = DataType::Float32;
    }
    renderer->ReadTexture(*readbackTex, TextureRegion{ Offset3D{}, texDesc.extent }, dstImageView);

    SaveDepthImage(readbackDepthBuffer, opt.resolution, "DepthBuffer", 1.0f, 10.0f);

    const DiffResult diff = DiffImages("DepthBuffer");

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*renderTarget);
    renderer->Release(*readbackTex);

    // Evaluate readback result
    if (readbackDepthValue == -1.0f)
    {
        Log::Errorf("Failed to read back value from depth buffer texture at center\n");
        return TestResult::FailedErrors;
    }
    if (deltaDepthValue > epsilon)
    {
        Log::Errorf(
            "Mismatch between depth buffer value at center (%f) and expected value (%f): delta = %f\n",
            readbackDepthValue, expectedDepthValue, deltaDepthValue
        );
        return TestResult::FailedMismatch;
    }

    return diff.Evaluate("depth buffer");
}

