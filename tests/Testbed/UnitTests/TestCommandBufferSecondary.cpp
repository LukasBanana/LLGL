/*
 * TestCommandBufferSecondary.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/Utility.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


struct ModelTransform
{
    Gs::Vector3f    origin;
    Gs::Vector3f    scale;
    Gs::Vector3f    color;
    float           pitch;
    float           yaw;
};

DEF_TEST( CommandBufferSecondary )
{
    TestResult result = TestResult::Passed;

    if (shaders[VSSolid] == nullptr || shaders[PSSolid] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    constexpr unsigned  numCmdBuffers = 3;
    constexpr int       diffThreshold = 1;
    constexpr unsigned  diffTolerance = 1;

    // Initialize scene constants
    sceneConstants          = {};
    sceneConstants.vpMatrix = projection;

    // Create graphics PSO
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
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoSecondaryCmdBuf");

    // Create scene buffers:
    const ModelTransform transforms[numCmdBuffers] =
    {
        ModelTransform{ Gs::Vector3f{ -2.0f, +1.0f, 4.0f }, Gs::Vector3f{ 0.5f, 1.5f, 0.5f }, Gs::Vector3f{ 1.0f, 0.6f, 0.6f }, 45.0f, 30.0f },
        ModelTransform{ Gs::Vector3f{  0.0f,  0.0f, 4.0f }, Gs::Vector3f{ 0.5f, 0.5f, 0.5f }, Gs::Vector3f{ 0.6f, 1.0f, 0.6f },  0.0f, 35.0f },
        ModelTransform{ Gs::Vector3f{ +1.5f, -0.5f, 4.0f }, Gs::Vector3f{ 0.4f, 0.5f, 0.6f }, Gs::Vector3f{ 0.6f, 0.6f, 1.0f }, 15.0f, 20.0f },
    };

    Buffer* sceneBuffers[numCmdBuffers] = {};

    for_range(i, numCmdBuffers)
    {
        const ModelTransform& transform = transforms[i];

        sceneConstants.solidColor = Gs::Vector4f{ transform.color, 1.0f };

        sceneConstants.wMatrix.LoadIdentity();
        Gs::Translate(sceneConstants.wMatrix, transform.origin);
        Gs::RotateFree(sceneConstants.wMatrix, Gs::Vector3f{ 1.0f, 0.0f, 0.0f }, transform.pitch);
        Gs::RotateFree(sceneConstants.wMatrix, Gs::Vector3f{ 0.0f, 1.0f, 0.0f }, transform.yaw);
        Gs::Scale(sceneConstants.wMatrix, transform.scale);

        BufferDescriptor sceneBufferDesc;
        {
            sceneBufferDesc.size        = sizeof(SceneConstants);
            sceneBufferDesc.bindFlags   = BindFlags::ConstantBuffer;
        }
        sceneBuffers[i] = renderer->CreateBuffer(sceneBufferDesc, &sceneConstants);
    }

    // Record secondary command buffers to render objects
    auto RecordMeshDrawCommand = [this, pso](CommandBuffer* innerCmdBuffer, const IndexedTriangleMesh& mesh, Buffer* sceneBuffer) -> void
    {
        innerCmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
        innerCmdBuffer->SetPipelineState(*pso);
        innerCmdBuffer->SetResource(0, *sceneBuffer);
        innerCmdBuffer->DrawIndexed(mesh.numIndices, 0);
    };

    auto RecordSecondaryCommandBuffer = [this, pso, &RecordMeshDrawCommand](CommandBuffer* secondaryCmdBuffer, const IndexedTriangleMesh& mesh, Buffer* sceneBuffer) -> void
    {
        secondaryCmdBuffer->Begin();
        {
            RecordMeshDrawCommand(secondaryCmdBuffer, mesh, sceneBuffer);
        }
        secondaryCmdBuffer->End();
    };

    CommandBuffer* secondaryCmdBuffers[numCmdBuffers] = {};

    for_range(i, numCmdBuffers)
    {
        CommandBufferDescriptor cmdBufferDesc;
        {
            cmdBufferDesc.flags             = CommandBufferFlags::Secondary;
            cmdBufferDesc.numNativeBuffers  = 1;
            cmdBufferDesc.renderPass        = swapChain->GetRenderPass(); // Continue rendering into render pass of primary command buffer
        }
        secondaryCmdBuffers[i] = renderer->CreateCommandBuffer(cmdBufferDesc);

        RecordSecondaryCommandBuffer(secondaryCmdBuffers[i], models[ModelCube], sceneBuffers[i]);
    }

    // Create readback texture
    const Extent2D resolution = swapChain->GetResolution();

    TextureDescriptor readbackTexDesc;
    {
        readbackTexDesc.bindFlags       = BindFlags::CopyDst;
        readbackTexDesc.format          = swapChain->GetColorFormat();
        readbackTexDesc.extent.width    = resolution.width;
        readbackTexDesc.extent.height   = resolution.height;
        readbackTexDesc.miscFlags       = MiscFlags::NoInitialData;
        readbackTexDesc.mipLevels       = 1;
    }
    Texture* readbackTex = renderer->CreateTexture(readbackTexDesc);

    // Record primary command buffer to render frame
    const TextureRegion texRegion{ Offset3D{}, readbackTexDesc.extent };

    cmdBuffer->Begin();
    {
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->Clear(ClearFlags::ColorDepth);
            cmdBuffer->SetViewport(resolution);
            for_range(i, numCmdBuffers)
            {
                // Draw meshes 0 and 2 with secondary command buffer, draw mesh 1 with primary command buffer
                if (i == 1)
                    RecordMeshDrawCommand(cmdBuffer, models[ModelCube], sceneBuffers[i]);
                else
                    cmdBuffer->Execute(*secondaryCmdBuffers[i]);
            }
            cmdBuffer->CopyTextureFromFramebuffer(*readbackTex, texRegion, Offset2D{});
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Read result from readback texture
    std::vector<ColorRGBub> readbackImage;
    readbackImage.resize(resolution.width * resolution.height);

    MutableImageView dstImageView;
    {
        dstImageView.format     = ImageFormat::RGB;
        dstImageView.dataType   = DataType::UInt8;
        dstImageView.data       = readbackImage.data();
        dstImageView.dataSize   = readbackImage.size() * sizeof(ColorRGBub);
    }
    renderer->ReadTexture(*readbackTex, texRegion, dstImageView);

    const std::string readbackImageName = "SecondaryCommandBuffer";
    SaveColorImage(readbackImage, resolution, readbackImageName);

    // Ignore single pixel differences because GL implementation of CIS server might produce slightyl different rasterization
    const DiffResult diff = DiffImages(readbackImageName, diffThreshold, diffTolerance);

    // Release resources
    for_range(i, numCmdBuffers)
    {
        renderer->Release(*secondaryCmdBuffers[i]);
        renderer->Release(*sceneBuffers[i]);
    }

    renderer->Release(*readbackTex);
    renderer->Release(*pso);

    return diff.Evaluate("secondary command buffer");
}


