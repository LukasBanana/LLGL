/*
 * TestVertexBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


/*
Renders simple geometry multiple times with a different vertex format each iteration but from the same vertex buffer source.
This tests if changing the vertex buffer attributes during command recording works correctly.
*/
DEF_TEST( VertexBuffer )
{
    constexpr unsigned numFrames = 4;

    static TestResult result = TestResult::Passed;
    static Buffer* vertexBuffer;

    const Gs::Vector4f solidColors[numFrames] =
    {
        Gs::Vector4f{ 0, 1, 0, 1 },
        Gs::Vector4f{ 1, 0, 1, 1 },
        Gs::Vector4f{ 1, 1, 1, 1 }, // White to let vertex colors paint the picture
        Gs::Vector4f{ 1, 1, 0, 1 },
    };

    const Simple2DVertex simple2DVertices[4] =
    {
        { -1.5f, +1.25f },
        { -1.5f, -1.25f },
        { +1.5f, +1.25f },
        { +1.5f, -1.25f },
    };

    const InterleavedVertex interleavedVertices[] =
    {
        InterleavedVertex{ { -1.0f, +1.0f }, { -2.0f, +1.5f }, { 255,   0,   0, 255 } },
        InterleavedVertex{ { -1.0f, -1.0f }, { -2.0f, -1.5f }, {   0, 255,   0, 255 } },
        InterleavedVertex{ { +1.0f, +1.0f }, { +2.0f, +1.5f }, {   0,   0, 255, 255 } },
        InterleavedVertex{ { +1.0f, -1.0f }, { +2.0f, -1.5f }, { 255,   0, 255, 255 } },
    };

    if (frame == 0)
    {
        if (shaders[VSVertexFormat0] == nullptr ||
            shaders[VSVertexFormat1] == nullptr ||
            shaders[VSVertexFormat2] == nullptr ||
            shaders[VSVertexFormat3] == nullptr ||
            shaders[PSVertexFormat] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        // Create vertex buffer
        BufferDescriptor bufDesc;
        {
            bufDesc.size            = sizeof(interleavedVertices);
            bufDesc.bindFlags       = BindFlags::VertexBuffer | BindFlags::CopyDst;
            bufDesc.vertexAttribs   = vertexFormats[VertFmtLayout0].attributes;
        }
        vertexBuffer = renderer->CreateBuffer(bufDesc, interleavedVertices);
    }

    // Create PSO
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = layouts[PipelineSolid];
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSVertexFormat0 + frame];
        psoDesc.fragmentShader      = shaders[PSVertexFormat];
        psoDesc.primitiveTopology   = PrimitiveTopology::TriangleStrip;
    }
    const std::string psoName = "psoVertexFormat" + std::to_string(frame);
    CREATE_GRAPHICS_PSO(pso, psoDesc, psoName.c_str());

    // Update scene constants
    sceneConstants = SceneConstants{};

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    auto TransformWorldMatrix = [](Gs::Matrix4f& wMatrix, float posX, float posY, float scale)
    {
        wMatrix.LoadIdentity();
        Gs::Translate(wMatrix, Gs::Vector3f{ posX, posY, 2.0f });
        Gs::Scale(wMatrix, Gs::Vector3f{ scale });
    };

    // Render scene
    Texture* readbackTex = nullptr;

    BEGIN();
    {
        if (frame + 1 == numFrames)
        {
            // Last frame uses the first format but with smaller stride
            cmdBuffer->FillBuffer(*vertexBuffer, 0, 0x00000000);
            cmdBuffer->UpdateBuffer(*vertexBuffer, 0, simple2DVertices, sizeof(simple2DVertices));
        }

        // Set vertex buffer with new attributes for each frame
        const VertexFormat& vertexFormat = vertexFormats[VertFmtLayout0 + frame];
        cmdBuffer->SetVertexBuffer(*vertexBuffer, static_cast<std::uint32_t>(vertexFormat.attributes.size()), vertexFormat.attributes.data());

        cmdBuffer->SetPipelineState(*pso);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::ColorDepth);
            cmdBuffer->SetViewport(opt.resolution);
            cmdBuffer->SetResource(0, *sceneCbuffer);

            // Draw rectangle as triangle strip
            sceneConstants.solidColor = solidColors[frame];
            TransformWorldMatrix(sceneConstants.wMatrix, 0.0f, 0.0f, 1.0f);

            cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
            cmdBuffer->Draw(4, 0);

            // Capture framebuffer in last iteration
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    END();

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "VertexBuffer_Format" + std::to_string(frame);

    SaveCapture(readbackTex, colorBufferName);

    const DiffResult diff = DiffImages(colorBufferName);

    // Evaluate readback result
    TestResult intermediateResult = diff.Evaluate("vertex buffer format", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    // Clear resources
    renderer->Release(*pso);

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    renderer->Release(*vertexBuffer);

    return result;
}

