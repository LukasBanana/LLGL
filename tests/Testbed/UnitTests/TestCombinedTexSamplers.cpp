/*
* TestCombinedTexSamplers.cpp
*
* Copyright (c) 2015 Lukas Hermanns. All rights reserved.
* Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
*/

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Scale.h>


/*
Render a scene with 3 textures and 2 sampler-states with 4 unique combinations:
TexA + SamplerA, TexB + SamplerA, TexB + SamplerB, TexC + SamplerB.
This requires the new array for combined texture-sampler since GLSL with OpenGL semantics does not support separate samplers.
*/
DEF_TEST( CombinedTexSamplers )
{
    //TODO: not supported for Vulkan and Metal yet
    if (renderer->GetRendererID() != RendererID::OpenGL &&
        renderer->GetRendererID() != RendererID::Direct3D11 &&
        renderer->GetRendererID() != RendererID::Direct3D12)
    {
        return TestResult::Skipped;
    }

    if (shaders[VSCombinedSamplers] == nullptr || shaders[PSCombinedSamplers] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Create PSO layout
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(
        Parse(
            // Heap resource bindings
            "heap{"
            "  texture(colorMapA@2):frag,"
            "  texture(colorMapB@3):frag,"
            "  sampler(texSamplerA@5):frag,"
            "},"

            // Dynamic resource bindings
            "cbuffer(Scene@1):vert,"
            "texture(colorMapC@4):frag,"
            "sampler(texSamplerB@6):frag,"

            // Combined texture-samplers
            "sampler<colorMapA, texSamplerA>(colorMapA_texSamplerA@2),"
            "sampler<colorMapB, texSamplerA>(colorMapB_texSamplerA@3),"
            "sampler<colorMapB, texSamplerB>(colorMapB_texSamplerB@4),"
            "sampler<colorMapC, texSamplerB>(colorMapC_texSamplerB@5),"
        )
    );

    // Create graphics PSO
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = psoLayout;
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSCombinedSamplers];
        psoDesc.fragmentShader      = shaders[PSCombinedSamplers];
        psoDesc.depth.testEnabled   = true;
        psoDesc.depth.writeEnabled  = true;
        psoDesc.rasterizer.cullMode = CullMode::Back;
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoCombinedSamplers");

    // Create resource heap and use samplers with no MIP-mapping (we don't want to test MIP-maps here)
    ResourceHeap* resHeap = renderer->CreateResourceHeap(
        psoLayout,
        {
            // Left rectangle resources:
            textures[TexturePaintingA_NPOT], textures[TextureDetailMap],
            samplers[SamplerLinearNoMips], // No MIPs due to NPOT texture

            // Right rectangle resources:
            textures[TexturePaintingB], textures[TextureDetailMap],
            samplers[SamplerNearest],
        }
    );

    // Update scene constants
    sceneConstants = SceneConstants{};

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    Gs::Scale(vMatrix, Gs::Vector3f{ 0.5f, 1, 1 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;

    // Render scene
    Texture* readbackTex = nullptr;

    const IndexedTriangleMesh& mesh = models[ModelRect];

    const Extent2D halfResolution{ opt.resolution.width/2, opt.resolution.height };

    cmdBuffer->Begin();
    {
        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));

        // Graphics can be set inside and outside a render pass, so test binding this PSO outside the render pass
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
        cmdBuffer->SetPipelineState(*pso);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::ColorDepth);
            cmdBuffer->SetResource(0, *sceneCbuffer);

            // Draw left rectangle
            cmdBuffer->SetViewport(Viewport{ Offset2D{ 0, 0 }, halfResolution });
            cmdBuffer->SetResourceHeap(*resHeap, 0);
            cmdBuffer->SetResource(1, *textures[TextureGrid10x10]); // colorMapC affects one combined samplers: "colorMapC_texSamplerB"
            cmdBuffer->SetResource(2, *samplers[SamplerNearest]);   // texSamplerB affects two combined samplers: "colorMapB_texSamplerB", "colorMapC_texSamplerB"
            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Draw right rectangle
            cmdBuffer->SetViewport(Viewport{ Offset2D{ static_cast<std::int32_t>(halfResolution.width), 0 }, halfResolution });
            cmdBuffer->SetResourceHeap(*resHeap, 1);
            cmdBuffer->SetResource(1, *textures[TextureGradient]);  // colorMapC affects one combined samplers: "colorMapC_texSamplerB"
            cmdBuffer->SetResource(2, *samplers[SamplerLinear]);    // texSamplerB affects two combined samplers: "colorMapB_texSamplerB", "colorMapC_texSamplerB"
            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Evaluate readback result
    SaveCapture(readbackTex, "CombinedSamplers");

    constexpr int threshold = 12; // Tolerate a threshold of 12 color values
    constexpr unsigned tolerance = 300; // Tolerate a rather high number of outliers due to differences in hardware samplers (even within the same backend)
    const DiffResult diff = DiffImages("CombinedSamplers", threshold, tolerance);

    TestResult result = diff.Evaluate("combined samplers");

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*psoLayout);
    renderer->Release(*resHeap);

    return result;
}

