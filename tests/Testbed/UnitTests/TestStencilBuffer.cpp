/*
 * TestStencilBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/ProjectionMatrix4.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <limits.h>


DEF_TEST( StencilBuffer )
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
        texDesc.format          = Format::D24UNormS8UInt;
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

    // Create PSO for rendering to the stencil buffer with dynamic reference value
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout              = layouts[PipelineSolid];
        psoDesc.renderPass                  = renderTarget->GetRenderPass();
        psoDesc.vertexShader                = shaders[VSSolid];
        psoDesc.stencil.testEnabled         = true;
        psoDesc.stencil.referenceDynamic    = true;
        psoDesc.stencil.front.compareOp     = LLGL::CompareOp::Greater;
        psoDesc.stencil.front.stencilFailOp = LLGL::StencilOp::Keep;
        psoDesc.stencil.front.depthFailOp   = LLGL::StencilOp::Keep;
        psoDesc.stencil.front.depthPassOp   = LLGL::StencilOp::Replace;
        psoDesc.rasterizer.cullMode         = CullMode::Back;
        psoDesc.blend.targets[0].colorMask  = 0; // Disable rasterize with colorMask=0 since we don't use a fragment shader
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoStencilBuf");

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
    constexpr std::uint32_t stencilRef = 50;
    static_assert(stencilRef <= UINT8_MAX, "'stencilRef' must not be greater than UINT8_MAX");

    cmdBuffer->Begin();
    {
        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
        cmdBuffer->BeginRenderPass(*renderTarget);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::Stencil);
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetStencilReference(stencilRef);
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

    constexpr std::uint8_t invalidStencilValue = 0xFF;

    std::uint8_t readbackStencilValue = invalidStencilValue;

    MutableImageView dstImageView;
    {
        dstImageView.format     = ImageFormat::Stencil;
        dstImageView.data       = &readbackStencilValue;
        dstImageView.dataSize   = sizeof(readbackStencilValue);
        dstImageView.dataType   = DataType::UInt8;
    }
    renderer->ReadTexture(*readbackTex, readbackTexRegion, dstImageView);

    const int deltaStencilValue = std::abs(static_cast<int>(readbackStencilValue) - static_cast<int>(stencilRef));

    // Match entire depth and create delta heat map
    std::vector<std::uint8_t> readbackStencilBuffer;
    readbackStencilBuffer.resize(texDesc.extent.width * texDesc.extent.height, 0);
    {
        dstImageView.format     = ImageFormat::Stencil;
        dstImageView.data       = readbackStencilBuffer.data();
        dstImageView.dataSize   = sizeof(decltype(readbackStencilBuffer)::value_type) * readbackStencilBuffer.size();
        dstImageView.dataType   = DataType::UInt8;
    }
    renderer->ReadTexture(*readbackTex, TextureRegion{ Offset3D{}, texDesc.extent }, dstImageView);

    SaveStencilImage(readbackStencilBuffer, opt.resolution, "StencilBuffer_Set50");

    const DiffResult diff = DiffImages("StencilBuffer_Set50");

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*renderTarget);
    renderer->Release(*readbackTex);

    // Evaluate readback result
    if (readbackStencilValue == invalidStencilValue)
    {
        Log::Errorf("Failed to read back value from stencil buffer texture at center\n");
        return TestResult::FailedErrors;
    }
    if (deltaStencilValue > 0)
    {
        Log::Errorf(
            "Mismatch between stencil buffer value at center (%d) and expected value (%d): delta = %d\n",
            static_cast<int>(readbackStencilValue), static_cast<int>(stencilRef), deltaStencilValue
        );
        return TestResult::FailedMismatch;
    }

    return diff.Evaluate("stencil buffer");
}

