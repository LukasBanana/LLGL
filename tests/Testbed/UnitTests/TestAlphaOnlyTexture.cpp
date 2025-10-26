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
    const char* texName = "AlphaChannel";

    const LLGL::Format expectedTexFormat = LLGL::Format::A8UNorm;
    LLGL::Texture* texA8UNorm = LoadTextureFromFile(texName, "AlphaChannel.png", expectedTexFormat);

    const LLGL::Format actualTexFormat = texA8UNorm->GetFormat();
    if (actualTexFormat != expectedTexFormat)
    {
        Log::Errorf(
            "Expected texture '%s' to have format LLGL::%s, but actual format is LLGL::%s\n",
            texName, LLGL::ToString(expectedTexFormat), LLGL::ToString(actualTexFormat)
        );
        return TestResult::FailedErrors;
    }

    // Render scene
    Texture* readbackTex = nullptr;

    cmdBuffer->Begin();
    {
        cmdBuffer->SetVertexBuffer(*meshBuffer); // Dummy vertex buffer

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw fullscreen traingle
            cmdBuffer->SetViewport(swapChain->GetResolution());
            cmdBuffer->Clear(LLGL::ClearFlags::Color, LLGL::ClearValue{ 1.0f, 1.0f, 1.0f, 1.0f });

            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetResource(0, *texA8UNorm);
            cmdBuffer->SetResource(1, *samplers[SamplerLinearClamp]);

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
    renderer->Release(*texA8UNorm);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    return diff.Evaluate("alpha-only texture");
}

