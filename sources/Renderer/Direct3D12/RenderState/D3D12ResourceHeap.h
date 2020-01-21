/*
 * D3D12ResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RESOURCE_HEAP_H
#define LLGL_D3D12_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <vector>
#include <cstddef>


namespace LLGL
{


struct ResourceHeapDescriptor;
struct D3D12RootParameterLayout;

class D3D12ResourceHeap final : public ResourceHeap
{

    public:

        void SetName(const char* name) override;

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        D3D12ResourceHeap(ID3D12Device* device, const ResourceHeapDescriptor& desc);

        // Inserts the resource barriers for the specified descritpor set into the command list.
        void InsertResourceBarriers(ID3D12GraphicsCommandList* commandList, UINT firstSet);

        // Returns the array of D3D descriptor heaps.
        inline ID3D12DescriptorHeap* const* GetDescriptorHeaps() const
        {
            return descriptorHeaps_;
        }

        // Returns the strides of GPU descriptor handles.
        inline const UINT* GetDescriptorHandleStrides() const
        {
            return descriptorHandleStrides_;
        }

        // Returns the number of D3D descriptor heap (either 1 or 2).
        inline UINT GetNumDescriptorHeaps() const
        {
            return numDescriptorHeaps_;
        }

        // Returns true if this resource heap has graphics root descriptors.
        inline bool HasGraphicsDescriptors() const
        {
            return hasGraphicsDescriptors_;
        }

        // Returns true if this resource heap has compute root descriptors.
        inline bool HasComputeDescriptors() const
        {
            return hasComputeDescriptors_;
        }

    private:

        D3D12_CPU_DESCRIPTOR_HANDLE CreateHeapTypeCbvSrvUav(ID3D12Device* device, const ResourceHeapDescriptor& desc);
        D3D12_CPU_DESCRIPTOR_HANDLE CreateHeapTypeSampler(ID3D12Device* device, const ResourceHeapDescriptor& desc);

        void CreateConstantBufferViews(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
            std::size_t&                    bindingIndex,
            std::size_t                     firstResourceIndex,
            const D3D12RootParameterLayout& rootParameterLayout
        );

        void CreateShaderResourceViews(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
            std::size_t&                    bindingIndex,
            std::size_t                     firstResourceIndex,
            const D3D12RootParameterLayout& rootParameterLayout
        );

        void CreateUnorderedAccessViews(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
            std::size_t&                    bindingIndex,
            std::size_t                     firstResourceIndex,
            const D3D12RootParameterLayout& rootParameterLayout
        );

        void CreateSamplers(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
            std::size_t&                    bindingIndex,
            std::size_t                     firstResourceIndex,
            const D3D12RootParameterLayout& rootParameterLayout
        );

        void AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap);

        void AppendUAVBarrier(ID3D12Resource* resource);

        UINT GetBarrierOffset(UINT firstSet) const;
        UINT GetBarrierCount(UINT firstSet) const;

    private:

        ComPtr<ID3D12DescriptorHeap>        heapTypeCbvSrvUav_;
        ComPtr<ID3D12DescriptorHeap>        heapTypeSampler_;

        ID3D12DescriptorHeap*               descriptorHeaps_[2]         = {};   // References to the ComPtr objects
        UINT                                descriptorHandleStrides_[2] = {};
        UINT                                numDescriptorHeaps_         = 0;    // Sizes of descriptor heaps array
        UINT                                numDescriptorSets_          = 0;    // Only used for 'GetNumDescriptorSets'

        std::vector<D3D12_RESOURCE_BARRIER> barriers_;                          // UAV barriers (TODO: also transition barriers)
        std::vector<UINT>                   barrierOffsets_;                    // Offsets into the barrier array for each descriptor set; array is either empty or has N+1 elements

        bool                                hasGraphicsDescriptors_     = false;
        bool                                hasComputeDescriptors_      = false;

};


} // /namespace LLGL


#endif



// ================================================================================
