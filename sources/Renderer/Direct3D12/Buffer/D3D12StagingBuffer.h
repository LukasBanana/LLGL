/*
 * D3D12StagingBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_STAGING_BUFFER_H
#define LLGL_D3D12_STAGING_BUFFER_H


#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


/*
Instances of this class represent a single chunk in the staging buffer pool
to handle dynamic buffer updates during command buffer recording.
*/
class D3D12StagingBuffer
{

    public:

        D3D12StagingBuffer() = default;

        // Creates the native D3D upload resource.
        D3D12StagingBuffer(
            ID3D12Device*   device,
            UINT64          size,
            UINT            alignment   = 256u,
            D3D12_HEAP_TYPE heapType    = D3D12_HEAP_TYPE_UPLOAD
        );

        D3D12StagingBuffer(D3D12StagingBuffer&& rhs) noexcept;
        D3D12StagingBuffer& operator = (D3D12StagingBuffer&& rhs) noexcept;

        D3D12StagingBuffer(const D3D12StagingBuffer&) = delete;
        D3D12StagingBuffer& operator = (const D3D12StagingBuffer&) = delete;

        // Creates a new resource and resets the writing offset.
        void Create(
            ID3D12Device*   device,
            UINT64          size,
            UINT            alignment   = 256u,
            D3D12_HEAP_TYPE heapType    = D3D12_HEAP_TYPE_UPLOAD
        );

        // Resets the writing offset.
        void Reset();

        // Returns true if the remaining buffer size can fit the specified data size.
        bool Capacity(UINT64 dataSize) const;

        // Writes the specified data to the native D3D upload buffer.
        HRESULT Write(
            ID3D12GraphicsCommandList*  commandList,
            ID3D12Resource*             dstBuffer,
            UINT64                      dstOffset,
            const void*                 data,
            UINT64                      dataSize
        );

        // Writes the specified data to the native D3D upload buffer and increments the write offset.
        HRESULT WriteAndIncrementOffset(
            ID3D12GraphicsCommandList*  commandList,
            ID3D12Resource*             dstBuffer,
            UINT64                      dstOffset,
            const void*                 data,
            UINT64                      dataSize
        );

        // Returns the native D3D resource.
        inline ID3D12Resource* GetNative() const
        {
            return native_.Get();
        }

        // Returns the size of the native D3D buffer.
        inline UINT64 GetSize() const
        {
            return size_;
        }

        // Returns the current writing offset.
        inline UINT64 GetOffset() const
        {
            return offset_;
        }

    private:

        ComPtr<ID3D12Resource>  native_;
        UINT64                  size_   = 0;
        UINT64                  offset_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
