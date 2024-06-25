/*
 * TestDualSourceBlending.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>


/*
Renders a fullscreen triangle directly to the swap-chain, i.e. only a single color attachment but the shader has two outputs.
This requires the dual-source blending feature, which is denoted as "Src1" in the blend states.

The shading-languages have different semantics to describe the respective blending outputs:
- HLSL uses the same semantic as if two color attachments where active:
    float4 colorA : SV_Target0;
    float4 colorB : SV_Target1;
- GLSL uses the same output location but with two different indices:
    layout(location = 0, index = 0) out vec4 colorA;
    layout(location = 0, index = 1) out vec4 colorB;
- Metal uses a similar semantic as GLSL:
    float4 colorA [[color(0), index(0)]];
    float4 colorB [[color(0), index(1)]];
*/
DEF_TEST( DualSourceBlending )
{
    if (shaders[VSDualSourceBlend] == nullptr || shaders[PSDualSourceBlend] == nullptr)
        return TestResult::Skipped;

    // Create all blend states
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(
        Parse(
            HasCombinedSamplers()
                ?   "texture(colorMapA@1,colorMapB@2):frag,sampler(1,2):frag"
                :   "texture(colorMapA@1,colorMapB@2):frag,sampler(3,4):frag"
        )
    );

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout                  = psoLayout;
        psoDesc.renderPass                      = swapChain->GetRenderPass();
        psoDesc.vertexShader                    = shaders[VSDualSourceBlend];
        psoDesc.fragmentShader                  = shaders[PSDualSourceBlend];
        psoDesc.blend.targets[0].blendEnabled   = true;
        psoDesc.blend.targets[0].srcColor       = BlendOp::One;
        psoDesc.blend.targets[0].dstColor       = BlendOp::Src1Color;
        psoDesc.blend.targets[0].srcAlpha       = BlendOp::One;
        psoDesc.blend.targets[0].dstAlpha       = BlendOp::Src1Alpha;
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoDualSourceBlend");

    Sampler* samplerA = renderer->CreateSampler(Parse("filter=linear"));
    Sampler* samplerB = renderer->CreateSampler(Parse("filter=linear")); // <-- Also linear filtering or CIS tests may fail (due to one-off pixels)

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
            cmdBuffer->SetResource(0, *textures[TexturePaintingA_NPOT]);
            cmdBuffer->SetResource(1, *textures[TextureGrid10x10]);
            cmdBuffer->SetResource(2, *samplerA);
            cmdBuffer->SetResource(3, *samplerB);

            cmdBuffer->Draw(3, 0);

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Match entire color buffer and create delta heat map
    const char* colorBufferName = "DualSourceBlend";

    SaveCapture(readbackTex, colorBufferName);

    constexpr int threshold = 12; // Accept threshold of 12 to avoid failure on CIS server; seen consistent diffs of 4 or 12 across multiple backends
    const DiffResult diff = DiffImages(colorBufferName, threshold);

    // Clear resources
    renderer->Release(*samplerA);
    renderer->Release(*samplerB);
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    return diff.Evaluate("dual source blending");
}

