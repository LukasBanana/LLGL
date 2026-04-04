/*
 * WGCommandBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_COMMAND_BUFFER_H
#define LLGL_WG_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGCommandBuffer final : public CommandBuffer
{

    public:

        #include <LLGL/Backend/CommandBuffer.inl>

    public:

        WGCommandBuffer(WGPUDevice device, const CommandBufferDescriptor& desc);

    private:

        void EnsureComputeEncoder();
        void FlushPassEncoders();

    private:

        WGPUDevice              device_             = nullptr;
        WGPUCommandEncoder      commandEncoder_     = nullptr;
        WGPURenderPassEncoder   renderPassEncoder_  = nullptr;
        WGPUComputePassEncoder  computePassEncoder_ = nullptr;
        WGPUCommandBuffer       commandBuffer_      = nullptr;


};


} // /namespace LLGL


#endif



// ================================================================================
