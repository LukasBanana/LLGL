/*
 * TestUniforms.cpp
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
Test changing uniforms dynamically before each draw call via CommandBuffer::SetUniforms().
In D3D, the uniforms are also distributed over two cbuffers, one explicitly "Model" and one implicitly "$Globals".
LLGL must be able to assign the uniforms accordingly no matter how they are distributed in how many cbuffers in the shader.
*/
DEF_TEST( Uniforms )
{
    static TestResult result = TestResult::Passed;
    static PipelineState* pso;
    static PipelineLayout* psoLayout;

    if (frame == 0)
    {
        result = TestResult::Passed;

        if (shaders[VSDynamic] == nullptr || shaders[PSDynamic] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        // Create graphics PSO
        SamplerDescriptor staticSamplerDesc = Parse("filter.min=nearest,filter.mag=nearest,address=clamp");
        PipelineLayoutDescriptor psoLayoutDesc;
        {
            psoLayoutDesc.bindings  =
            {
                BindingDescriptor{ "Scene",    ResourceType::Buffer,  BindFlags::ConstantBuffer, StageFlags::VertexStage,   1u },
                BindingDescriptor{ "colorMap", ResourceType::Texture, BindFlags::Sampled,        StageFlags::FragmentStage, 3u },
            };
            psoLayoutDesc.staticSamplers =
            {
                StaticSamplerDescriptor{ "linearSampler", StageFlags::FragmentStage, (HasCombinedSamplers() ? 3u : 4u), staticSamplerDesc }
            };
            psoLayoutDesc.uniforms =
            {
                UniformDescriptor{ "wMatrix",    UniformType::Float4x4 },
                UniformDescriptor{ "solidColor", UniformType::Float4   },
                UniformDescriptor{ "lightVec",   UniformType::Float3   },
            };
        }
        psoLayout = renderer->CreatePipelineLayout(psoLayoutDesc);

        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.debugName                       = "Test.Uniforms.PSO";
            psoDesc.pipelineLayout                  = psoLayout;
            psoDesc.renderPass                      = swapChain->GetRenderPass();
            psoDesc.vertexShader                    = shaders[VSDynamic];
            psoDesc.fragmentShader                  = shaders[PSDynamic];
            psoDesc.depth.testEnabled               = true;
            psoDesc.depth.writeEnabled              = true;
            psoDesc.rasterizer.cullMode             = CullMode::Back;
            psoDesc.blend.targets[0].blendEnabled   = true;
        }
        CREATE_GRAPHICS_PSO_EXT(pso, psoDesc, nullptr);
    }

    // Skip every other frame on fast test
    if (opt.fastTest && (frame % 2 == 0))
        return TestResult::ContinueSkipFrame;

    // Update scene constants
    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -3 });
    vMatrix.MakeInverse();

    const Gs::Matrix4f vpMatrix = projection * vMatrix;

    auto TransformWorldMatrix = [](Gs::Matrix4f& wMatrix, float pos, float scale, float turn)
    {
        wMatrix.LoadIdentity();
        Gs::Translate(wMatrix, Gs::Vector3f{ 0, pos, 2.0f });
        Gs::RotateFree(wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(turn));
        Gs::Scale(wMatrix, Gs::Vector3f{ 1, scale, 1 });
    };

    struct alignas(16) ModelUniforms
    {
        Gs::Matrix4f    wMatrix;
        ColorRGBAf      solidColor;
        Gs::Vector3f    lightVec = { 0, 0, -1 };
    };

    static_assert(sizeof(ModelUniforms) == (16+4+4)*sizeof(float), "ModelUniforms must be 6 float4-vectors large (92 bytes)");
    static_assert(offsetof(ModelUniforms, solidColor) == 64, "ModelUniforms::solidColor must have offset 64");
    static_assert(offsetof(ModelUniforms, lightVec) == 80, "ModelUniforms::lightVec must have offset 80");

    ModelUniforms modelData = ModelUniforms{};

    constexpr unsigned numFrames = 10;
    const float rotation = static_cast<float>(frame) * 90.0f / static_cast<float>(numFrames - 1);

    // Render scene
    Texture* readbackTex = nullptr;

    const IndexedTriangleMesh& mesh = models[ModelCube];

    cmdBuffer->Begin();
    {
        // Graphics can be set inside and outside a render pass, so test binding this PSO outside the render pass
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
        cmdBuffer->SetPipelineState(*pso);

        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &vpMatrix, sizeof(vpMatrix));

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::ColorDepth, bgColorDarkBlue);
            cmdBuffer->SetViewport(opt.resolution);
            cmdBuffer->SetResource(0, *sceneCbuffer);

            // Draw top part
            modelData.solidColor = { 1.0f, 1.0f, 0.0f, 1.0f }; // pure yellow
            TransformWorldMatrix(modelData.wMatrix, 0.5f, 0.5f, rotation);

            cmdBuffer->SetResource(1, *textures[TextureGrid10x10]);
            cmdBuffer->SetUniforms(0, &modelData, sizeof(modelData));                       // Set all uniforms at once

            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Draw bottom part
            modelData.solidColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // white
            TransformWorldMatrix(modelData.wMatrix, -0.75f, 0.25f, rotation);

            cmdBuffer->SetResource(1, *textures[TextureGrid10x10]);
            cmdBuffer->SetUniforms(1, &(modelData.solidColor), sizeof(LLGL::ColorRGBAf));   // Set solid color
            cmdBuffer->SetUniforms(0, &(modelData.wMatrix), sizeof(Gs::Matrix4f));          // Set world matrix

            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Draw middle part
            modelData.solidColor = { 1.0f, 1.0f, 1.0f, 0.5f }; // half-translucent
            TransformWorldMatrix(modelData.wMatrix, -0.25f, 0.25f, rotation);

            cmdBuffer->SetResource(1, *textures[TextureGradient]);
            cmdBuffer->SetUniforms(0, &(modelData.wMatrix), sizeof(Gs::Matrix4f));          // Set world matrix
            cmdBuffer->SetUniforms(1, &(modelData.solidColor), sizeof(LLGL::ColorRGBAf));   // Set solid color
            cmdBuffer->SetUniforms(2, &(modelData.lightVec), sizeof(Gs::Vector3f));         // Set light vector

            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "Uniforms_Frame" + std::to_string(frame);

    SaveCapture(readbackTex, colorBufferName);

    constexpr int threshold = 20;       // High threshold because of nearest texture filter
    constexpr unsigned tolerance = 100; // High tolerance because of nearest texture filter
    const DiffResult diff = DiffImages(colorBufferName, threshold, tolerance);

    // Evaluate readback result and tolerate 5 pixel that are beyond the threshold due to GPU differences with the reinterpretation of pixel formats
    TestResult intermediateResult = diff.Evaluate("uniforms", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    return result;
}

