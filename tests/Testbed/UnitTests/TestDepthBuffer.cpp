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
    const Extent2D resolution{ swapChain->GetResolution() };

    if (shaders[VSSolid] == nullptr || shaders[PSSolid] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Create texture for readback with depth-only format (D32Float)
    TextureDescriptor texDesc;
    {
        texDesc.format          = Format::D32Float;
        texDesc.extent.width    = resolution.width;
        texDesc.extent.height   = resolution.height;
        texDesc.bindFlags       = BindFlags::DepthStencilAttachment;
        texDesc.mipLevels       = 1;
    }
    Texture* readbackTex = renderer->CreateTexture(texDesc);
    readbackTex->SetName("readbackTex");

    // Create depth-only render target for scene
    RenderTargetDescriptor renderTargetDesc;
    {
        renderTargetDesc.resolution             = resolution;
        renderTargetDesc.depthStencilAttachment = readbackTex;
    }
    RenderTarget* renderTarget = renderer->CreateRenderTarget(renderTargetDesc);
    renderTarget->SetName("renderTarget");

    // Create PSO for rendering to the depth buffer
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = layouts[PipelineSolid];
        psoDesc.renderPass          = renderTarget->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSSolid];
        psoDesc.depth.testEnabled   = true;
        psoDesc.depth.writeEnabled  = true;
        psoDesc.rasterizer.cullMode = CullMode::Back;
    }
    PipelineState* pso = renderer->CreatePipelineState(psoDesc);

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
            cmdBuffer->SetViewport(resolution);
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
        static_cast<std::int32_t>(resolution.width/2),
        static_cast<std::int32_t>(resolution.height/2),
        0,
    };
    const TextureRegion readbackTexRegion{ readbackTexPosition, Extent3D{ 1, 1, 1 } };

    constexpr float invalidDepthValue = -1.0f;

    float readbackDepthValue = invalidDepthValue;

    DstImageDescriptor dstImageDesc;
    {
        dstImageDesc.format     = ImageFormat::Depth;
        dstImageDesc.data       = &readbackDepthValue;
        dstImageDesc.dataSize   = sizeof(readbackDepthValue);
        dstImageDesc.dataType   = DataType::Float32;
    }
    renderer->ReadTexture(*readbackTex, readbackTexRegion, dstImageDesc);

    constexpr float expectedDepthValue = 0.975574434f;

    const float deltaDepthValue = std::abs(readbackDepthValue - expectedDepthValue);

    // Match entire depth and create delta heat map
    std::vector<float> readbackDepthBuffer;
    readbackDepthBuffer.resize(texDesc.extent.width * texDesc.extent.height, -1.0f);
    {
        dstImageDesc.format     = ImageFormat::Depth;
        dstImageDesc.data       = readbackDepthBuffer.data();
        dstImageDesc.dataSize   = sizeof(decltype(readbackDepthBuffer)::value_type) * readbackDepthBuffer.size();
        dstImageDesc.dataType   = DataType::Float32;
    }
    renderer->ReadTexture(*readbackTex, TextureRegion{ Offset3D{}, texDesc.extent }, dstImageDesc);

    SaveDepthImage(readbackDepthBuffer, resolution, "DepthBuffer", 1.0f, 10.0f);

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
    if (diff)
    {
        Log::Errorf("Mismatch between reference and result images for depth buffer (%s)\n", diff.Print());
        return TestResult::FailedMismatch;
    }

    return TestResult::Passed;
}

