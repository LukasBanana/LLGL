/*
 * TestTextureStrides.cpp
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
Test creating two textures from the same image source using row strides. Then render them onto two separate cubes.
First frame:
  Create textures with *half* row stride, this will result in interleaved rows,
  i.e. row 0 from left side, row 1 from right side, row 2 from left side again, etc.
  Rendering this texture will create the "moire" effect, which is acceptable here.
Second frame:
  Create textures with full row stride, this will result in a cutoff image as if only a region was copied from the image.
*/
DEF_TEST( TextureStrides )
{
    static TestResult result = TestResult::Passed;
    static PipelineState* pso;
    static PipelineLayout* psoLayout;

    constexpr unsigned numFrames = 2;

    if (frame == 0)
    {
        result = TestResult::Passed;

        if (shaders[VSTextured] == nullptr || shaders[PSTextured] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        // Create graphics PSO
        GraphicsPipelineDescriptor psoDesc;
        {
            psoDesc.debugName           = "TextureStrides.PSO";
            psoDesc.pipelineLayout      = layouts[PipelineTextured];
            psoDesc.renderPass          = swapChain->GetRenderPass();
            psoDesc.vertexShader        = shaders[VSTextured];
            psoDesc.fragmentShader      = shaders[PSTextured];
            psoDesc.depth.testEnabled   = true;
            psoDesc.depth.writeEnabled  = true;
            psoDesc.rasterizer.cullMode = CullMode::Back;
        }
        CREATE_GRAPHICS_PSO_EXT(pso, psoDesc, psoDesc.debugName);
    }

    // Create primary texture and two with different stride/offset
    const std::string imagePath = "../Media/Textures/";
    Image image = TestbedContext::LoadImageFromFile(imagePath + "Grid10x10.png", opt.verbose);

    ImageView imageViewA;
    {
        imageViewA.format       = image.GetFormat();
        imageViewA.dataType     = image.GetDataType();
        imageViewA.data         = image.GetData();
        imageViewA.dataSize     = image.GetDataSize();
        imageViewA.rowStride    = (frame == 0 ? image.GetRowStride()/2 : image.GetRowStride());
    }
    TextureDescriptor texADesc;
    {
        texADesc.debugName      = "texA-strides";
        texADesc.format         = Format::RGBA8UNorm;
        texADesc.extent.width   = image.GetExtent().width/2;
        texADesc.extent.height  = image.GetExtent().height;
        texADesc.extent.depth   = 1;
        texADesc.mipLevels      = 1;
    }
    CREATE_TEXTURE(texA, texADesc, texADesc.debugName, &imageViewA);

    const char* imageDataBytes = static_cast<const char*>(image.GetData());
    const std::ptrdiff_t imageBOffset = (frame == 0 ? image.GetDataSize()/2 : image.GetRowStride()/2);

    ImageView imageViewB;
    {
        imageViewB.format       = image.GetFormat();
        imageViewB.dataType     = image.GetDataType();
        imageViewB.data         = imageDataBytes + imageBOffset;
        imageViewB.dataSize     = image.GetDataSize() - imageBOffset;
        imageViewB.rowStride    = (frame == 0 ? image.GetRowStride()/2 : image.GetRowStride());
    }
    TextureDescriptor texBDesc;
    {
        texBDesc.debugName      = "texB-strides";
        texBDesc.format         = Format::RGBA8UNorm;
        texBDesc.extent.width   = image.GetExtent().width/2;
        texBDesc.extent.height  = image.GetExtent().height;
        texBDesc.extent.depth   = 1;
        texBDesc.mipLevels      = 1;
    }
    CREATE_TEXTURE(texB, texBDesc, texBDesc.debugName, &imageViewB);

    // Initialize scene constants
    sceneConstants = SceneConstants{};

    sceneConstants.vpMatrix = projection;

    auto TransformWorldMatrixAndUpdateCbuffer = [this](Gs::Matrix4f& wMatrix, float posX, float turn)
    {
        wMatrix.LoadIdentity();
        Gs::Translate(wMatrix, Gs::Vector3f{ posX, 0.0f, 3.5f });
        Gs::RotateFree(wMatrix, Gs::Vector3f{ 0, 1, 0 }, Gs::Deg2Rad(turn));
        Gs::Scale(wMatrix, Gs::Vector3f{ 0.5f });

        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
    };

    // Render scene
    Texture* readbackTex = nullptr;

    const IndexedTriangleMesh& mesh = models[ModelCube];

    cmdBuffer->Begin();
    {
        // Graphics can be set inside and outside a render pass, so test binding this PSO outside the render pass
        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);
        cmdBuffer->SetPipelineState(*pso);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            // Draw scene
            cmdBuffer->Clear(ClearFlags::ColorDepth, bgColorDarkBlue);
            cmdBuffer->SetViewport(opt.resolution);
            cmdBuffer->SetResource(0, *sceneCbuffer);
            cmdBuffer->SetResource(2, *samplers[SamplerNearestClamp]);

            // Draw left cube
            TransformWorldMatrixAndUpdateCbuffer(sceneConstants.wMatrix, -1.0f, +35.0f);
            cmdBuffer->SetResource(1, *texA);

            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Draw right cube
            TransformWorldMatrixAndUpdateCbuffer(sceneConstants.wMatrix, +1.0f, -35.0f);
            cmdBuffer->SetResource(1, *texB);

            cmdBuffer->DrawIndexed(mesh.numIndices, 0);

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "TextureStrides_Frame" + std::to_string(frame);

    SaveCapture(readbackTex, colorBufferName);

    const int threshold = 20;                           // High threshold because of nearest texture filter
    const unsigned tolerance = (frame == 0 ? 250 : 50); // Higher tolerance for frame 0, because of moire effect

    const DiffResult diff = DiffImages(colorBufferName, threshold, tolerance);

    // Evaluate readback result and tolerate 5 pixel that are beyond the threshold due to GPU differences with the reinterpretation of pixel formats
    TestResult intermediateResult = diff.Evaluate("texture strides", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    renderer->Release(*texA);
    renderer->Release(*texB);

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    renderer->Release(*pso);

    return result;
}

