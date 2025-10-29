/*
 * TestAlphaOnlyTexture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <LLGL/Utils/TypeNames.h>


/*
Loads a texture in A8Unorm format and renders its alpha channel as gray-scaled color to the screen.
This test ensure A8Unorm textures are loaded correctly into the GPU textures,
since some backends such as OpenGL have to emulate this format via texture component swizzling.
*/
DEF_TEST( AlphaOnlyTexture )
{
    if (shaders[VSAlphaOnlyTexture] == nullptr || shaders[PSAlphaOnlyTexture] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Create PSO for alpha-only texture rendering
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(
        Parse("texture(colorMap@1):frag,sampler(texSampler@2):frag")
    );

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout  = psoLayout;
        psoDesc.renderPass      = swapChain->GetRenderPass();
        psoDesc.vertexShader    = shaders[VSAlphaOnlyTexture];
        psoDesc.fragmentShader  = shaders[PSAlphaOnlyTexture];
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoAlphaOnlyTexture");

    // Load texture with A8UNorm format
    const char* texNameA = "AlphaChannelTex_A";

    const Format expectedTexFormat = LLGL::Format::A8UNorm;
    Texture* texA8UNorm_A = LoadTextureFromFile(texNameA, "AlphaChannel.png", expectedTexFormat);

    const Format actualTexFormat = texA8UNorm_A->GetFormat();
    if (actualTexFormat != expectedTexFormat)
    {
        Log::Errorf(
            "Expected texture '%s' to have format LLGL::%s, but actual format is LLGL::%s\n",
            texNameA, LLGL::ToString(expectedTexFormat), LLGL::ToString(actualTexFormat)
        );
        return TestResult::FailedErrors;
    }

    // Create texture with A8UNorm format and fill image data separately
    TextureDescriptor texBDesc;
    {
        texBDesc.extent     = texA8UNorm_A->GetMipExtent(0);
        texBDesc.format     = Format::A8UNorm;
        texBDesc.mipLevels  = 1;
    }
    CREATE_TEXTURE(texA8UNorm_B, texBDesc, "AlphaChannelTex_B", nullptr);

    // Write same image into texture via WriteTexture()
    {
        Image imgB = TestbedContext::LoadImageFromFile(textureDir + "AlphaChannel.png", opt.verbose, ImageFormat::RGBA);
        if (imgB.GetData() == nullptr)
            return TestResult::FailedErrors;
        const TextureRegion texB_Region{ Offset3D{}, imgB.GetExtent() };
        renderer->WriteTexture(*texA8UNorm_B, texB_Region, imgB.GetView());
    }

    // Render scene
    Texture* readbackTex = nullptr;

    const LLGL::Extent2D resolution = swapChain->GetResolution();
    const LLGL::Extent2D halfResolution{ resolution.width / 2, resolution.height };

    cmdBuffer->Begin();
    {
        cmdBuffer->SetVertexBuffer(*meshBuffer); // Dummy vertex buffer

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->Clear(ClearFlags::Color, ClearValue{ 1.0f, 0.0f, 0.0f, 1.0f });
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetResource(1, *samplers[SamplerLinearNoMips]);

            // Draw first texture into left side of the screen
            cmdBuffer->SetViewport(Viewport{ Offset2D{ 0, 0 }, halfResolution });
            cmdBuffer->SetResource(0, *texA8UNorm_A);
            cmdBuffer->Draw(3, 0);

            // Draw second texture into right side of the screen
            cmdBuffer->SetViewport(Viewport{ Offset2D{ static_cast<std::int32_t>(halfResolution.width), 0 }, halfResolution });
            cmdBuffer->SetResource(0, *texA8UNorm_B);
            cmdBuffer->Draw(3, 0);

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Match entire color buffer and create delta heat map
    const char* colorBufferName = "AlphaOnlyTexture";

    SaveCapture(readbackTex, colorBufferName);

    constexpr int threshold = 12; // Accept threshold of 12 to avoid failure on CIS server; seen consistent diffs of 4 or 12 across multiple backends
    const DiffResult diff = DiffImages(colorBufferName, threshold);

    // Clear resources
    renderer->Release(*texA8UNorm_A);
    renderer->Release(*texA8UNorm_B);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    return diff.Evaluate("alpha-only texture");
}

