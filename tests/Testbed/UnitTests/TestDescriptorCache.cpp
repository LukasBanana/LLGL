/*
 * TestVertexBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include "Testset.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>


/*
Sets resource descriptors thousands of times to stress test the descriptor caches of primarily the D3D12 and Vulkan backends.
Incorporate both dynamic bindings and heap bindings.
*/
DEF_TEST( DescriptorCache )
{
    if (shaders[VSTextured8] == nullptr || shaders[PSTextured8] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    // This test needs to render all framecaptures in a single command buffer in order to stress test the descriptor cache.
    constexpr int numFramesInSingleCmdEncoding = 8;

    // Create dummy textures with single pixel; This test only tests the descriptor cache, not the content of textures.
    auto CreateSolidColorTexture = [this](Texture*& outTex, const ColorRGBAub& col) -> TestResult
    {
        const UTF8String texName = UTF8String::Printf(
            "solid{%02X-%02X-%02X-%02X}",
            static_cast<unsigned>(col.r),
            static_cast<unsigned>(col.g),
            static_cast<unsigned>(col.b),
            static_cast<unsigned>(col.a)
        );
        TextureDescriptor texDesc;
        {
            texDesc.debugName   = texName.c_str();
            texDesc.extent      = { 1, 1, 1 };
            texDesc.format      = Format::RGBA8UNorm;
            texDesc.mipLevels   = 1;
        }
        ImageView img;
        {
            img.format      = ImageFormat::RGBA;
            img.dataType    = DataType::UInt8;
            img.data        = &col;
            img.dataSize    = sizeof(col);
        }
        return CreateTexture(texDesc, texName.c_str(), &outTex, &img);
    };

    constexpr int numTextures = 8;

    constexpr int numTextureBindings        = numTextures/2;
    constexpr int numTextureHeapBindings    = numTextures - numTextures/2;

    const ArrayView<LLGL::ColorRGBAub> solidColors = Testset::GetColorsRgbaUb8();

    Texture* solidTextures[numTextures] = {};
    for_range(i, numTextures)
    {
        TestResult texCreateResult = CreateSolidColorTexture(solidTextures[i], solidColors[i % solidColors.size()]);
        if (texCreateResult != TestResult::Passed)
            return texCreateResult;
    }

    // Create PSO
    PipelineLayout* psoLayout = renderer->CreatePipelineLayout(
        Parse(
            "cbuffer(Scene@1):vert:frag,"

            "sampler(texSampler0@10):frag,"
            "sampler(texSampler1@11):frag,"

            "texture(colorMap0@2):frag,"
            "texture(colorMap1@3):frag,"
            "texture(colorMap2@4):frag,"
            "texture(colorMap3@5):frag,"

            "heap{"
            "  texture(colorMap4@6):frag,"
            "  texture(colorMap5@7):frag,"
            "  texture(colorMap6@8):frag,"
            "  texture(colorMap7@9):frag,"
            "},"

            "sampler<colorMap0, texSampler0>(colorMap0@2),"
            "sampler<colorMap1, texSampler1>(colorMap1@3),"
            "sampler<colorMap2, texSampler0>(colorMap2@4),"
            "sampler<colorMap3, texSampler1>(colorMap3@5),"
            "sampler<colorMap4, texSampler0>(colorMap4@6),"
            "sampler<colorMap5, texSampler1>(colorMap5@7),"
            "sampler<colorMap6, texSampler0>(colorMap6@8),"
            "sampler<colorMap7, texSampler1>(colorMap7@9),"
        )
    );
    LLGL_VERIFY(psoLayout != nullptr);

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.debugName           = "MultiTextured[16]";
        psoDesc.pipelineLayout      = psoLayout;
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.inputVertexAttribs  = vertexFormats[VertFmtStd].attributes;
        psoDesc.vertexShader        = shaders[VSTextured8];
        psoDesc.fragmentShader      = shaders[PSTextured8];
        psoDesc.primitiveTopology   = PrimitiveTopology::TriangleList;
    }
    CREATE_GRAPHICS_PSO(pso, psoDesc, psoDesc.debugName);

    // Initialize viepwort to fit a grid of rectangles.
    // The grid dimensions should be common denominators for screen resolution (800 x 600),
    // to avoid one-off pixels between APIs due to different viewport transformations.
    constexpr int gridSizeX = 16;
    constexpr int gridSizeY = 12;

    constexpr int numTiles              = gridSizeX * gridSizeY;
    constexpr int numHeapDescriptors    = numTextureHeapBindings * numTiles * numFramesInSingleCmdEncoding;

    const float tileSizeX = static_cast<float>(opt.resolution.width ) / static_cast<float>(gridSizeX);
    const float tileSizeY = static_cast<float>(opt.resolution.height) / static_cast<float>(gridSizeY);

    Viewport viewport{ 0.0f, 0.0f, tileSizeX, tileSizeY };

    // Create large resource heap that is used for all tiles to be rendered
    const UTF8String resHeapName = UTF8String::Printf("ResourceHeap[%d]", numHeapDescriptors);
    ResourceHeapDescriptor resHeapDesc;
    {
        resHeapDesc.debugName           = resHeapName.c_str();
        resHeapDesc.pipelineLayout      = psoLayout;
        resHeapDesc.numResourceViews    = numHeapDescriptors;
    }
    ResourceHeap* resHeap = renderer->CreateResourceHeap(resHeapDesc);
    LLGL_VERIFY(resHeap != nullptr);

    // Fill resource heap with textures in an odd number, to add variety to the output
    const std::uint64_t resHeapStartTime = Timer::Tick();
    {
        constexpr int oddNumberOfTextures = numTextures - 1;
        for_range(descriptor, numHeapDescriptors)
            renderer->WriteResourceHeap(*resHeap, descriptor, { solidTextures[descriptor % oddNumberOfTextures] });
    }
    const std::uint64_t resHeapEndTime = Timer::Tick();

    if (opt.showTiming)
    {
        const double resHeapElapsedMS = (static_cast<double>(resHeapEndTime - resHeapStartTime) / static_cast<double>(Timer::Frequency())) * 1000.0;
        Log::Printf(
            Log::ColorFlags::StdAnnotation,
            "Time to write %d descriptors into '%s' -> %.2f ms\n",
            numHeapDescriptors, resHeapName.c_str(), resHeapElapsedMS
        );
    }

    // Update scene constants
    sceneConstants = SceneConstants{};
    sceneConstants.vpMatrix = projection;
    sceneConstants.wMatrix.LoadIdentity();
    Gs::Translate(sceneConstants.wMatrix, Gs::Vector3f{ 0.0f, 0.0f, 3.0f });

    // Render scene
    const IndexedTriangleMesh& mesh = models[ModelRect];

    Texture* readbackTexs[numFramesInSingleCmdEncoding] = {};

    const std::uint64_t cmdEncodingStartTime = Timer::Tick();

    constexpr int numResourcesPerTiles = numTextureBindings + numTextureHeapBindings + 3;
    constexpr int numRenderedTiles = gridSizeX * gridSizeY * numFramesInSingleCmdEncoding * numResourcesPerTiles;

    BEGIN();
    {
        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));

        cmdBuffer->SetVertexBuffer(*meshBuffer);
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);

        cmdBuffer->SetPipelineState(*pso);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            std::uint32_t descriptorSet = 0;
            for_range(i, numFramesInSingleCmdEncoding)
            {
                // Draw scene
                cmdBuffer->Clear(ClearFlags::ColorDepth);

                for_range(y, gridSizeY)
                {
                    for_range(x, gridSizeX)
                    {
                        // Bind resources
                        cmdBuffer->SetResource(0, *sceneCbuffer);

                        cmdBuffer->SetResource(1, *samplers[SamplerNearestNoMips]);
                        cmdBuffer->SetResource(2, *samplers[SamplerLinearNoMips]);

                        for_range(i, numTextureBindings)
                            cmdBuffer->SetResource(3 + i, *solidTextures[i]);

                        cmdBuffer->SetResourceHeap(*resHeap, descriptorSet++);

                        // Place viewport to fit grid of views into a single window
                        viewport.x = static_cast<float>(x) * tileSizeX;
                        viewport.y = static_cast<float>(y) * tileSizeY;

                        // Draw simple geometry into viewport tile
                        cmdBuffer->SetViewport(viewport);
                        cmdBuffer->DrawIndexed(mesh.numIndices, 0);
                    }
                }

                // Capture framebuffer in current iteration
                readbackTexs[i] = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
            }
        }
        cmdBuffer->EndRenderPass();
    }
    END();

    const std::uint64_t cmdEncodingEndTime = Timer::Tick();

    if (opt.showTiming)
    {
        const double cmdEncodingElapsedMS = (static_cast<double>(cmdEncodingEndTime - cmdEncodingStartTime) / static_cast<double>(Timer::Frequency())) * 1000.0;
        Log::Printf(
            Log::ColorFlags::StdAnnotation,
            "Time to encode command buffer with %d descriptors total -> %.2f ms\n",
            numRenderedTiles, cmdEncodingElapsedMS
        );
    }

    // Match all readback textures against references
    TestResult result = TestResult::Passed;

    for_range(i, numFramesInSingleCmdEncoding)
    {
        const std::string colorBufferName = "DescriptorCache_View" + std::to_string(i);
        SaveCapture(readbackTexs[i], colorBufferName);
        const DiffResult diff = DiffImages(colorBufferName);

        // Evaluate readback result
        TestResult intermediateResult = diff.Evaluate("descriptor cache", static_cast<unsigned>(i));
        if (intermediateResult != TestResult::Passed)
        {
            result = intermediateResult;
            if (!opt.greedy)
                break;
        }
    }

    // Clear resources
    renderer->Release(*pso);
    renderer->Release(*psoLayout);

    for_range(i, numTextures)
        renderer->Release(*solidTextures[i]);

    return result;
}

