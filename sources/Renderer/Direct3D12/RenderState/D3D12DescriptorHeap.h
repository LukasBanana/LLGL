/*
 * D3D12DescriptorHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_DESCRIPTOR_HEAP_H
#define LLGL_D3D12_DESCRIPTOR_HEAP_H


#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


// Base class for D3D12 descriptor heap wrappers.
class D3D12DescriptorHeap
{

    public:

        // Creates a native D3D12 descriptor heap or throws an exception on failure.
        static ComPtr<ID3D12DescriptorHeap> CreateNativeOrThrow(ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

    public:

        D3D12DescriptorHeap() = default;

        D3D12DescriptorHeap(D3D12DescriptorHeap&& rhs) noexcept;
        D3D12DescriptorHeap& operator = (D3D12DescriptorHeap&& rhs) noexcept;

        D3D12DescriptorHeap(const D3D12DescriptorHeap&) = delete;
        D3D12DescriptorHeap& operator = (const D3D12DescriptorHeap&) = delete;

        // Initializes the descriptor heap with the specified type, size, and flags.
        D3D12DescriptorHeap(
            ID3D12Device*               device,
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            UINT                        size,
            D3D12_DESCRIPTOR_HEAP_FLAGS flags   = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        );

        virtual ~D3D12DescriptorHeap() = default;

        // Creates the native D3D descriptor heap. This is always a shader-visible descriptor heap.
        void Create(
            ID3D12Device*               device,
            D3D12_DESCRIPTOR_HEAP_TYPE  type,
            UINT                        size,
            D3D12_DESCRIPTOR_HEAP_FLAGS flags   = D3D12_DESCRIPTOR_HEAP_FLAG_NONE
        );

        // Re-creates the descriptor heap with a new size. This will discard all previously created descritpors.
        void Reset(UINT size);

        // Resets the internal descriptor.
        void Reset();

        // Returns the CPU descriptor handle for heap start.
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandleStart() const;

        // Returns the CPU descriptor handle at the specified position within this heap.
        D3D12_CPU_DESCRIPTOR_HANDLE GetCpuHandleWithOffset(UINT offset) const;

        // Returns the GPU descriptor handle for heap start.
        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandleStart() const;

        // Returns the GPU descriptor handle at the specified position within this heap.
        D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandleWithOffset(UINT offset) const;

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

        // Returns the stride (in bytes) for each descriptor within the heap.
        inline UINT GetStride() const
        {
            return stride_;
        }

    private:

        ComPtr<ID3D12DescriptorHeap>    native_;
        D3D12_DESCRIPTOR_HEAP_TYPE      type_   = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
        UINT                            size_   = 0;
        UINT                            stride_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
