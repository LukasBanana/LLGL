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
Renders instanced geometry with different methods of binding vertex input:
1: Two vertex buffers
   -> vertices slot 0, instances slot 1.
2: One vertex buffer at two binding slots
   -> vertices slot 0, instances slot 1 (same buffer).
3: Three vertex buffers
   -> vertices slot 0, instance matrices slot 1, instance colors slot 2.
4: Three vertex buffers with the third binding from a larger base offset
   -> vertices slot 0, instance matrices slot 1, instance colors slot 2 (offset second half of 'buf3')
*/
DEF_TEST( VertexArray )
{
    //TODO: shaders not implemented for Metal yet
    if (renderer->GetRendererID() == RendererID::Metal)
        return TestResult::Skipped;

    if (shaders[VSSolidInstanced] == nullptr || shaders[PSSolid] == nullptr)
    {
        Log::Errorf(Log::ColorFlags::StdError, "Missing shaders for VertexBufferArray test\n");
        return TestResult::FailedErrors;
    }

    constexpr std::uint32_t minRequiredVertexBufferInputs = 3;
    if (caps.limits.maxVertexBufferInputs < minRequiredVertexBufferInputs)
    {
        Log::Errorf(
            Log::ColorFlags::StdError,
            "Renderer does not support enough vertex buffer bindings (less than %u)\n",
            minRequiredVertexBufferInputs
        );
        return TestResult::FailedErrors;
    }

    static TestResult result = TestResult::Passed;

    if (frame == 0)
        result = TestResult::Passed;

    constexpr unsigned numFrames = 2;

    // Define vertex data
    enum InputLayouts
    {
        InputLayout_2Buffers = 0,
        InputLayout_1Buffer2Bindings,
        InputLayout_3Buffers,
        InputLayout_3BuffersBaseOffset,

        NumInputLayouts,
    };

    IndexedTriangleMeshBuffer meshData;
    IndexedTriangleMesh model;
    CreateModelCube(meshData, model);

    const std::uint32_t numInstancesPerDraw = 2;
    const std::uint32_t numInstances        = numInstancesPerDraw * NumInputLayouts;
    const std::uint32_t numVertices         = static_cast<std::uint32_t>(meshData.vertices.size());

    struct Instance
    {
        Instance() = default;
        Instance(float x, float y, float angle, ColorRGBAub color) :
            color { color }
        {
            matrix.LoadIdentity();
            Gs::Translate(matrix, Gs::Vector3f{ x, y, 0.0f });
            Gs::RotateFree(matrix, Gs::Vector3f{ 1, 1, 1 }.Normalized(), Gs::Deg2Rad(angle));
        }

        Gs::Matrix4f    matrix;
        ColorRGBAub     color;
    };

    // Build instance data (numInstances * NumInputLayouts)
    const ArrayView<ColorRGBAub> colors = Testset::GetColorsRgbaUb8();
    const Instance instanceData[] =
    {
        Instance{ -6.0f, +2.0f,  0.0f, colors[0] },
        Instance{ -2.0f, +2.0f,  0.0f, colors[1] },
        Instance{ +2.0f, +2.0f, 15.0f, colors[2] },
        Instance{ +6.0f, +2.0f, 15.0f, colors[3] },
        Instance{ -6.0f, -2.0f, 30.0f, colors[4] },
        Instance{ -2.0f, -2.0f, 30.0f, colors[5] },
        Instance{ +2.0f, -2.0f, 45.0f, colors[6] },
        Instance{ +6.0f, -2.0f, 45.0f, colors[7] },
    };

    LLGL_VERIFY(sizeof(instanceData)/sizeof(instanceData[0]) == numInstances);

    // Vertex three vertex buffers
    BufferDescriptor bufVertexDesc;
    {
        bufVertexDesc.size      = sizeof(StandardVertex)*numVertices + sizeof(Instance)*numInstances;
        bufVertexDesc.bindFlags = BindFlags::VertexBuffer;
    }
    CREATE_BUFFER(buf1, bufVertexDesc, "buf1{Vertices+Instances}", nullptr);

    // Write all vertices and then all instances into 'buf1'
    const std::uint64_t vertexDataSize = sizeof(StandardVertex)*meshData.vertices.size();
    renderer->WriteBuffer(*buf1, 0, meshData.vertices.data(), vertexDataSize);
    renderer->WriteBuffer(*buf1, vertexDataSize, instanceData, sizeof(instanceData));

    BufferDescriptor bufInstanceDesc;
    {
        bufInstanceDesc.size       = sizeof(Instance)*numInstances;
        bufInstanceDesc.bindFlags  = BindFlags::VertexBuffer;
    }
    CREATE_BUFFER(buf2, bufInstanceDesc, "buf2{Instances}", nullptr);
    CREATE_BUFFER(buf3, bufInstanceDesc, "buf3{InstanceMatrix+InstanceColors}", nullptr);

    // Write all instances as one unit each into 'buf2'
    renderer->WriteBuffer(*buf2, 0, instanceData, sizeof(instanceData));

    // Write instance colors after all matrices for 'buf3'
    const std::uint64_t bug3InstanceColorsBaseOffset = sizeof(Gs::Matrix4f)*numInstances;
    for_range(i, numInstances)
    {
        // Stress-test WriteBuffer() function by writing both components for each vertex one by one
        renderer->WriteBuffer(
            *buf3,
            sizeof(Gs::Matrix4f)*i,
            &(instanceData[i].matrix),
            sizeof(Gs::Matrix4f)
        );
        renderer->WriteBuffer(
            *buf3,
            bug3InstanceColorsBaseOffset + sizeof(ColorRGBAub)*i,
            &(instanceData[i].color),
            sizeof(ColorRGBAub)
        );
    }

    // Define vertex attributes for all test cases
    #define DECL_ATTRIBS_VERTEX(STRIDE, SLOT, BASE_OFFSET)                                                                                                                  \
        VertexAttribute{ "position", Format::RGB32Float, /*reg:*/ 0, static_cast<std::uint32_t>((BASE_OFFSET) + offsetof(StandardVertex, position)), (STRIDE), (SLOT) },    \
        VertexAttribute{ "normal",   Format::RGB32Float, /*reg:*/ 1, static_cast<std::uint32_t>((BASE_OFFSET) + offsetof(StandardVertex, normal  )), (STRIDE), (SLOT) },    \
        VertexAttribute{ "texCoord", Format::RG32Float,  /*reg:*/ 2, static_cast<std::uint32_t>((BASE_OFFSET) + offsetof(StandardVertex, texCoord)), (STRIDE), (SLOT) },

    #define DECL_ATTRIBS_MATRIX(STRIDE, SLOT, BASE_OFFSET)                                                                                                          \
        VertexAttribute{ "instanceMatrix", 0, Format::RGBA32Float, /*reg:*/ 3, static_cast<std::uint32_t>((BASE_OFFSET)                   ), (STRIDE), (SLOT), 1 }, \
        VertexAttribute{ "instanceMatrix", 1, Format::RGBA32Float, /*reg:*/ 4, static_cast<std::uint32_t>((BASE_OFFSET) + sizeof(float)* 4), (STRIDE), (SLOT), 1 }, \
        VertexAttribute{ "instanceMatrix", 2, Format::RGBA32Float, /*reg:*/ 5, static_cast<std::uint32_t>((BASE_OFFSET) + sizeof(float)* 8), (STRIDE), (SLOT), 1 }, \
        VertexAttribute{ "instanceMatrix", 3, Format::RGBA32Float, /*reg:*/ 6, static_cast<std::uint32_t>((BASE_OFFSET) + sizeof(float)*12), (STRIDE), (SLOT), 1 },

    #define DECL_ATTRIBS_COLOR(STRIDE, SLOT, BASE_OFFSET) \
        VertexAttribute{ "instanceColor", Format::RGBA8UNorm, /*reg:*/ 7, static_cast<std::uint32_t>(BASE_OFFSET), (STRIDE), (SLOT), 1 },

    VertexAttribute vertexAttribs[NumInputLayouts][8] =
    {
        // InputLayout_2Buffers
        {
            DECL_ATTRIBS_VERTEX(sizeof(StandardVertex), 0, 0)
            DECL_ATTRIBS_MATRIX(sizeof(Instance),       1, 0)
            DECL_ATTRIBS_COLOR (sizeof(Instance),       1, offsetof(Instance, color))
        },

        // InputLayout_1Buffer2Bindings
        {
            DECL_ATTRIBS_VERTEX(sizeof(StandardVertex), 0, 0)
            DECL_ATTRIBS_MATRIX(sizeof(Instance),       1, 0)
            DECL_ATTRIBS_COLOR (sizeof(Instance),       1, offsetof(Instance, color))
        },

        // InputLayout_3Buffers
        {
            DECL_ATTRIBS_VERTEX(sizeof(StandardVertex), 0, 0)
            DECL_ATTRIBS_MATRIX(sizeof(Gs::Matrix4f),   1, 0)
            DECL_ATTRIBS_COLOR (sizeof(ColorRGBAub),    2, 0)
        },

        // InputLayout_3BuffersBaseOffset
        {
            DECL_ATTRIBS_VERTEX(sizeof(StandardVertex), 0, 0)
            DECL_ATTRIBS_MATRIX(sizeof(Instance),       1, 0)
            DECL_ATTRIBS_COLOR (sizeof(ColorRGBAub),    2, 0)
        },
    };

    #undef DECL_ATTRIBS_VERTEX
    #undef DECL_ATTRIBS_MATRIX
    #undef DECL_ATTRIBS_COLOR

    const SmallVector<VertexBufferView, 3> vertexBufferViews[NumInputLayouts] =
    {
        // InputLayout_2Buffers
        {
            VertexBufferView{ buf1, sizeof(StandardVertex), 0 },
            VertexBufferView{ buf2, sizeof(Instance),       0 },
        },

        // InputLayout_1Buffer2Bindings
        {
            VertexBufferView{ buf1, sizeof(StandardVertex), 0 },
            VertexBufferView{ buf1, sizeof(Instance),       sizeof(StandardVertex)*numVertices },
        },

        // InputLayout_3Buffers
        {
            VertexBufferView{ buf1, sizeof(StandardVertex), 0 },
            VertexBufferView{ buf3, sizeof(Gs::Matrix4f),   0 },
            VertexBufferView{ buf2, sizeof(ColorRGBAub),    0 },
        },

        // InputLayout_3BuffersBaseOffset
        {
            VertexBufferView{ buf1, sizeof(StandardVertex), 0 },
            VertexBufferView{ buf2, sizeof(Instance),       0 },
            VertexBufferView{ buf3, sizeof(ColorRGBAub),    sizeof(Gs::Matrix4f)*numInstances },
        },
    };

    // Create PSO with current vertex input layout
    PipelineState* pso[NumInputLayouts] = {};

    GraphicsPipelineDescriptor psoDesc;
    {
        psoDesc.pipelineLayout      = layouts[PipelineSolid];
        psoDesc.renderPass          = swapChain->GetRenderPass();
        psoDesc.vertexShader        = shaders[VSSolidInstanced];
        psoDesc.fragmentShader      = shaders[PSSolid];
        psoDesc.primitiveTopology   = PrimitiveTopology::TriangleList;
        psoDesc.rasterizer.cullMode = CullMode::Back;
        psoDesc.depth.testEnabled   = true;
        psoDesc.depth.writeEnabled  = true;
    }

    for_range(layoutIndex, NumInputLayouts)
    {
        const std::string psoName = "psoVertexArray[" + std::to_string(layoutIndex) + "]";
        {
            psoDesc.debugName           = psoName.c_str();
            psoDesc.inputVertexAttribs  = vertexAttribs[layoutIndex];
        }
        CREATE_GRAPHICS_PSO_EXT(pso[layoutIndex], psoDesc, psoDesc.debugName);
    }

    // Create vertex buffer arrays for second frame
    BufferArray* bufferArrays[NumInputLayouts] = {};
    if (frame > 0)
    {
        for_range(layoutIndex, NumInputLayouts)
            bufferArrays[layoutIndex] = renderer->CreateBufferArray(vertexBufferViews[layoutIndex]);
    }

    // Update scene constants
    sceneConstants = SceneConstants{};

    Gs::Matrix4f vMatrix;
    vMatrix.LoadIdentity();
    Gs::Translate(vMatrix, Gs::Vector3f{ 0, 0, -17 });
    vMatrix.MakeInverse();

    sceneConstants.vpMatrix = projection * vMatrix;
    sceneConstants.wMatrix.LoadIdentity();

    // Render scene
    Texture* readbackTex = nullptr;

    const IndexedTriangleMesh& mesh = models[ModelCube];

    BEGIN();
    {
        cmdBuffer->UpdateBuffer(*sceneCbuffer, 0, &sceneConstants, sizeof(sceneConstants));
        cmdBuffer->SetIndexBuffer(*meshBuffer, Format::R32UInt, mesh.indexBufferOffset);

        cmdBuffer->BeginRenderPass(*swapChain);
        {
            cmdBuffer->SetViewport(swapChain->GetResolution());
            cmdBuffer->Clear(ClearFlags::ColorDepth, bgColorLightBlue);

            // Draw geometry in all input layout variations
            std::uint32_t firstInstance = 0;
            for_range(layoutIndex, NumInputLayouts)
            {
                cmdBuffer->SetPipelineState(*pso[layoutIndex]);
                cmdBuffer->SetResource(0, *sceneCbuffer);

                // Bind as loose buffers in first frame and as pre-allocated buffer arrays in the second one
                if (frame == 0)
                    cmdBuffer->SetVertexBuffers(static_cast<std::uint32_t>(vertexBufferViews[layoutIndex].size()), vertexBufferViews[layoutIndex].data());
                else
                    cmdBuffer->SetVertexBufferArray(*bufferArrays[layoutIndex]);

                cmdBuffer->DrawIndexedInstanced(mesh.numIndices, numInstancesPerDraw, /*firstIndex:*/ 0, /*vertexOffset:*/ 0, firstInstance);
                firstInstance += numInstancesPerDraw;
            }

            // Capture framebuffer in current iteration
            readbackTex = CaptureFramebuffer(*cmdBuffer, swapChain->GetColorFormat(), opt.resolution);
        }
        cmdBuffer->EndRenderPass();
    }
    END();

    // Match readback texture against reference
    const std::string colorBufferName = std::string("VertexArray_") + (frame == 0 ? "OnTheFly" : "BufferArray");
    SaveCapture(readbackTex, colorBufferName);
    const DiffResult diff = DiffImages(colorBufferName);

    // Clear resources
    for_range(layoutIndex, NumInputLayouts)
    {
        renderer->Release(*pso[layoutIndex]);
        if (frame == 1 && bufferArrays[layoutIndex] != nullptr)
            renderer->Release(*bufferArrays[layoutIndex]);
    }

    renderer->Release(*buf1);
    renderer->Release(*buf2);
    renderer->Release(*buf3);

    // Evaluate readback result
    TestResult intermediateResult = diff.Evaluate("vertex array", frame);
    if (intermediateResult != TestResult::Passed)
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    return result;
}

