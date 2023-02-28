/*
 * D3D12StagingDescriptorHeap.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_H
#define LLGL_D3D12_STAGING_DESCRIPTOR_HEAP_H


#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12Device;

// D3D12 descriptor heap wrapper to manager shader-visible descriptor heaps.
class D3D12StagingDescriptorHeap
{

    public:

        D3D12StagingDescriptorHeap() = default;

        // Creates the native D3D descriptor heap. This is always a shader-visible descriptor heap.
        D3D12StagingDescriptorHeap(
            D3D12Device&                device,
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            UINT                        size
        );

        D3D12StagingDescriptorHeap(D3D12StagingDescriptorHeap&& rhs);
        D3D12StagingDescriptorHeap& operator = (D3D12StagingDescriptorHeap&& rhs);

        D3D12StagingDescriptorHeap(const D3D12StagingDescriptorHeap&) = delete;
        D3D12StagingDescriptorHeap& operator = (const D3D12StagingDescriptorHeap&) = delete;

        // Creates a new descriptor heap and resets the writing offset.
        void Create(
            D3D12Device&                device,
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            UINT                        size
        );

        // Resets the writing offset.
        void Reset();

        // Returns true if the remaining heap size can fit the specified number of descriptors.
        bool Capacity(UINT count) const;

        // Copies the specified source descriptors into the native D3D descriptor heap.
        void CopyDescriptors(
            ID3D12Device*               device,
            D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
            UINT                        firstDescriptor,
            UINT                        numDescriptors
        );

        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandleForHeapStart() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(UINT descriptor) const;

        // Increments the offset for the next range of descriptor handles.
        void IncrementOffset(UINT stride);

        // Returns the native D3D descriptor heap.
        inline ID3D12DescriptorHeap* GetNative() const
        {
            return native_.Get();
        }

        // Returns the native D3D12 descriptor heap type.
        inline D3D12_DESCRIPTOR_HEAP_TYPE GetType() const
        {
            return type_;
        }

        // Returns the size (in number of descriptors) of the native D3D descriptor heap.
        inline UINT GetSize() const
        {
            return size_;
        }

        // Returns the current writing offset for the next descriptor.
        inline UINT GetOffset() const
        {
            return offset_;
        }

    private:

        ComPtr<ID3D12DescriptorHeap>    native_;
        D3D12_DESCRIPTOR_HEAP_TYPE      type_   = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
        UINT                            size_   = 0;
        UINT                            offset_ = 0;
        UINT                            stride_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
