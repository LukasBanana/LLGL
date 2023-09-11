/*
 * TestSceneUpdate.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/ProjectionMatrix4.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


DEF_TEST( SceneUpdate )
{
    const Extent2D resolution{ swapChain->GetResolution() };

    static Texture*         readbackTex;
    static PipelineLayout*  psoLayout;
    static PipelineState*   pso;

    if (frame == 0)
    {
        if (shaders[VSSolid] == nullptr || shaders[PSSolid] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        // Create texture for readback
        TextureDescriptor texDesc;
        {
            texDesc.format          = Format::RGBA8UNorm;
            texDesc.extent.width    = resolution.width;
            texDesc.extent.height   = resolution.height;
            texDesc.bindFlags       = BindFlags::CopyDst;
            texDesc.mipLevels       = 1;
        }
        readbackTex = renderer->CreateTexture(texDesc);
        readbackTex->SetName("readbackTex");

        // Create PSO for rendering to the depth buffer
        psoLayout = renderer->CreatePipelineLayout(Parse("cbuffer(Scene@1):vert:frag"));

        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.pipelineLayout      = psoLayout;
            psoDesc.renderPass          = swapChain->GetRenderPass();
            psoDesc.vertexShader        = shaders[VSSolid];
            psoDesc.fragmentShader      = shaders[PSSolid];
            psoDesc.depth.testEnabled   = true;
            psoDesc.depth.writeEnabled  = true;
            psoDesc.rasterizer.cullMode = CullMode::Back;
        }
        pso = renderer->CreatePipelineState(psoDesc);

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
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    auto TransformWorldMatrix = [](Gs::Matrix4f& wMatrix, float posY, float rotation)
    {
        constexpr float posZ = 2.0f;
        wMatrix.LoadIdentity();
        Gs::Translate(wMatrix, Gs::Vector3f{ 0, posY, posZ });
        Gs::RotateFree(wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(rotation));
        Gs::Scale(wMatrix, Gs::Vector3f{ 1, 0.4f, 1 });
    };

    constexpr unsigned numFrames = 10;
    const float rotation = static_cast<float>(frame) * 90.0f / static_cast<float>(numFrames - 1);

    const TextureRegion texRegionFullRes{ Offset3D{}, Extent3D{ resolution.width, resolution.height, 1 } };

    // Render scene
    cmdBuffer->Begin();
    {
        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::ColorDepth);
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetViewport(resolution);
            cmdBuffer->SetVertexBuffer(*meshBuffer);
            cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, models[ModelCube].indexBufferOffset);
            cmdBuffer->SetResource(0, *sceneCbuffer);

            // Draw top part
            sceneConstants.solidColor = { 1.0f, 0.7f, 0.6f, 1.0f };
            TransformWorldMatrix(sceneConstants.wMatrix, 0.8f, rotation);
            cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
            cmdBuffer->DrawIndexed(models[ModelCube].numIndices, 0);

            // Draw middle part
            sceneConstants.solidColor = { 0.5f, 1.0f, 0.4f, 1.0f };
            TransformWorldMatrix(sceneConstants.wMatrix, 0.0f, rotation);
            cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
            cmdBuffer->DrawIndexed(models[ModelCube].numIndices, 0);

            // Draw bottom part
            sceneConstants.solidColor = { 0.3f, 0.7f, 1.0f, 1.0f };
            TransformWorldMatrix(sceneConstants.wMatrix, -0.8f, rotation);
            cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
            cmdBuffer->DrawIndexed(models[ModelCube].numIndices, 0);

            cmdBuffer->CopyTextureFromFramebuffer(*readbackTex, texRegionFullRes, Offset2D{});
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Match entire color buffer and create delta heat map
    std::vector<ColorRGBub> readbackColorBuffer;
    readbackColorBuffer.resize(resolution.width * resolution.height);

    DstImageDescriptor dstImageDesc;
    {
        dstImageDesc.format     = ImageFormat::RGB;
        dstImageDesc.data       = readbackColorBuffer.data();
        dstImageDesc.dataSize   = sizeof(decltype(readbackColorBuffer)::value_type) * readbackColorBuffer.size();
        dstImageDesc.dataType   = DataType::UInt8;
    }
    renderer->ReadTexture(*readbackTex, texRegionFullRes, dstImageDesc);

    const std::string colorBufferName = "ColorBuffer_Frame" + std::to_string(frame);

    SaveColorImageTGA(readbackColorBuffer, resolution, colorBufferName);

    const int diff = DiffImagesTGA(colorBufferName);

    // Evaluate readback result
    static bool diffFailed = false;
    if (diff != 0)
    {
        Log::Errorf("Mismatch between reference and result images for color buffer [frame %u] (diff = %d)\n", frame, diff);
        diffFailed = true;
    }

    if (frame + 1 < numFrames)
        return TestResult::Continue;

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*psoLayout);
    renderer->Release(*readbackTex);

    return (diffFailed ? TestResult::FailedMismatch : TestResult::Passed);
}

