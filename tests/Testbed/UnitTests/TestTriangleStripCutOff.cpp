/*
 * TestTriangleStripCutOff.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/Parse.h>


DEF_TEST( TriangleStripCutOff )
{
    // Create unprojected 2D vertices
    auto GenerateRectVertices = [](UnprojectedVertex* vertices, float centerX, float centerY, float size) -> void
    {
        const float left    = centerX - size / 2.0f;
        const float right   = centerX + size / 2.0f;
        const float top     = centerY + size / 2.0f;
        const float bottom  = centerY - size / 2.0f;

        vertices[0] = UnprojectedVertex{ { right, top    }, { 255,   0,   0, 255 } };
        vertices[1] = UnprojectedVertex{ { right, bottom }, {   0, 255,   0, 255 } };
        vertices[2] = UnprojectedVertex{ { left,  top    }, {   0,   0, 255, 255 } };
        vertices[3] = UnprojectedVertex{ { left,  bottom }, { 255, 255, 255, 255 } };
    };

    UnprojectedVertex vertices[16];
    GenerateRectVertices(&vertices[ 0], +0.5f, +0.5f, 0.8f);
    GenerateRectVertices(&vertices[ 4], -0.5f, +0.5f, 0.8f);
    GenerateRectVertices(&vertices[ 8], +0.5f, -0.5f, 0.8f);
    GenerateRectVertices(&vertices[12], -0.5f, -0.5f, 0.8f);

    BufferDescriptor vertexBufDesc;
    {
        vertexBufDesc.size          = sizeof(vertices);
        vertexBufDesc.bindFlags     = BindFlags::VertexBuffer;
        vertexBufDesc.vertexAttribs = vertexFormats[VertFmtUnprojected].attributes;
    }
    CREATE_BUFFER(vertexBuf, vertexBufDesc, "vertices2D", vertices);

    // Create 16-bit and 32-bit indices into single buffer
    std::uint16_t indicesUI16[] = { 0x0, 0x1, 0x2, 0x3, 0xFFFF,     0x4, 0x5, 0x6, 0x7, 0xFFFF     };
    std::uint32_t indicesUI32[] = { 0x8, 0x9, 0xA, 0xB, 0xFFFFFFFF, 0xC, 0xD, 0xE, 0xF, 0xFFFFFFFF };

    constexpr std::uint64_t indicesUI16Offset = 0;
    constexpr std::uint64_t indicesUI32Offset = sizeof(indicesUI16);
    constexpr std::uint32_t numUI16Indices = sizeof(indicesUI16)/sizeof(indicesUI16[0]);
    constexpr std::uint32_t numUI32Indices = sizeof(indicesUI32)/sizeof(indicesUI32[0]);

    BufferDescriptor indedBufDesc;
    {
        indedBufDesc.size       = sizeof(indicesUI16) + sizeof(indicesUI32);
        indedBufDesc.bindFlags  = BindFlags::IndexBuffer;
    }
    CREATE_BUFFER(indexBuf, indedBufDesc, "indices2D", nullptr);

    renderer->WriteBuffer(*indexBuf, indicesUI16Offset, indicesUI16, sizeof(indicesUI16));
    renderer->WriteBuffer(*indexBuf, indicesUI32Offset, indicesUI32, sizeof(indicesUI32));

    // Create PSO for rendering triangle strips
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = nullptr; // No resource bindings, therefore no pipeline layout
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSUnprojected];
        psoDesc.fragmentShader      = shaders[PSUnprojected];
        psoDesc.primitiveTopology   = PrimitiveTopology::TriangleStrip;
    }

    constexpr int numFormats = 3;
    constexpr Format indexFormats[numFormats] = { Format::Undefined, Format::R16UInt, Format::R32UInt };
    PipelineState* pso[numFormats] = {};

    for_range(i, numFormats)
    {
        psoDesc.indexFormat = indexFormats[i];
        const std::string psoName = "Test.StripCutOff.Format(" + std::string(ToString(indexFormats[i])) + ")";
        CREATE_GRAPHICS_PSO_EXT(pso[i], psoDesc, psoName.c_str());
    }

    Texture* readbackTex[2] = {};

    // Render scene
    cmdBuffer->Begin();
    {
        cmdBuffer->SetVertexBuffer(*vertexBuf);
        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->SetViewport(opt.resolution);

            // Draw first capture with undefined index format
            cmdBuffer->Clear(ClearFlags::Color);
            {
                cmdBuffer->SetPipelineState(*pso[0]);

                cmdBuffer->SetIndexBuffer(*indexBuf, Format::R16UInt, indicesUI16Offset);
                cmdBuffer->DrawIndexed(numUI16Indices, 0);

                cmdBuffer->SetIndexBuffer(*indexBuf, Format::R32UInt, indicesUI32Offset);
                cmdBuffer->DrawIndexed(numUI32Indices, 0);
            }
            readbackTex[0] = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);

            // Draw second capture with fixed index format R16UInt (pso[1]) and R32UInt (pso[2])
            cmdBuffer->Clear(ClearFlags::Color);
            {
                cmdBuffer->SetPipelineState(*pso[1]);
                cmdBuffer->SetIndexBuffer(*indexBuf, Format::R16UInt, indicesUI16Offset);
                cmdBuffer->DrawIndexed(numUI16Indices, 0);

                cmdBuffer->SetPipelineState(*pso[2]);
                cmdBuffer->SetIndexBuffer(*indexBuf, Format::R32UInt, indicesUI32Offset);
                cmdBuffer->DrawIndexed(numUI32Indices, 0);
            }
            readbackTex[1] = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Diff color buffer for undefined and fixed index formats
    bool diffFailed = false;

    const std::string colorBuffer0Name = "TriangleStrip_UndefinedFormat";
    SaveCapture(readbackTex[0], colorBuffer0Name);
    const DiffResult diff0 = DiffImages(colorBuffer0Name);

    TestResult result0 = diff0.Evaluate("triangle strip with undefined format");
    if (result0 != TestResult::Passed)
        diffFailed = true;

    const std::string colorBuffer1Name = "TriangleStrip_FixedFormat";
    SaveCapture(readbackTex[1], colorBuffer1Name);
    const DiffResult diff1 = DiffImages(colorBuffer1Name);

    TestResult result1 = diff1.Evaluate("triangle strip with fixed format");
    if (result1 != TestResult::Passed)
        diffFailed = true;

    // Clear resources
    for_range(i, numFormats)
        renderer->Release(*pso[i]);

    return (diffFailed ? TestResult::FailedMismatch : TestResult::Passed);
}

