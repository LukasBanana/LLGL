/*
 * TestStreamOutput.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Translate.h>
#include <Gauss/Rotate.h>
#include <Gauss/Scale.h>
#include <Gauss/Algebra.h>


struct alignas(16) SOScene
{
    Gs::Matrix4f    vsMatrix;
    Gs::Matrix4f    gsMatrices[3];
    Gs::Vector4f    lightVec            = { 0, 0, -1, 0 };
    float           normalizeFactorVS   = 0.0f;
    float           normalizeFactorDS   = 0.0f;
    float           tessLevelOuter      = 0.0f;
    float           tessLevelInner      = 0.0f;
};

static_assert(sizeof(SOScene) == (16*4+4+4)*sizeof(float), "SOScene must be 18 float4-vectors large (288 bytes)");

/*
Test stream output by rendering a single object in a StreamOutput section that transforms the vertices,
then draw it with DrawStreamOutput() command and generate yet another stream output.
Repeat several times and then render the final result with a geometry shader to multiply the geometry as 3 instances.
*/
DEF_TEST( StreamOutput )
{
    if (!caps.features.hasStreamOutputs)
        return TestResult::Skipped;

    static TestResult result = TestResult::Passed;

    static PipelineLayout* psoLayoutVert;
    static PipelineLayout* psoLayoutTess;
    static PipelineLayout* psoLayoutGeom;
    static PipelineLayout* psoLayoutFrag;

    static PipelineState* psoVert;
    static PipelineState* psoTess;
    static PipelineState* psoGeom;
    static PipelineState* psoFrag;

    static Buffer* soSceneCbuffer;
    static Buffer* soVertexBuffers[2]; // VertexBuffer | StreamOutputBuffer
    static QueryHeap* queryHeaps[2];

    static std::uint32_t numInitialVertices;
    static std::vector<ColoredVertex> cubeVertices;

    constexpr std::uint32_t maxSOVertices = 20000;
    constexpr int numVertPreTransforms = 3;
    constexpr int numTessPreTransforms = 1;
    constexpr int numGeomPreTransforms = 1;

    constexpr unsigned numFrames = 10;

    constexpr std::uint32_t expectedSOVerticesPerFrame[numFrames] =
    {
          108,18252,18252,18252,18252,
        18252,18252,18252,18252,18252,
    };

    static_assert(expectedSOVerticesPerFrame[0] % 3 == 0, "expected number of SO vertices must be a multiple of 3");
    static_assert(expectedSOVerticesPerFrame[1] % 3 == 0, "expected number of SO vertices must be a multiple of 3");

    if (frame == 0)
    {
        result = TestResult::Passed;

        if (shaders[VSStreamOutput   ] == nullptr ||
            shaders[VSStreamOutputXfb] == nullptr ||
            shaders[HSStreamOutput   ] == nullptr ||
            shaders[DSStreamOutput   ] == nullptr ||
            shaders[DSStreamOutputXfb] == nullptr ||
            shaders[GSStreamOutputXfb] == nullptr ||
            shaders[PSStreamOutput   ] == nullptr)
        {
            Log::Errorf("Missing shaders for backend\n");
            return TestResult::FailedErrors;
        }

        // Create scene cbuffer
        BufferDescriptor cbufDesc;
        {
            cbufDesc.debugName  = "SOScene.cbuffer";
            cbufDesc.size       = sizeof(SOScene);
            cbufDesc.bindFlags  = BindFlags::ConstantBuffer;
        }
        result = CreateBuffer(cbufDesc, cbufDesc.debugName, &soSceneCbuffer);
        if (result != TestResult::Passed)
            return result;

        // Create vertex input buffer
        for_range(i, 2)
        {
            const std::string vertBufName = "SOVertexBuffer[" + std::to_string(i) + "]";
            BufferDescriptor vertBufDesc;
            {
                vertBufDesc.debugName       = vertBufName.c_str();
                vertBufDesc.size            = sizeof(ColoredVertex) * maxSOVertices;
                vertBufDesc.bindFlags       = BindFlags::VertexBuffer | BindFlags::StreamOutputBuffer;

                // IA stage uses "position" attribute instead of SystemValue::Position, so use VertFmtColored instead of VertFmtColoredSO
                vertBufDesc.vertexAttribs   = vertexFormats[VertFmtColored].attributes;
            }
            result = CreateBuffer(vertBufDesc, vertBufDesc.debugName, &soVertexBuffers[i]);
            if (result != TestResult::Passed)
                return result;
        }

        // Create initial vertex data
        IndexedTriangleMeshBuffer indexedCubeMeshBuffer;
        IndexedTriangleMesh indexedCubeMesh;
        CreateModelCube(indexedCubeMeshBuffer, indexedCubeMesh);

        cubeVertices.clear();
        ConvertToColoredVertexList(indexedCubeMeshBuffer, cubeVertices);
        numInitialVertices = static_cast<std::uint32_t>(cubeVertices.size());

        // Create StreamOutPrimitivesWritten and  queries
        QueryHeapDescriptor queryHeapDesc0;
        {
            queryHeapDesc0.debugName    = "SO.PrimitivesOut.Query";
            queryHeapDesc0.type         = QueryType::StreamOutPrimitivesWritten;
            queryHeapDesc0.numQueries   = 1;
        }
        queryHeaps[0] = renderer->CreateQueryHeap(queryHeapDesc0);

        QueryHeapDescriptor queryHeapDesc1;
        {
            queryHeapDesc1.debugName    = "SO.Overflow.Query";
            queryHeapDesc1.type         = QueryType::StreamOutOverflow;
            queryHeapDesc1.numQueries   = 1;
        }
        queryHeaps[1] = renderer->CreateQueryHeap(queryHeapDesc1);

        // Create graphics PSOs
        psoLayoutVert = renderer->CreatePipelineLayout(Parse("cbuffer(SOScene@1):vert"));
        psoLayoutTess = renderer->CreatePipelineLayout(Parse("cbuffer(SOScene@1):vert:tesc:tese"));
        psoLayoutGeom = renderer->CreatePipelineLayout(Parse("cbuffer(SOScene@1):vert:tesc:tese:geom"));
        psoLayoutFrag = renderer->CreatePipelineLayout(Parse("cbuffer(SOScene@1):vert:frag"));

        GraphicsPipelineDescriptor psoVertDesc;
        {
            psoVertDesc.debugName                       = "SO.VERT.PSO";
            psoVertDesc.pipelineLayout                  = psoLayoutVert;
            psoVertDesc.renderPass                      = swapChain->GetRenderPass();
            psoVertDesc.vertexShader                    = shaders[VSStreamOutputXfb];
            psoVertDesc.primitiveTopology               = PrimitiveTopology::TriangleList;
            psoVertDesc.rasterizer.discardEnabled       = true;
        }
        CREATE_GRAPHICS_PSO_EXT(psoVert, psoVertDesc, nullptr);

        GraphicsPipelineDescriptor psoTessDesc;
        {
            psoTessDesc.debugName                       = "SO.TESS.PSO";
            psoTessDesc.pipelineLayout                  = psoLayoutTess;
            psoTessDesc.renderPass                      = swapChain->GetRenderPass();
            psoTessDesc.vertexShader                    = shaders[VSStreamOutput];
            psoTessDesc.tessControlShader               = shaders[HSStreamOutput];
            psoTessDesc.tessEvaluationShader            = shaders[DSStreamOutputXfb];
            psoTessDesc.primitiveTopology               = PrimitiveTopology::Patches3;
            psoTessDesc.rasterizer.discardEnabled       = true;
        }
        CREATE_GRAPHICS_PSO_EXT(psoTess, psoTessDesc, nullptr);

        GraphicsPipelineDescriptor psoGeomDesc;
        {
            psoGeomDesc.debugName                       = "SO.GEOM.PSO";
            psoGeomDesc.pipelineLayout                  = psoLayoutGeom;
            psoGeomDesc.renderPass                      = swapChain->GetRenderPass();
            psoGeomDesc.vertexShader                    = shaders[VSStreamOutput];
            psoGeomDesc.tessControlShader               = shaders[HSStreamOutput];
            psoGeomDesc.tessEvaluationShader            = shaders[DSStreamOutput];
            psoGeomDesc.geometryShader                  = shaders[GSStreamOutputXfb];
            psoGeomDesc.primitiveTopology               = PrimitiveTopology::Patches3;
            psoGeomDesc.rasterizer.discardEnabled       = true;
        }
        CREATE_GRAPHICS_PSO_EXT(psoGeom, psoGeomDesc, nullptr);

        GraphicsPipelineDescriptor psoFragDesc;
        {
            psoFragDesc.debugName                       = "SO.FRAG.PSO";
            psoFragDesc.pipelineLayout                  = psoLayoutFrag;
            psoFragDesc.renderPass                      = swapChain->GetRenderPass();
            psoFragDesc.vertexShader                    = shaders[VSStreamOutput];
            psoFragDesc.fragmentShader                  = shaders[PSStreamOutput];
            psoFragDesc.primitiveTopology               = PrimitiveTopology::TriangleList;
            psoFragDesc.depth.testEnabled               = true;
            psoFragDesc.depth.writeEnabled              = true;
            psoFragDesc.rasterizer.cullMode             = CullMode::Back;
        }
        CREATE_GRAPHICS_PSO_EXT(psoFrag, psoFragDesc, nullptr);
    }

    // Skip every other frame on fast test
    if (opt.fastTest && (frame % 2 == 0))
        return TestResult::ContinueSkipFrame;

    // Initialize scene constants
    const float rotation = static_cast<float>(frame) * 90.0f / static_cast<float>(numFrames - 1);

    constexpr float bgColor[4] = { 0.2f, 0.2f, 0.4f, 1.0f };

    // Initialize scene settings
    const float frameTransition = static_cast<float>(frame) / static_cast<float>(numFrames - 1);

    SOScene soSceneConstants;
    {
        soSceneConstants.vsMatrix.LoadIdentity();

        for_range(i, 3)
        {
            soSceneConstants.gsMatrices[i].LoadIdentity();
            const float posX = static_cast<float>(i - 1) * 2.0f;
            Gs::Translate(soSceneConstants.gsMatrices[i], Gs::Vector3f{ posX, 0.0f, 0.0f });
            Gs::Scale(soSceneConstants.gsMatrices[i], Gs::Vector3f{ 0.8f, 1.5f, 0.8f });
        }

        soSceneConstants.normalizeFactorVS  = Gs::Lerp(0.0f, 0.1f, frameTransition);
        soSceneConstants.normalizeFactorDS  = Gs::Lerp(0.2f, 1.0f, frameTransition);
        soSceneConstants.tessLevelInner     = Gs::Lerp(1.0f, 3.0f, frameTransition);
        soSceneConstants.tessLevelOuter     = Gs::Lerp(1.0f, 3.0f, frameTransition);
    }

    Texture* readbackTex = nullptr;

    // Reset first vertex buffer
    renderer->WriteBuffer(*soVertexBuffers[0], 0, cubeVertices.data(), sizeof(ColoredVertex) * cubeVertices.size());

    // Draw frame
    cmdBuffer->Begin();
    {
        // Initialize constant buffer with identity matrices to perform pre-transformations only in model space
        cmdBuffer->UpdateBuffer(*soSceneCbuffer, 0, &soSceneConstants, sizeof(soSceneConstants));

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->Clear(ClearFlags::ColorDepth, ClearValue{ bgColor });
            cmdBuffer->SetViewport(opt.resolution);

            int currentSOSwapBuffer = 0;

            // Pre-transform mesh with vertex shader only
            cmdBuffer->PushDebugGroup("SO.VertexOnly");
            {
                cmdBuffer->SetPipelineState(*psoVert);
                cmdBuffer->SetResource(0, *soSceneCbuffer);

                for_range(i, numVertPreTransforms)
                {
                    cmdBuffer->SetVertexBuffer(*soVertexBuffers[currentSOSwapBuffer]);
                    cmdBuffer->BeginStreamOutput(1, &(soVertexBuffers[(currentSOSwapBuffer + 1) % 2]));
                    {
                        if (i == 0)
                            cmdBuffer->Draw(numInitialVertices, 0); //TODO: use non-indexed mesh
                        else
                            cmdBuffer->DrawStreamOutput();
                    }
                    cmdBuffer->EndStreamOutput();
                    currentSOSwapBuffer = (currentSOSwapBuffer + 1) % 2;
                }
            }
            cmdBuffer->PopDebugGroup();

            // Continue transformation of vertices with tessellation shader
            cmdBuffer->PushDebugGroup("SO.Tessellation");
            {
                cmdBuffer->SetPipelineState(*psoTess);
                cmdBuffer->SetResource(0, *soSceneCbuffer);

                for_range(i, numTessPreTransforms)
                {
                    cmdBuffer->SetVertexBuffer(*soVertexBuffers[currentSOSwapBuffer]);
                    cmdBuffer->BeginStreamOutput(1, &(soVertexBuffers[(currentSOSwapBuffer + 1) % 2]));
                    {
                        cmdBuffer->DrawStreamOutput();
                    }
                    cmdBuffer->EndStreamOutput();
                    currentSOSwapBuffer = (currentSOSwapBuffer + 1) % 2;
                }
            }
            cmdBuffer->PopDebugGroup();

            // Continue transformation of vertices with tessellation and geometry shaders
            cmdBuffer->PushDebugGroup("SO.Tess+Geom");
            {
                cmdBuffer->SetPipelineState(*psoGeom);
                cmdBuffer->SetResource(0, *soSceneCbuffer);

                for_range(i, numGeomPreTransforms)
                {
                    cmdBuffer->SetVertexBuffer(*soVertexBuffers[currentSOSwapBuffer]);
                    cmdBuffer->BeginStreamOutput(1, &(soVertexBuffers[(currentSOSwapBuffer + 1) % 2]));
                    {
                        cmdBuffer->DrawStreamOutput();
                    }
                    cmdBuffer->EndStreamOutput();
                    currentSOSwapBuffer = (currentSOSwapBuffer + 1) % 2;
                }
            }
            cmdBuffer->PopDebugGroup();

            // Before drawing the final transformation onto the screen, update matrices to use the camera projection
            cmdBuffer->PushDebugGroup("SO.Final");
            {
                soSceneConstants.vsMatrix.LoadIdentity();
                Gs::Translate(soSceneConstants.vsMatrix, Gs::Vector3f{ 0, 0, 6.0f });
                soSceneConstants.vsMatrix = projection * soSceneConstants.vsMatrix;

                cmdBuffer->UpdateBuffer(*soSceneCbuffer, 0, &soSceneConstants, sizeof(soSceneConstants));

                // Draw final scene with fragment shader and query how many primitives have been written out
                cmdBuffer->SetPipelineState(*psoFrag);
                cmdBuffer->SetResource(0, *soSceneCbuffer);

                // Only bind a stream-output buffer so we can query the number of written primitives.
                // The stream-output buffer is not needed for anything else.
                cmdBuffer->SetVertexBuffer(*soVertexBuffers[currentSOSwapBuffer]);
                cmdBuffer->BeginStreamOutput(1, &(soVertexBuffers[(currentSOSwapBuffer + 1) % 2]));
                {
                    cmdBuffer->BeginQuery(*queryHeaps[0], 0);
                    cmdBuffer->BeginQuery(*queryHeaps[1], 0);
                    {
                        cmdBuffer->DrawStreamOutput();
                    }
                    cmdBuffer->EndQuery(*queryHeaps[1], 0);
                    cmdBuffer->EndQuery(*queryHeaps[0], 0);
                }
                cmdBuffer->EndStreamOutput();
            }
            cmdBuffer->PopDebugGroup();

            // Capture framebuffer
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    cmdBuffer->End();

    cmdQueue->Submit(*cmdBuffer);

    // Query number of written stream-output vertices and match them with the expected numbers per frame
    const std::uint32_t expectedNumPrimitives = expectedSOVerticesPerFrame[frame] / 3;
    std::uint32_t actualNumPrimitives = 0xDEADBEEF;
    if (QueryResultsWithTimeout(*queryHeaps[0], 0, 1, &actualNumPrimitives, sizeof(actualNumPrimitives)))
    {
        if (actualNumPrimitives != expectedNumPrimitives)
        {
            Log::Errorf(
                "Mismatch between number of written stream-output primitives (0x%08X) in frame [%u] and expected value (0x%08X)\n",
                actualNumPrimitives, frame, expectedNumPrimitives
            );
            result = TestResult::FailedMismatch;
            if (!opt.greedy)
                return result;
        }
    }
    else
        result = TestResult::FailedErrors;

    const std::uint32_t expectedPrimitiveOverflow = 0;
    std::uint32_t actualPrimitiveOverflow = 0xDEADBEEF;
    if (QueryResultsWithTimeout(*queryHeaps[1], 0, 1, &actualPrimitiveOverflow, sizeof(actualPrimitiveOverflow)))
    {
        if (actualPrimitiveOverflow != expectedPrimitiveOverflow)
        {
            Log::Errorf(
                "Mismatch between stream-output primitive overflow flag (0x%08X) in frame [%u] and expected value (0x%08X)\n",
                actualPrimitiveOverflow, frame, expectedPrimitiveOverflow
            );
            result = TestResult::FailedMismatch;
            if (!opt.greedy)
                return result;
        }
    }
    else
        result = TestResult::FailedErrors;

    // Match entire color buffer and create delta heat map
    const std::string colorBufferName = "StreamOutput_Frame" + std::to_string(frame);

    SaveCapture(readbackTex, colorBufferName);

    constexpr int threshold = 5;
    constexpr unsigned tolerance = 10;
    const DiffResult diff = DiffImages(colorBufferName, threshold, tolerance);

    TestResult intermediateResult = diff.Evaluate("stream-output", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    renderer->Release(*psoVert);
    renderer->Release(*psoTess);
    renderer->Release(*psoGeom);
    renderer->Release(*psoFrag);
    renderer->Release(*psoLayoutVert);
    renderer->Release(*psoLayoutTess);
    renderer->Release(*psoLayoutGeom);
    renderer->Release(*psoLayoutFrag);

    return result;
}

