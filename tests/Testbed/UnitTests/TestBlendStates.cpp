/*
 * TestBlendStates.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/ProjectionMatrix4.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


/*
Renders a matrix of source/destination blend state combinations to ensure the configurations work the same on all backends.
Each combination is tested with two simple geometries (rectangles) that overlap to visualize its blending effect.
*/
DEF_TEST( BlendStates )
{
    if (shaders[VSTextured] == nullptr || shaders[PSTextured] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // Create all blend states
    struct BlendPair
    {
        BlendOp color, alpha;
    };

    const BlendPair blendPairs[] =
    {
        { BlendOp::SrcColor,    BlendOp::SrcAlpha    },
        { BlendOp::InvSrcColor, BlendOp::InvSrcAlpha },
        { BlendOp::SrcAlpha,    BlendOp::One         },
        { BlendOp::InvSrcAlpha, BlendOp::One         },
        { BlendOp::DstColor,    BlendOp::DstAlpha    },
        { BlendOp::InvDstColor, BlendOp::InvDstAlpha },
        { BlendOp::DstAlpha,    BlendOp::Zero        },
        { BlendOp::InvDstAlpha, BlendOp::Zero        },
    };

    constexpr int numBlendOps = sizeof(blendPairs)/sizeof(blendPairs[0]);
    PipelineState* pso[numBlendOps][numBlendOps] = {};

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout  = layouts[PipelineTextured];
        psoDesc.renderPass      = swapChain->GetRenderPass();
        psoDesc.vertexShader    = shaders[VSTextured];
        psoDesc.fragmentShader  = shaders[PSTextured];
    }

    BlendTargetDescriptor& target0Desc = psoDesc.blend.targets[0];
    target0Desc.blendEnabled = true;

    for_range(i, numBlendOps)
    {
        for_range(j, numBlendOps)
        {
            target0Desc.srcColor = blendPairs[i].color;
            target0Desc.dstColor = blendPairs[j].color;
            target0Desc.srcAlpha = blendPairs[i].alpha;
            target0Desc.dstAlpha = blendPairs[j].alpha;
            CREATE_GRAPHICS_PSO_EXT(pso[i][j], psoDesc, "psoBlendStates");
        }
    }

    // Initialize scene constants
    sceneConstants = SceneConstants{};

    sceneConstants.vpMatrix.LoadIdentity();

    auto TransformRect = [](Gs::Matrix4f& mat, float x, float y)
    {
        mat.LoadIdentity();
        Gs::Translate(mat, Gs::Vector3f{ x, y, 0.0f });
        Gs::Scale(mat, Gs::Vector3f{ 0.65f });
    };

    // Initialize viepwort to fit all blend state scenes into a single window
    Viewport viewport;
    {
        viewport.width  = static_cast<float>(opt.resolution.width ) / static_cast<float>(numBlendOps);
        viewport.height = static_cast<float>(opt.resolution.height) / static_cast<float>(numBlendOps);
    }

    // Render scene
    const IndexedTriangleMesh& mesh = models[ModelRect];

    constexpr float offset = 0.16f;
    constexpr float bgColor[4] = { 127.0f/255.0f, 127.0f/255.0f, 1.0f, 1.0f };

    Texture* readbackTex = nullptr;

    cmdBuffer->Begin();
    {
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::Color, ClearValue{ bgColor });

            for_range(i, numBlendOps)
            {
                for_range(j, numBlendOps)
                {
                    // Bind PSO with current blend states
                    cmdBuffer->SetPipelineState(*pso[i][j]);
                    cmdBuffer->SetResource(0, *sceneCbuffer);
                    cmdBuffer->SetResource(2, *samplers[SamplerLinearClamp]);

                    // Place viewport to fit all blend state scenes into a single window
                    viewport.x = static_cast<float>(i) * viewport.width;
                    viewport.y = static_cast<float>(j) * viewport.height;
                    cmdBuffer->SetViewport(viewport);

                    // Draw background rectangle
                    TransformRect(sceneConstants.wMatrix, -offset, -offset);
                    cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
                    cmdBuffer->SetResource(1, *textures[TextureGrid10x10]);
                    cmdBuffer->DrawIndexed(mesh.numIndices, 0);

                    // Draw foreground rectangle
                    TransformRect(sceneConstants.wMatrix, +offset, +offset);
                    cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
                    cmdBuffer->SetResource(1, *textures[TextureGradient]);
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
    const char* colorBufferName = "BlendStates";

    SaveCapture(readbackTex, colorBufferName);

    constexpr int threshold = 12; // Accept threshold of 12 to avoid failure on CIS server; seen consistent diffs of 4 or 12 across multiple backends
    const DiffResult diff = DiffImages(colorBufferName, threshold);

    // Clear resources
    for_range(i, numBlendOps)
    {
        for_range(j, numBlendOps)
            renderer->Release(*pso[i][j]);
    }

    return diff.Evaluate("blend states");
}

