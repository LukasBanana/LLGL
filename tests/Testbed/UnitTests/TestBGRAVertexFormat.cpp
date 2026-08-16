/*
 * TestBGRAVertexFormat.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/Parse.h>


// Tests that BGRA formats (e.g. LLGL::Format::BGRA8UNorm) work with vertex input assembly.
// This is an edge case for OpenGL vertex attribute declaration and needs the `GL_ARB_vertex_array_bgra` extension.
// Renders a triangle in two viewports each, left one is RGBA vertex input and right one is BGRA vertex input.
DEF_TEST( BGRAVertexFormat )
{
    // Skip if BGRA vertex formats are not supported by backend
    if (shaders[VSUnprojectedBGRA] == nullptr)
    {
        if (opt.verbose)
            Log::Printf("BGRA vertex format not supported\n");
        return TestResult::Skipped;
    }

    // Create unprojected 2D vertices in BGRA color formats
    UnprojectedVertex vertices[3] =
    {
        UnprojectedVertex{ {  0.00f, +0.5f }, { 255, 0, 0, 255 } }, // blue
        UnprojectedVertex{ { +0.75f, -0.5f }, { 0, 255, 0, 255 } }, // green
        UnprojectedVertex{ { -0.75f, -0.5f }, { 0, 0, 255, 255 } }, // red
    };

    BufferDescriptor vertexBufDesc;
    {
        vertexBufDesc.size          = sizeof(vertices);
        vertexBufDesc.bindFlags     = BindFlags::VertexBuffer;
        vertexBufDesc.vertexAttribs = vertexFormats[VertFmtUnprojected].attributes;
    }
    CREATE_BUFFER(vertexBufRGBA, vertexBufDesc, "vertices2DRGBA", vertices);
    {
        vertexBufDesc.vertexAttribs = vertexFormats[VertFmtUnprojectedBGRA].attributes;
    }
    CREATE_BUFFER(vertexBufBGRA, vertexBufDesc, "vertices2DBGRA", vertices);

    // Create PSO for rendering triangle strips
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = nullptr; // No resource bindings, therefore no pipeline layout
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSUnprojected];
        psoDesc.fragmentShader      = shaders[PSUnprojected];
        psoDesc.primitiveTopology   = PrimitiveTopology::TriangleList;
    }
    CREATE_GRAPHICS_PSO(psoRGBA, psoDesc, "Test.BGRAVertexFormat.RGBAPso");
    {
        psoDesc.vertexShader        = shaders[VSUnprojectedBGRA];
    }
    CREATE_GRAPHICS_PSO(psoBGRA, psoDesc, "Test.BGRAVertexFormat.BGRAPso");

    Texture* readbackTex = nullptr;
    const float resWidthHalf = static_cast<float>(opt.resolution.width/2);
    const float resHeight = static_cast<float>(opt.resolution.height);

    // Render scene
    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->Clear(ClearFlags::Color);
            {
                // Draw RGBA vertices
                cmdBuffer->SetViewport(Viewport{ 0.0f, 0.0f, resWidthHalf, resHeight });
                cmdBuffer->SetPipelineState(*psoRGBA);
                cmdBuffer->SetVertexBuffer(*vertexBufRGBA);
                cmdBuffer->Draw(3, 0);

                // Draw BGRA vertices
                cmdBuffer->SetViewport(Viewport{ resWidthHalf, 0.0f, resWidthHalf, resHeight });
                cmdBuffer->SetPipelineState(*psoBGRA);
                cmdBuffer->SetVertexBuffer(*vertexBufBGRA);
                cmdBuffer->Draw(3, 0);
            }
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    END();

    // Diff color buffer for BGRA colored vertices
    const std::string colorBufferName = "BGRAVertexFormat";
    SaveCapture(readbackTex, colorBufferName);
    const DiffResult diff = DiffImages(colorBufferName);

    TestResult result = diff.Evaluate("BGRA vertex format");

    // Clear resources
    renderer->Release(*psoRGBA);
    renderer->Release(*psoBGRA);

    return result;
}

