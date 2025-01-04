/*
 * D3D12CPUAccessBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_CPU_ACCESS_BUFFER_H
#define LLGL_D3D12_CPU_ACCESS_BUFFER_H


#include "D3D12StagingBuffer.h"
#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{


struct D3D12Resource;
class D3D12CommandContext;
class D3D12CommandQueue;

class D3D12CPUAccessBuffer
{

    public:

        D3D12CPUAccessBuffer() = default;
        D3D12CPUAccessBuffer(ID3D12Device* device);

        // Initializes the device object and chunk size.
        void InitializeDevice(ID3D12Device* device);

        D3D12StagingBuffer& GetUploadBufferAndGrow(UINT64 size, UINT64 alignment = 8);
        D3D12StagingBuffer& GetReadbackBufferAndGrow(UINT64 size, UINT64 alignment = 8);

        // Copies the specified subresource region into the global readback buffer and writes it into the output data.
        HRESULT ReadSubresourceRegion(
            D3D12CommandContext&    commandContext,
            D3D12CommandQueue&      commandQueue,
            D3D12Resource&          srcBuffer,
            UINT64                  srcOffset,
            void*                   data,
            UINT64                  dataSize,
            UINT64                  alignment   = 256u
        );

        HRESULT MapFeedbackBuffer(
            D3D12CommandContext&    commandContext,
            D3D12CommandQueue&      commandQueue,
            D3D12Resource&          srcBuffer,
            const D3D12_RANGE&      readRange,
            void**                  mappedData
        );
        void UnmapFeedbackBuffer();

        HRESULT MapUploadBuffer(
            SIZE_T                  size,
            void**                  mappedData
        );
        void UnmapUploadBuffer(
            D3D12CommandContext&    commandContext,
            D3D12CommandQueue&      commandQueue,
            D3D12Resource&          dstBuffer,
            const D3D12_RANGE&      writtenRange
        );

        // Returns a bitwise-OR combination of CPUAccessFlags for the buffers that are currently being mapped between GPU and CPU.
        inline long GetCurrentCPUAccessFlags() const
        {
            return currentCPUAccessFlags_;
        }

    private:

        // Resizes the specified staging buffer, but only grows its size.
        void ResizeBuffer(
            D3D12StagingBuffer& stagingBuffer,
            D3D12_HEAP_TYPE     heapType,
            UINT64              size,
            UINT64              alignment
        );

    private:

        ID3D12Device*       device_                 = nullptr;
        long                currentCPUAccessFlags_  = 0; // CPUAccessFlags

        D3D12StagingBuffer  globalUploadBuffer_;
        D3D12StagingBuffer  globalReadbackBuffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
