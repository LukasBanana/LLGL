/*
 * D3D12StagingDescriptorHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_H
#define LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_H


#include "D3D12DescriptorHeap.h"


namespace LLGL
{


class D3D12Device;

// D3D12 descriptor heap wrapper to manage shader-visible descriptor heaps.
class D3D12StagingDescriptorHeap final : private D3D12DescriptorHeap
{

    public:

        D3D12StagingDescriptorHeap() = default;

        D3D12StagingDescriptorHeap(D3D12StagingDescriptorHeap&& rhs) noexcept;
        D3D12StagingDescriptorHeap& operator = (D3D12StagingDescriptorHeap&& rhs) noexcept;

        // Initializes the descriptor heap with the specified type and size.
        D3D12StagingDescriptorHeap(
            ID3D12Device*               device,
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            UINT                        size
        );

        // Creates a new descriptor heap and resets the writing offset. This is always a shader-visible descriptor heap.
        void Create(
            ID3D12Device*               device,
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            UINT                        size
        );

        // Resets the writing offset.
        void ResetOffset();

        // Increments the offset for the next range of descriptor handles.
        void IncrementOffset(UINT stride);

        // Returns true if the remaining heap size can fit the specified number of descriptors.
        bool Capacity(UINT count) const;

        // Copies the specified source descriptors into the native D3D descriptor heap.
        void CopyDescriptors(
            ID3D12Device*               device,
            D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
            UINT                        firstDescriptor,
            UINT                        numDescriptors
        );

        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandleWithOffset() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandleWithOffset(UINT descriptor) const;

        // Returns the native D3D descriptor heap.
        inline ID3D12DescriptorHeap* GetNative() const
        {
            return D3D12DescriptorHeap::GetNative();
        }

        // Returns the native D3D12 descriptor heap type.
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetType() const
        {
            return D3D12DescriptorHeap::GetType();
        }

        // Returns the size (in number of descriptors) of the native D3D descriptor heap.
        inline UINT GetSize() const
        {
            return D3D12DescriptorHeap::GetSize();
        }

        // Returns the stride (in bytes) for each descriptor within the heap.
        inline UINT GetStride() const
        {
            return D3D12DescriptorHeap::GetStride();
        }

        // Returns the current writing offset for the next descriptor.
        inline UINT GetOffset() const
        {
            return offset_;
        }

    private:

        UINT offset_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
