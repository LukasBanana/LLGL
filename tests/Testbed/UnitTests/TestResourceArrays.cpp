/*
* TestResourceArrays.cpp
*
* Copyright (c) 2015 Lukas Hermanns. All rights reserved.
* Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
*/

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Scale.h>


/*
Renders some geometry (two rectangles into separate viewports) with more than one texture (only two textures right now).
The test must ensure that the texture resources are bound as an array in the shader, e.g. "sampler2D myTextures[2];" in GLSL.
Such resource arrays must be bound with a ResourceHeap as LLGL does not allow such arrays with individual descriptors.
*/
DEF_TEST( ResourceArrays )
{
    //TODO: temporarily disable this test for Metal as it's currently not supported
    if (renderer->GetRendererID() == RendererID::Metal)
        return TestResult::Skipped;

    if (shaders[VSResourceArrays] == nullptr || shaders[PSResourceArrays] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Create PSO layout
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(
        Parse(
            "cbuffer(Scene@1):vert:frag,"           // Bind individual resource for scene constant buffer
            "heap{"
            "  texture(colorMaps@2[2]):frag,"       // Declare a texture array with 2 elements
            "  sampler(texSamplers@%u[2]):frag,"    // Declare a sampler array with 2 elements
            "}",
            (HasCombinedSamplers() ? 2 : 4)         // GL needs to bind the sampler at the same binding slots
        )
    );

    // Create graphics PSO
    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = psoLayout;
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSResourceArrays];
        psoDesc.fragmentShader      = shaders[PSResourceArrays];
        psoDesc.depth.testEnabled   = true;
        psoDesc.depth.writeEnabled  = true;
        psoDesc.rasterizer.cullMode = CullMode::Back;
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoResourceArrays");

    // Create resource heap and use samplers with no MIP-mapping (we don't want to test MIP-maps here)
    ResourceHeap* resHeap = renderer->CreateResourceHeap(
        psoLayout,
        {
            // Left box resources:
            textures[TexturePaintingA_NPOT], textures[TextureDetailMap],
            samplers[SamplerLinearNoMips], samplers[SamplerNearestNoMips],

            // Right box resources:
            textures[TexturePaintingB], textures[TextureDetailMap],
            samplers[SamplerLinearNoMips], samplers[SamplerLinearNoMips],
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

            // Draw left box
            cmdBuffer->SetViewport(Viewport{ Offset2D{ 0, 0 }, halfResolution });
            cmdBuffer->SetResourceHeap(*resHeap, 0);
            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Draw right box
            cmdBuffer->SetViewport(Viewport{ Offset2D{ static_cast<std::int32_t>(halfResolution.width), 0 }, halfResolution });
            cmdBuffer->SetResourceHeap(*resHeap, 1);
            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Evaluate readback result
    SaveCapture(readbackTex, "ResourceArrays");

    constexpr int threshold = 3; // Tolerate a threshold of 3 color values
    const DiffResult diff = DiffImages("ResourceArrays", threshold);

    TestResult result = diff.Evaluate("resource arrays");

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*psoLayout);
    renderer->Release(*resHeap);

    return result;
}

