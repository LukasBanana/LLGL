/*
 * TestTextureViews.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Utils/Parse.h>
#include <Gauss/ProjectionMatrix4.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


/*
Test rendering the same texture with views (LLGL::TextureViewDescriptor) of different formats, base MIP levels, and texture swizzling.
This should result in different visualizations of the same texture that are interpreted differently while using the same PSO.
A texture view of BGRA with swizzling BGRA should cancel each other out and result in the exact same image if the texture has the base format RGBA.
*/
DEF_TEST( TextureViews )
{
    // Skip if texture swizzling is not supported
    if (!caps.features.hasTextureViewSwizzle)
        return TestResult::Skipped;

    if (shaders[VSTextured] == nullptr || shaders[PSTextured] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Create graphics PSO
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(
        Parse(
            HasCombinedSamplers()
                ?   "cbuffer(Scene@1):vert:frag,"
                    "heap{texture(colorMap@2):frag},"
                    "sampler(2):frag,"
                :
                    "cbuffer(Scene@1):vert:frag,"
                    "heap{texture(colorMap@2):frag},"
                    "sampler(linearSampler@3):frag,"
        )
    );

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout                  = psoLayout;
        psoDesc.renderPass                      = swapChain->GetRenderPass();
        psoDesc.vertexShader                    = shaders[VSTextured];
        psoDesc.fragmentShader                  = shaders[PSTextured];
        psoDesc.blend.targets[0].blendEnabled   = true;
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, "psoTexViews");

    // D3D does not support reinterpretation of texture view formats, i.e. RGBA8 cannot be reinterpreted to RG16, but Vulkan, GL, and Metal support it.
    const bool isTextureFormatReinterpretationSupported = renderer->GetRenderingCaps().features.hasTextureViewFormatSwizzle;

    // Create all resource heap with all texture swizzle configurations
    struct ViewPair
    {
        Format      format;
        const char* swizzle;
        int         mip;
    };

    const ViewPair viewConfigs[] =
    {
        ViewPair{ Format::RGBA8UNorm, "rgba", 1 }, ViewPair{ Format::RGBA8UNorm, "bgra", 1 }, ViewPair{ Format::RGBA8UNorm, "rgrg", 1 }, ViewPair{ Format::BGRA8UNorm, "rrr1", 1 },
        ViewPair{ Format::BGRA8UNorm, "rgba", 1 }, ViewPair{ Format::BGRA8UNorm, "bgra", 4 }, ViewPair{ Format::RG16UNorm,  "rgrg", 4 }, ViewPair{ Format::RG16UNorm,  "ggrr", 4 },
        ViewPair{ Format::RGBA8UNorm, "0011", 1 }, ViewPair{ Format::RG16UNorm,  "1rg1", 1 }, ViewPair{ Format::RGBA8UNorm, "1rr1", 1 }, ViewPair{ Format::RG16SNorm,  "rg11", 0 },
        ViewPair{ Format::RGBA8UNorm, "rrrr", 1 }, ViewPair{ Format::RGBA8UNorm, "raaa", 1 }, ViewPair{ Format::RGBA8UNorm, "rara", 1 }, ViewPair{ Format::RG16Float,  "rg11", 0 },
    };

    constexpr int numViewConfigsSqrt = 4;
    constexpr int numViewConfigs = numViewConfigsSqrt*numViewConfigsSqrt;

    static_assert(numViewConfigs == sizeof(viewConfigs)/sizeof(viewConfigs[0]), "number of texture swizzle configurations must be the square of 3");

    ResourceViewDescriptor resViewDescs[numViewConfigs];
    {
        for_range(i, numViewConfigs)
        {
            Texture* tex = textures[TexturePaintingB];
            ResourceViewDescriptor& viewDesc = resViewDescs[i];
            viewDesc.resource                               = tex;
            viewDesc.textureView.type                       = tex->GetType();
            viewDesc.textureView.format                     = (isTextureFormatReinterpretationSupported ? viewConfigs[i].format : tex->GetFormat());
            viewDesc.textureView.subresource.baseMipLevel   = static_cast<std::uint32_t>(viewConfigs[i].mip);
            viewDesc.textureView.swizzle                    = Parse(viewConfigs[i].swizzle);
        }
    }
    ResourceHeap* resHeap = renderer->CreateResourceHeap(psoLayout, resViewDescs);

    // Initialize scene constants
    sceneConstants = SceneConstants{};

    sceneConstants.vpMatrix.LoadIdentity();
    sceneConstants.wMatrix.LoadIdentity();

    // Initialize viepwort to fit all blend state scenes into a single window
    Viewport viewport;
    {
        viewport.width  = static_cast<float>(opt.resolution.width ) / static_cast<float>(numViewConfigsSqrt);
        viewport.height = static_cast<float>(opt.resolution.height) / static_cast<float>(numViewConfigsSqrt);
    }

    // Render scene
    const IndexedTriangleMesh& mesh = models[ModelRect];

    Texture* readbackTex = nullptr;

    cmdBuffer->Begin();
    {
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);

        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::Color, bgColorLightBlue);

            // Bind PSO with current blend states
            cmdBuffer->SetPipelineState(*pso);
            cmdBuffer->SetResource(0, *sceneCbuffer);
            cmdBuffer->SetResource(1, *samplers[SamplerLinearClamp]);

            for_range(y, numViewConfigsSqrt)
            {
                for_range(x, numViewConfigsSqrt)
                {
                    // Place viewport to fit all texture swizzle configuration into a single window
                    viewport.x = static_cast<float>(x) * viewport.width;
                    viewport.y = static_cast<float>(y) * viewport.height;
                    cmdBuffer->SetViewport(viewport);

                    // Select texture view from resource heap
                    const std::uint32_t texViewIndex = y*numViewConfigsSqrt + x;
                    cmdBuffer->SetResourceHeap(*resHeap, texViewIndex);

                    // Draw rectangle
                    cmdBuffer->DrawIndexed(mesh.numIndices, 0);
                }
            }

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Match entire color buffer and create delta heat map
    const char* colorBufferName = (isTextureFormatReinterpretationSupported ? "TextureViews" : "TextureViews_Limited");

    SaveCapture(readbackTex, colorBufferName);

    // Evaluate readback result and tolerate 5 pixel that are beyond the threshold due to GPU differences with the reinterpretation of pixel formats
    constexpr int threshold = 5;
    constexpr unsigned tolerance = 5;
    const DiffResult diff = DiffImages(colorBufferName, threshold, tolerance);

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    return diff.Evaluate("texture views");
}

