/*
 * TestMultiview.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <algorithm>


/*
Verifies multiview (single-pass layered) rendering: a single draw is broadcast to every view of a multiview
render pass, and each view i writes into array layer i of the render target. The shader colors each view
differently by its view index (view 0 -> red, view 1 -> green), so reading back each layer confirms both that
multiview routed each view to the correct layer and that the per-view index reached the shader.

Skipped on backends/devices without multiview support (RenderingFeatures::hasMultiview). Only Vulkan
(VK_KHR_multiview) and Direct3D 12 (view instancing, requires DXC) report it.
*/
DEF_TEST( Multiview )
{
    if (!caps.features.hasMultiview || caps.limits.maxViews < 2)
        return TestResult::Skipped;

    constexpr std::uint32_t numViews = 2;
    const Extent2D resolution{ 16, 16 };

    // Two-layer color array texture, one layer per view. CopySrc so we can read each layer back.
    TextureDescriptor texDesc;
    {
        texDesc.type        = TextureType::Texture2DArray;
        texDesc.format      = Format::RGBA8UNorm;
        texDesc.extent      = { resolution.width, resolution.height, 1 };
        texDesc.arrayLayers = numViews;
        texDesc.mipLevels   = 1;
        texDesc.bindFlags   = BindFlags::ColorAttachment | BindFlags::CopySrc;
    }
    CREATE_TEXTURE(colorTex, texDesc, "multiviewColor{rgba8,2-layer}", nullptr);

    // Multiview render target: 'views' makes the backend build a multiview render pass (Vulkan view mask /
    // Direct3D 12 view instancing) that broadcasts a single draw to both array layers.
    RenderTargetDescriptor targetDesc;
    {
        targetDesc.resolution          = resolution;
        targetDesc.views               = numViews;
        targetDesc.colorAttachments[0] = colorTex;
    }
    CREATE_RENDER_TARGET(target, targetDesc, "multiviewTarget{2-view}");

    // Multiview shaders. The vertex shader is a full-screen triangle (no vertex buffer) whose color depends on
    // the view index. Only HLSL (SM 6.1) and SPIR-V variants exist, matching the multiview-capable backends.
    Shader* vertShader = nullptr;
    Shader* fragShader = nullptr;

    if (IsShadingLanguageSupported(ShadingLanguage::HLSL_6_1))
    {
        vertShader = LoadShaderFromFile("Multiview.hlsl", ShaderType::Vertex,   "VSMain", "vs_6_1", nullptr, VertFmtEmpty);
        fragShader = LoadShaderFromFile("Multiview.hlsl", ShaderType::Fragment, "PSMain", "ps_6_1");
    }
    else if (IsShadingLanguageSupported(ShadingLanguage::SPIRV))
    {
        vertShader = LoadShaderFromFile("Multiview.450core.vert.spv", ShaderType::Vertex,   nullptr, nullptr, nullptr, VertFmtEmpty);
        fragShader = LoadShaderFromFile("Multiview.450core.frag.spv", ShaderType::Fragment, nullptr, nullptr);
    }

    if (vertShader == nullptr || fragShader == nullptr)
    {
        // The shaders need Shader Model 6.1 (SV_ViewID). hasMultiview reports the device's view-instancing
        // support, but this test compiles its shaders at runtime, which for HLSL needs DXC (dxcompiler.dll). When
        // that runtime compiler is unavailable (e.g. CI on the Basic Render Driver), the feature is supported but
        // the test can't build its shaders here -- skip rather than fail. (Coverage still comes from the SPIR-V
        // path and from D3D12 environments where DXC is present.)
        Log::Printf("Skipping Multiview: could not build multiview shaders (runtime SM 6.1 / DXC unavailable?)\n");
        return TestResult::Skipped;
    }

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.vertexShader    = vertShader;
        psoDesc.fragmentShader  = fragShader;
        psoDesc.renderPass      = target->GetRenderPass();
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "multiviewPSO");

    // Single draw, broadcast to both views/layers.
    BEGIN();
    {
        cmdBuffer->BeginRenderPass(*target);
        {
            cmdBuffer->Clear(ClearFlags::Color, ClearValue{ 0.0f, 0.0f, 0.0f, 1.0f });
            cmdBuffer->SetViewport(resolution);
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->Draw(3, 0);
        }
        cmdBuffer->EndRenderPass();
    }
    END();

    // Read back the center pixel of each layer and verify view i -> layer i with the expected per-view color.
    TestResult result = TestResult::Passed;
    const std::uint8_t expectedColors[numViews][4] =
    {
        { 255,   0,   0, 255 }, // view 0 -> red
        {   0, 255,   0, 255 }, // view 1 -> green
    };

    for_range(view, numViews)
    {
        TextureRegion region;
        {
            region.subresource  = TextureSubresource{ /*baseArrayLayer:*/ view, /*numArrayLayers:*/ 1u, /*baseMipLevel:*/ 0u, /*numMipLevels:*/ 1u };
            region.offset       = { static_cast<std::int32_t>(resolution.width / 2), static_cast<std::int32_t>(resolution.height / 2), 0 };
            region.extent       = { 1, 1, 1 };
        }

        std::uint8_t pixel[4] = {};
        MutableImageView dstView;
        {
            dstView.format      = ImageFormat::RGBA;
            dstView.dataType    = DataType::UInt8;
            dstView.data        = pixel;
            dstView.dataSize    = sizeof(pixel);
        }
        renderer->ReadTexture(*colorTex, region, dstView);

        if (!IsRGBA8ubInThreshold(pixel, expectedColors[view], 2))
        {
            Log::Errorf(
                "Multiview layer %u mismatch: got (%u, %u, %u, %u), expected (%u, %u, %u, %u)\n",
                view,
                pixel[0], pixel[1], pixel[2], pixel[3],
                expectedColors[view][0], expectedColors[view][1], expectedColors[view][2], expectedColors[view][3]
            );
            result = TestResult::FailedMismatch;
        }
    }

    // Cleanup
    renderer->Release(*pso);
    renderer->Release(*target);
    renderer->Release(*colorTex);
    renderer->Release(*vertShader);
    renderer->Release(*fragShader);

    return result;
}
