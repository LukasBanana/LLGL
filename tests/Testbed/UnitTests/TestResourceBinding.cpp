/*
 * TestResourceBinding.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Testbed.h"
#include <LLGL/Utils/Parse.h>
#include <Gauss/Vector4.h>


static bool VectorsEqual(const Gs::Vector4i& lhs, const Gs::Vector4i& rhs)
{
    return
    (
        lhs[0] == rhs[0] &&
        lhs[1] == rhs[1] &&
        lhs[2] == rhs[2] &&
        lhs[3] == rhs[3]
    );
}

#define SAFE_RELEASE(OBJ)           \
    if (OBJ != nullptr)             \
    {                               \
        renderer->Release(*OBJ);    \
        OBJ = nullptr;              \
    }

/*
This test is primarily aiming at the D3D11 backend to ensure the automatic unbinding of R/W resources is working correctly (see D3D11BindingTable, D3DBindingLocator).
Bind buffer and texture resources as SRV and UAV in an alternating fashion and across both graphics and compute stages.
*/
DEF_TEST( ResourceBinding )
{
    #if 1
    if (renderer->GetRendererID() == RendererID::Metal)
    {
        //TODO: temporarily disable this test for Metal as it's currently not supported
        return TestResult::Skipped;
    }
    #endif

    enum PSOList
    {
        GraphicsPSO = 0,
        GraphicsPSOResourceHeap,
        ComputePSO,
        ComputePSOResourceHeap,

        NumPSOs,
    };

    struct ExpectedResults
    {
        Gs::Vector4i buffers[4];
        Gs::Vector4i textures[4];
    };

    static TestResult result = TestResult::Passed;
    static RenderPass* renderPass = nullptr;
    static PipelineLayout* psoLayout[NumPSOs] = {};
    static PipelineState* pso[NumPSOs] = {};
    static Buffer* buffers[4] = {};
    static Buffer* intermediateBuffer = nullptr;
    static Texture* textures[4] = {};
    static RenderTarget* renderTargets[2] = {};
    static ResourceHeap* graphicsResourceHeaps[2] = {};
    static ResourceHeap* computeResourceHeaps[2] = {};

    if (frame == 0)
    {
        result = TestResult::Passed;
        renderPass = nullptr;
        memset(psoLayout, 0, sizeof(psoLayout));
        memset(pso, 0, sizeof(pso));
        memset(buffers, 0, sizeof(buffers));
        intermediateBuffer = nullptr;
        memset(textures, 0, sizeof(textures));
        memset(renderTargets, 0, sizeof(renderTargets));
        memset(graphicsResourceHeaps, 0, sizeof(graphicsResourceHeaps));
        memset(computeResourceHeaps, 0, sizeof(computeResourceHeaps));
    }

    if (shaders[VSResourceBinding] == nullptr || shaders[PSResourceBinding] == nullptr || shaders[CSResourceBinding] == nullptr)
    {
        Log::Errorf("Missing shaders for backend\n");
        return TestResult::FailedErrors;
    }

    auto CreateBuffersAndTextures = [this]() -> void
    {
        SAFE_RELEASE(intermediateBuffer);

        // Create in/out resources
        BufferDescriptor bufDesc;
        {
            bufDesc.size        = sizeof(std::int32_t)*4;
            bufDesc.format      = Format::RGBA32SInt;
            bufDesc.bindFlags   = BindFlags::Sampled | BindFlags::Storage | BindFlags::CopySrc | BindFlags::CopyDst;
        }

        TextureDescriptor texDesc;
        {
            texDesc.type        = TextureType::Texture1D;
            texDesc.format      = Format::RGBA32SInt;
            texDesc.extent      = { 1, 1, 1 };
            texDesc.bindFlags   = BindFlags::ColorAttachment | BindFlags::Storage | BindFlags::Sampled | BindFlags::CopySrc | BindFlags::CopyDst;
            texDesc.miscFlags   = MiscFlags::NoInitialData;
        }

        for_range(i, 4)
        {
            SAFE_RELEASE(buffers[i]);
            const std::string bufName = "RWBuffer<int4>[" + std::to_string(i) + "]";
            bufDesc.debugName = bufName.c_str();
            buffers[i] = renderer->CreateBuffer(bufDesc);

            SAFE_RELEASE(textures[i]);
            const std::string texName = "RWTexture1D<int4>[" + std::to_string(i) + "]";
            texDesc.debugName = texName.c_str();
            textures[i] = renderer->CreateTexture(texDesc);
        }

        bufDesc.debugName = "RWBuffer<int4>.intermediate";
        intermediateBuffer = renderer->CreateBuffer(bufDesc);

        #if 0 //TODO
        // Create extra texture with multiple MIP maps
        TextureDescriptor texDescMips;
        {
            texDescMips.debugName   = "RWTexture1D<int4>.mips[3]";
            texDescMips.type        = TextureType::Texture1D;
            texDescMips.format      = Format::RGBA32SInt;
            texDescMips.extent      = { 4, 1, 1 };
            texDescMips.bindFlags   = BindFlags::ColorAttachment | BindFlags::Storage | BindFlags::Sampled | BindFlags::CopySrc | BindFlags::CopyDst;
            texDescMips.miscFlags   = MiscFlags::NoInitialData;
        }
        textures[4] = renderer->CreateTexture(texDescMips);
        #endif

        #if 0 //TODO
        // Define subresource views to read and write to texture resource simultaneously, but at different MIP levels
        ResourceViewDescriptor texture4ResView0;
        texture4ResView0.resource                                = textures[4];
        texture4ResView0.textureView.type                        = TextureType::Texture1D;
        texture4ResView0.textureView.format                      = textures[4]->GetFormat();
        texture4ResView0.textureView.subresource.baseMipLevel    = 0;

        ResourceViewDescriptor texture4ResView2;
        texture4ResView2.resource                                = textures[4];
        texture4ResView2.textureView.type                        = TextureType::Texture1D;
        texture4ResView2.textureView.format                      = textures[4]->GetFormat();
        texture4ResView2.textureView.subresource.baseMipLevel    = 2;
        #endif

        // Create resource heaps
        SAFE_RELEASE(graphicsResourceHeaps[0]);
        SAFE_RELEASE(graphicsResourceHeaps[1]);

        graphicsResourceHeaps[0] = renderer->CreateResourceHeap(psoLayout[GraphicsPSOResourceHeap], { buffers[0], buffers[1], buffers[2], buffers[3], textures[0] });
        graphicsResourceHeaps[1] = renderer->CreateResourceHeap(psoLayout[GraphicsPSOResourceHeap], { buffers[0], buffers[3], buffers[1], buffers[2], textures[2] });

        SAFE_RELEASE(computeResourceHeaps[0]);
        SAFE_RELEASE(computeResourceHeaps[1]);

        computeResourceHeaps[0] = renderer->CreateResourceHeap(psoLayout[ComputePSOResourceHeap], { buffers[0], buffers[1], buffers[2], buffers[3], textures[0] });
        computeResourceHeaps[1] = renderer->CreateResourceHeap(psoLayout[ComputePSOResourceHeap], { buffers[0], buffers[3], buffers[1], buffers[2], textures[2] });
    };

    auto CreateRenderTargets = [this]() -> void
    {
        // Create render targets for graphics PSO output
        for_range(i, 2)
        {
            const Extent3D texExtent = textures[i*2+0]->GetMipExtent(0);
            RenderTargetDescriptor renderTargetDesc;
            {
                renderTargetDesc.renderPass             = renderPass;
                renderTargetDesc.resolution.width       = texExtent.width;
                renderTargetDesc.resolution.height      = texExtent.height;
                renderTargetDesc.colorAttachments[0]    = textures[i*2+0];
                renderTargetDesc.colorAttachments[1]    = textures[i*2+1];
            }
            renderTargets[i] = renderer->CreateRenderTarget(renderTargetDesc);
        }
    };

    if (frame == 0)
    {
        result = TestResult::Passed;

        // Create render pass for two color attachments
        RenderPassDescriptor renderPassDesc;
        {
            renderPassDesc.debugName                    = "ResourceBinding.RenderPass";
            renderPassDesc.colorAttachments[0].format   = Format::RGBA32SInt;
            renderPassDesc.colorAttachments[0].storeOp  = AttachmentStoreOp::Store;
            renderPassDesc.colorAttachments[1].format   = Format::RGBA32SInt;
            renderPassDesc.colorAttachments[1].storeOp  = AttachmentStoreOp::Store;
        }
        renderPass = renderer->CreateRenderPass(renderPassDesc);

        // Create graphics PSOs
        psoLayout[GraphicsPSO] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "buffer(inBufferA@0):vert:frag,"
                "buffer(inBufferB@1):vert:frag,"
                "rwbuffer(outBufferA@2):vert:frag,"
                "rwbuffer(outBufferB@4):frag,"
                "texture(inTextureA@%d):frag,"
                "texture(inTextureB@%d):frag,"
                "barriers{rw},",
                this->HasUniqueBindingSlots() ? 5 : 2,
                this->HasUniqueBindingSlots() ? 6 : 4
            )
        );

        psoLayout[GraphicsPSOResourceHeap] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "heap{"
                "  buffer(inBufferA@0):vert:frag,"
                "  buffer(inBufferB@1):vert:frag,"
                "  rwbuffer(outBufferA@2):vert:frag,"
                "  rwbuffer(outBufferB@4):frag,"
                "  texture(inTextureA@%d):frag,"
                "},"
                "texture(inTextureB@%d):frag,"
                "barriers{rw},",
                this->HasUniqueBindingSlots() ? 5 : 2,
                this->HasUniqueBindingSlots() ? 6 : 4
            )
        );

        // Create first graphics PSO with individual resource bindings
        GraphicsPipelineDescriptor psoDescGraphics;
        {
            psoDescGraphics.debugName           = "ResourceBinding.Gfx.PSO";
            psoDescGraphics.pipelineLayout      = psoLayout[GraphicsPSO];
            psoDescGraphics.renderPass          = renderPass;
            psoDescGraphics.vertexShader        = shaders[VSResourceBinding];
            psoDescGraphics.fragmentShader      = shaders[PSResourceBinding];
            psoDescGraphics.primitiveTopology   = LLGL::PrimitiveTopology::PointList;
        }
        pso[GraphicsPSO] = renderer->CreatePipelineState(psoDescGraphics);

        // Create second graphics PSO with resource heap
        {
            psoDescGraphics.debugName           = "ResourceBinding.Gfx.ResHeap-PSO";
            psoDescGraphics.pipelineLayout      = psoLayout[GraphicsPSOResourceHeap];
        }
        pso[GraphicsPSOResourceHeap] = renderer->CreatePipelineState(psoDescGraphics);

        // Create compute PSOs
        psoLayout[ComputePSO] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "buffer(inBufferA@0):comp,"
                "buffer(inBufferB@1):comp,"
                "rwbuffer(outBufferA@2):comp,"
                "rwbuffer(outBufferB@4):comp,"
                "texture(inTextureA@%d):comp,"
                "texture(inTextureB@%d):comp,"
                "rwtexture(outTextureA@%d):comp,"
                "rwtexture(outTextureB@%d):comp,"
                "barriers{rw},",
                this->HasUniqueBindingSlots() ? 5 : 2,
                this->HasUniqueBindingSlots() ? 6 : 4,
                this->HasUniqueBindingSlots() ? 7 : 0,
                this->HasUniqueBindingSlots() ? 8 : 1
            )
        );

        psoLayout[ComputePSOResourceHeap] = renderer->CreatePipelineLayout(
            LLGL::Parse(
                "heap{"
                "  buffer(inBufferA@0):comp,"
                "  buffer(inBufferB@1):comp,"
                "  rwbuffer(outBufferA@2):comp,"
                "  rwbuffer(outBufferB@4):comp,"
                "  texture(inTextureA@%d):comp,"
                "},"
                "texture(inTextureB@%d):comp,"
                "rwtexture(outTextureA@%d):comp,"
                "rwtexture(outTextureB@%d):comp,"
                "barriers{rw},",
                this->HasUniqueBindingSlots() ? 5 : 2,
                this->HasUniqueBindingSlots() ? 6 : 4,
                this->HasUniqueBindingSlots() ? 7 : 0,
                this->HasUniqueBindingSlots() ? 8 : 1
            )
        );

        // Create first compute PSO with individual resource bindings
        ComputePipelineDescriptor psoDescCompute;
        {
            psoDescCompute.debugName        = "ResourceBinding.Comp.PSO";
            psoDescCompute.pipelineLayout   = psoLayout[ComputePSO];
            psoDescCompute.computeShader    = shaders[CSResourceBinding];
        }
        pso[ComputePSO] = renderer->CreatePipelineState(psoDescCompute);
        {
            psoDescCompute.debugName        = "ResourceBinding.Comp.ResHeap-PSO";
            psoDescCompute.pipelineLayout   = psoLayout[ComputePSOResourceHeap];
        }
        pso[ComputePSOResourceHeap] = renderer->CreatePipelineState(psoDescCompute);

        for_range(i, NumPSOs)
        {
            if (const Report* report = pso[i]->GetReport())
            {
                if (report->HasErrors())
                {
                    Log::Errorf("PSO creation failed:\n%s", report->GetText());
                    return TestResult::FailedErrors;
                }
            }
        }

        CreateBuffersAndTextures();
        CreateRenderTargets();
    }

    auto PrintIntermediateResultsVerbose = [this, frame](const char* dispatchName, const ExpectedResults& expectedResults) -> void
    {
        if (this->opt.sanityCheck)
        {
            Log::Printf(
                Log::ColorFlags::StdAnnotation,
                "Intermediate expected results (Frame %u, %s):\n",
                frame, dispatchName
            );
            for_range(i, 4)
            {
                Log::Printf(
                    Log::ColorFlags::StdAnnotation,
                    "  buffer%d [%d, %d, %d, %d]\n",
                    i, expectedResults.buffers[i][0], expectedResults.buffers[i][1], expectedResults.buffers[i][2], expectedResults.buffers[i][3]
                );
            }
            for_range(i, 4)
            {
                Log::Printf(
                    Log::ColorFlags::StdAnnotation,
                    "  texture%d [%d, %d, %d, %d]\n",
                    i, expectedResults.textures[i][0], expectedResults.textures[i][1], expectedResults.textures[i][2], expectedResults.textures[i][3]
                );
            }
        }
    };

    auto EncodeCommandBuffer = [&](CommandBuffer& cmdBuf, ExpectedResults& expectedResults) -> void
    {
        auto InitExpectedResults = [&](ExpectedResults& expectedResults) -> void
        {
            // Initialize expected result values with semi-random values
            expectedResults.buffers[0]  = { 1, 2, 3, 4 };
            expectedResults.buffers[1]  = { 16, 5, 9, 4 };
            expectedResults.buffers[2]  = { -7, -2, 3, 70 };
            expectedResults.buffers[3]  = { 9, 5, 5, 3 };
            expectedResults.textures[0] = { 15, 20, 30, 40 };
            expectedResults.textures[1] = { 20, -15, -16, -19 };
            expectedResults.textures[2] = { -8, 3, 3, 1 };
            expectedResults.textures[3] = { 60, 40, -20, -50 };

            cmdBuf.PushDebugGroup("InitExpectedResults");

            // Fill buffers with new values
            for_range(i, 4)
            {
                cmdBuf.FillBuffer(*buffers[i], sizeof(std::int32_t)*0, static_cast<std::uint32_t>(expectedResults.buffers[i][0]), sizeof(std::int32_t));
                cmdBuf.FillBuffer(*buffers[i], sizeof(std::int32_t)*1, static_cast<std::uint32_t>(expectedResults.buffers[i][1]), sizeof(std::int32_t));
                cmdBuf.FillBuffer(*buffers[i], sizeof(std::int32_t)*2, static_cast<std::uint32_t>(expectedResults.buffers[i][2]), sizeof(std::int32_t));
                cmdBuf.FillBuffer(*buffers[i], sizeof(std::int32_t)*3, static_cast<std::uint32_t>(expectedResults.buffers[i][3]), sizeof(std::int32_t));
            }

            // Fill intermediate buffer with new values and copy buffer into texture
            TextureRegion texRegion;
            texRegion.extent = { 1, 1, 1 };

            for_range(i, 4)
            {
                cmdBuf.FillBuffer(*intermediateBuffer, sizeof(std::int32_t)*0, static_cast<std::uint32_t>(expectedResults.textures[i][0]), sizeof(std::int32_t));
                cmdBuf.FillBuffer(*intermediateBuffer, sizeof(std::int32_t)*1, static_cast<std::uint32_t>(expectedResults.textures[i][1]), sizeof(std::int32_t));
                cmdBuf.FillBuffer(*intermediateBuffer, sizeof(std::int32_t)*2, static_cast<std::uint32_t>(expectedResults.textures[i][2]), sizeof(std::int32_t));
                cmdBuf.FillBuffer(*intermediateBuffer, sizeof(std::int32_t)*3, static_cast<std::uint32_t>(expectedResults.textures[i][3]), sizeof(std::int32_t));
                cmdBuf.CopyTextureFromBuffer(*textures[i], texRegion, *intermediateBuffer, 0);
            }

            cmdBuf.PopDebugGroup();

            PrintIntermediateResultsVerbose("InitExpectedResults", expectedResults);
        };

        auto DispatchOrder0 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("DispatchOrder0");
            cmdBuf.SetPipelineState(*pso[ComputePSO]);
            cmdBuf.SetResource(0, *buffers[0]); // inBufferA
            cmdBuf.SetResource(1, *buffers[1]); // inBufferB
            cmdBuf.SetResource(2, *buffers[2]); // outBufferA
            cmdBuf.SetResource(3, *buffers[3]); // outBufferB
            cmdBuf.SetResource(4, *textures[0]); // inTextureA
            cmdBuf.SetResource(5, *textures[1]); // inTextureB
            cmdBuf.SetResource(6, *textures[2]); // outTextureA
            cmdBuf.SetResource(7, *textures[3]); // outTextureB
            cmdBuf.Dispatch(1, 1, 1);
            cmdBuf.PopDebugGroup();

            // in-buffers=0,1; out-buffers=2,3; in-textures=0,1; out-textures=2,3
            expectedResults.buffers[2] = (expectedResults.buffers[0] + expectedResults.buffers[1]);
            expectedResults.buffers[3] = (expectedResults.buffers[0] - expectedResults.buffers[1]) * 2;
            expectedResults.textures[2] = (expectedResults.textures[0] + expectedResults.textures[1]);
            expectedResults.textures[3] = (expectedResults.textures[0] - expectedResults.textures[1]) * 2;

            PrintIntermediateResultsVerbose("DispatchOrder0", expectedResults);
        };

        auto DispatchOrder1 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("DispatchOrder1");
            cmdBuf.SetPipelineState(*pso[ComputePSOResourceHeap]);
            cmdBuf.SetResourceHeap(*computeResourceHeaps[1]);
            cmdBuf.SetResource(0, *textures[3]); // inTextureB
            cmdBuf.SetResource(1, *textures[0]); // outTextureA
            cmdBuf.SetResource(2, *textures[1]); // outTextureB
            cmdBuf.Dispatch(1, 1, 1);
            cmdBuf.PopDebugGroup();

            // in-buffers=0,3; out-buffers=1,2; in-textures=2,3; out-textures=0,1
            expectedResults.buffers[1] = (expectedResults.buffers[0] + expectedResults.buffers[3]);
            expectedResults.buffers[2] = (expectedResults.buffers[0] - expectedResults.buffers[3]) * 2;
            expectedResults.textures[0] = (expectedResults.textures[2] + expectedResults.textures[3]);
            expectedResults.textures[1] = (expectedResults.textures[2] - expectedResults.textures[3]) * 2;

            PrintIntermediateResultsVerbose("DispatchOrder1", expectedResults);
        };

        auto DispatchOrder2 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("DispatchOrder2");
            cmdBuf.SetPipelineState(*pso[ComputePSOResourceHeap]);
            cmdBuf.SetResourceHeap(*computeResourceHeaps[0]);
            cmdBuf.SetResource(0, *textures[1]); // inTextureB
            cmdBuf.SetResource(1, *textures[2]); // outTextureA
            cmdBuf.SetResource(2, *textures[3]); // outTextureB
            cmdBuf.Dispatch(1, 1, 1);
            cmdBuf.PopDebugGroup();

            // in-buffers=0,1; out-buffers=2,3; in-textures=0,1; out-textures=2,3
            expectedResults.buffers[2] = (expectedResults.buffers[0] + expectedResults.buffers[1]);
            expectedResults.buffers[3] = (expectedResults.buffers[0] - expectedResults.buffers[1]) * 2;
            expectedResults.textures[2] = (expectedResults.textures[0] + expectedResults.textures[1]);
            expectedResults.textures[3] = (expectedResults.textures[0] - expectedResults.textures[1]) * 2;

            PrintIntermediateResultsVerbose("DispatchOrder2", expectedResults);
        };

        auto DispatchOrder3 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("DispatchOrder3");
            cmdBuf.SetPipelineState(*pso[ComputePSO]);
            cmdBuf.SetResource(0, *buffers[0]); // inBufferA
            cmdBuf.SetResource(1, *buffers[3]); // inBufferB
            cmdBuf.SetResource(2, *buffers[1]); // outBufferA
            cmdBuf.SetResource(3, *buffers[2]); // outBufferB
            cmdBuf.SetResource(4, *textures[2]); // inTextureA
            cmdBuf.SetResource(5, *textures[3]); // inTextureB
            cmdBuf.SetResource(6, *textures[0]); // outTextureA
            cmdBuf.SetResource(7, *textures[1]); // outTextureB
            cmdBuf.Dispatch(1, 1, 1);
            cmdBuf.PopDebugGroup();

            // in-buffers=0,3; out-buffers=1,2; in-textures=2,3; out-textures=0,1
            expectedResults.buffers[1] = (expectedResults.buffers[0] + expectedResults.buffers[3]);
            expectedResults.buffers[2] = (expectedResults.buffers[0] - expectedResults.buffers[3]) * 2;
            expectedResults.textures[0] = (expectedResults.textures[2] + expectedResults.textures[3]);
            expectedResults.textures[1] = (expectedResults.textures[2] - expectedResults.textures[3]) * 2;

            PrintIntermediateResultsVerbose("DispatchOrder3", expectedResults);
        };

        auto RenderOrder0 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("RenderOrder0");
            cmdBuf.BeginRenderPass(*renderTargets[1]);
            {
                cmdBuf.SetViewport(Extent2D{ 1, 1 });
                cmdBuf.SetPipelineState(*pso[GraphicsPSO]);
                cmdBuf.SetResource(0, *buffers[0]); // inBufferA
                cmdBuf.SetResource(1, *buffers[1]); // inBufferB
                cmdBuf.SetResource(2, *buffers[2]); // outBufferA
                cmdBuf.SetResource(3, *buffers[3]); // outBufferB
                cmdBuf.SetResource(4, *textures[0]); // inTextureA
                cmdBuf.SetResource(5, *textures[1]); // inTextureB
                cmdBuf.Draw(1, 0);
            }
            cmdBuf.EndRenderPass();
            cmdBuf.PopDebugGroup();

            // in-buffers=0,1; out-buffers=2,3; in-textures=0,1; out-textures=2,3
            expectedResults.buffers[2] = (expectedResults.buffers[0] + expectedResults.buffers[1]) * 3;
            expectedResults.buffers[3] = (expectedResults.buffers[0] - expectedResults.buffers[1]) / 2;
            expectedResults.textures[2] = (expectedResults.textures[0] + expectedResults.textures[1]);
            expectedResults.textures[3] = (expectedResults.textures[0] - expectedResults.textures[1]) * 2;

            PrintIntermediateResultsVerbose("RenderOrder0", expectedResults);
        };

        auto RenderOrder1 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("RenderOrder1");
            cmdBuf.BeginRenderPass(*renderTargets[0]);
            {
                cmdBuf.SetViewport(Extent2D{ 1, 1 });
                cmdBuf.SetPipelineState(*pso[GraphicsPSOResourceHeap]);
                cmdBuf.SetResourceHeap(*graphicsResourceHeaps[1]);
                cmdBuf.SetResource(0, *textures[3]); // inTextureB
                cmdBuf.Draw(1, 0);
            }
            cmdBuf.EndRenderPass();
            cmdBuf.PopDebugGroup();

            // in-buffers=0,3; out-buffers=1,2; in-textures=2,3; out-textures=0,1
            expectedResults.buffers[1] = (expectedResults.buffers[0] + expectedResults.buffers[3]) * 3;
            expectedResults.buffers[2] = (expectedResults.buffers[0] - expectedResults.buffers[3]) / 2;
            expectedResults.textures[0] = (expectedResults.textures[2] + expectedResults.textures[3]);
            expectedResults.textures[1] = (expectedResults.textures[2] - expectedResults.textures[3]) * 2;

            PrintIntermediateResultsVerbose("RenderOrder1", expectedResults);
        };

        auto RenderOrder2 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("RenderOrder2");
            cmdBuf.BeginRenderPass(*renderTargets[1]);
            {
                cmdBuf.SetViewport(Extent2D{ 1, 1 });
                cmdBuf.SetPipelineState(*pso[GraphicsPSOResourceHeap]);
                cmdBuf.SetResourceHeap(*graphicsResourceHeaps[0]);
                cmdBuf.SetResource(0, *textures[1]); // inTextureB
                cmdBuf.Draw(1, 0);
            }
            cmdBuf.EndRenderPass();
            cmdBuf.PopDebugGroup();

            // in-buffers=0,1; out-buffers=2,3; in-textures=0,1; out-textures=2,3
            expectedResults.buffers[2] = (expectedResults.buffers[0] + expectedResults.buffers[1]) * 3;
            expectedResults.buffers[3] = (expectedResults.buffers[0] - expectedResults.buffers[1]) / 2;
            expectedResults.textures[2] = (expectedResults.textures[0] + expectedResults.textures[1]);
            expectedResults.textures[3] = (expectedResults.textures[0] - expectedResults.textures[1]) * 2;

            PrintIntermediateResultsVerbose("RenderOrder2", expectedResults);
        };

        auto RenderOrder3 = [&](ExpectedResults& expectedResults) -> void
        {
            cmdBuf.PushDebugGroup("RenderOrder3");
            cmdBuf.BeginRenderPass(*renderTargets[0]);
            {
                cmdBuf.SetViewport(Extent2D{ 1, 1 });
                cmdBuf.SetPipelineState(*pso[GraphicsPSO]);
                cmdBuf.SetResource(0, *buffers[0]); // inBufferA
                cmdBuf.SetResource(1, *buffers[3]); // inBufferA
                cmdBuf.SetResource(2, *buffers[1]); // outBufferB
                cmdBuf.SetResource(3, *buffers[2]); // outBufferB
                cmdBuf.SetResource(4, *textures[2]); // inTextureA
                cmdBuf.SetResource(5, *textures[3]); // inTextureB
                cmdBuf.Draw(1, 0);
            }
            cmdBuf.EndRenderPass();
            cmdBuf.PopDebugGroup();

            // in-buffers=0,3; out-buffers=1,2; in-textures=2,3; out-textures=0,1
            expectedResults.buffers[1] = (expectedResults.buffers[0] + expectedResults.buffers[3]) * 3;
            expectedResults.buffers[2] = (expectedResults.buffers[0] - expectedResults.buffers[3]) / 2;
            expectedResults.textures[0] = (expectedResults.textures[2] + expectedResults.textures[3]);
            expectedResults.textures[1] = (expectedResults.textures[2] - expectedResults.textures[3]) * 2;

            PrintIntermediateResultsVerbose("RenderOrder3", expectedResults);
        };

        auto RecreateResources = [&]() -> void
        {
            if (this->opt.verbose)
                Log::Printf("Recreate resources\n");

            CreateBuffersAndTextures();
            CreateRenderTargets();
        };

        // Re-create resources intermittently
        if (frame % 10 == 10 - 1)
            RecreateResources();

        cmdBuf.Begin();
        {
            // Initialize expected results and its resources
            InitExpectedResults(expectedResults);

            // Dispatch compute shaders
            switch (frame % 3)
            {
                case 0:
                    DispatchOrder0(expectedResults);
                    DispatchOrder1(expectedResults);
                    DispatchOrder2(expectedResults);
                    DispatchOrder3(expectedResults);
                    break;
                case 1:
                    DispatchOrder0(expectedResults);
                    DispatchOrder3(expectedResults);
                    DispatchOrder1(expectedResults);
                    DispatchOrder2(expectedResults);
                    break;
                case 2:
                    DispatchOrder3(expectedResults);
                    DispatchOrder2(expectedResults);
                    DispatchOrder1(expectedResults);
                    DispatchOrder0(expectedResults);
                    break;
            }

            // Render with graphics shaders
            cmdBuf.SetVertexBuffer(*meshBuffer);

            switch ((frame/2) % 3)
            {
                case 0:
                    RenderOrder0(expectedResults);
                    RenderOrder1(expectedResults);
                    RenderOrder2(expectedResults);
                    RenderOrder3(expectedResults);
                    break;
                case 1:
                    RenderOrder3(expectedResults);
                    RenderOrder2(expectedResults);
                    RenderOrder1(expectedResults);
                    RenderOrder0(expectedResults);
                    break;
                case 2:
                    RenderOrder3(expectedResults);
                    RenderOrder2(expectedResults);
                    RenderOrder0(expectedResults);
                    RenderOrder1(expectedResults);
                    break;
            }
        }
        cmdBuf.End();
    };

    // Encode dispatch and render commands to calculate values in buffer/texture
    ExpectedResults expectedResults = {};
    EncodeCommandBuffer(*cmdBuffer, expectedResults);

    // Run this test many times in full test mode to ensure resource transitioning works, but only use a few iterations in fast mode
    const unsigned numFrames = (opt.fastTest ? 10 : 1000);

    // Evaluate readback result
    TestResult intermediateResult = TestResult::Passed;

    for_range(i, 4)
    {
        // Evaluate buffer
        Gs::Vector4i readbackValue = { 0, 0, 0, 0 };
        renderer->ReadBuffer(*buffers[i], 0, readbackValue.Ptr(), sizeof(readbackValue));
        if (!VectorsEqual(readbackValue, expectedResults.buffers[i]))
        {
            Log::Errorf(
                "Mismatch between buffer %d (Frame %u) [%d, %d, %d, %d] and expected value [%d, %d, %d, %d]\n",
                i, frame,
                readbackValue[0], readbackValue[1], readbackValue[2], readbackValue[3],
                expectedResults.buffers[i][0], expectedResults.buffers[i][1], expectedResults.buffers[i][2], expectedResults.buffers[i][3]
            );
            intermediateResult = TestResult::FailedMismatch;
        }
    }

    for_range(i, 4)
    {
        // Evaluate texture
        Gs::Vector4i readbackValue = { 0, 0, 0, 0 };
        TextureRegion texRegion;
        {
            texRegion.extent = { 1, 1, 1 };
        }
        MutableImageView readbackImage;
        {
            readbackImage.format    = ImageFormat::RGBA;
            readbackImage.dataType  = DataType::Int32;
            readbackImage.data      = readbackValue.Ptr();
            readbackImage.dataSize  = sizeof(readbackValue);
        }
        renderer->ReadTexture(*textures[i], texRegion, readbackImage);
        if (!VectorsEqual(readbackValue, expectedResults.textures[i]))
        {
            Log::Errorf(
                "Mismatch between texture %d (Frame %u) [%d, %d, %d, %d] and expected value [%d, %d, %d, %d]\n",
                i, frame,
                readbackValue[0], readbackValue[1], readbackValue[2], readbackValue[3],
                expectedResults.textures[i][0], expectedResults.textures[i][1], expectedResults.textures[i][2], expectedResults.textures[i][3]
            );
            intermediateResult = TestResult::FailedMismatch;
        }
    }

    if (intermediateResult == TestResult::Passed)
    {
        if (opt.verbose)
            Log::Printf("Binding test passed (Frame %u)\n", frame);
    }
    else
        result = intermediateResult;

    if (intermediateResult == TestResult::Passed || opt.greedy)
    {
        if (frame + 1 < numFrames)
            return TestResult::Continue;
    }

    // Clear resources
    for_range(i, NumPSOs)
    {
        renderer->Release(*pso[i]);
        renderer->Release(*psoLayout[i]);
    }

    for_range(i, 2)
    {
        renderer->Release(*graphicsResourceHeaps[i]);
        renderer->Release(*computeResourceHeaps[i]);
        renderer->Release(*renderTargets[i]);
    }

    for_range(i, 4)
    {
        renderer->Release(*buffers[i]);
        renderer->Release(*textures[i]);
    }

    renderer->Release(*renderPass);
    renderer->Release(*intermediateBuffer);

    return result;
}

