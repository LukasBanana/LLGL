/*
 * WGCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_COMMAND_BUFFER_H
#define LLGL_WG_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <LLGL/Constants.h>
#include <webgpu/webgpu.h>

#include "../WGSwapChain.h"


namespace LLGL
{


class WGCommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        WGCommandBuffer(WGPUDevice device, WGPUQueue queue, const CommandBufferDescriptor& desc);

    private:

        static constexpr std::uint32_t maxNumVertexBuffers  = 16;

        enum DirtyBits
        {
            // Render encoder
            DirtyBit_Viewports      = (1 << 0),
            DirtyBit_Scissors       = (1 << 1),
            DirtyBit_VertexBuffers  = (1 << 2),
            DirtyBit_IndexBuffer    = (1 << 3),
        };

        struct WGRenderEncoderState
        {
            WGPUBuffer      vertexBuffers[maxNumVertexBuffers]  = {};
            std::uint32_t   vertexBufferCount                   = 0;
            WGPUBuffer      indexBuffer                         = nullptr;
            std::uint64_t   indexBufferOffset                   = 0;
            WGPUIndexFormat indexBufferFormat                   = WGPUIndexFormat_Undefined;
            Scissor         scissor;
            Viewport        viewport;
        };

    private:

        void EnsureComputeEncoder();
        void FlushPassEncoders();
        void FlushRenderEncoderStates();
        void ResetRenderStates();

        void EmplaceVertexBuffer(WGPUBuffer wgpuBuffer);
        void EmplaceVertexBuffers(const WGPUBuffer* wgpuBuffers, std::size_t count);
        void EmplaceIndexBuffer(WGPUBuffer wgpuBuffer, WGPUIndexFormat indexFormat, std::uint64_t offset);

    private:

        WGPUDevice              device_             = nullptr;
        WGPUQueue               queue_              = nullptr;
        WGPUCommandEncoder      commandEncoder_     = nullptr;
        WGPURenderPassEncoder   renderPassEncoder_  = nullptr;
        WGPUComputePassEncoder  computePassEncoder_ = nullptr;
        WGPUCommandBuffer       commandBuffer_      = nullptr;

        WGRenderEncoderState    renderEncoderState_;

        std::uint32_t           renderDirtyBits_    = 0;
        std::uint32_t           computeDirtyBits_   = 0;

        const bool              isImmediateSubmit_  = false;

};


} // /namespace LLGL


#endif



// ================================================================================
