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


namespace LLGL
{


class D3D12ResourceHeap final : public ResourceHeap
{

    public:

        void SetName(const char* name) override;

    public:

        D3D12ResourceHeap(ID3D12Device* device, const ResourceHeapDescriptor& desc);

        // Returns the array of D3D descriptor heaps.
        inline ID3D12DescriptorHeap* const* GetDescriptorHeaps() const
        {
            return descriptorHeaps_;
        }

        // Returns the number of D3D descriptor heap (either 1 or 2).
        inline UINT GetNumDescriptorHeaps() const
        {
            return numDescriptorHeaps_;
        }

    private:

        D3D12_CPU_DESCRIPTOR_HANDLE CreateHeapTypeCbvSrvUav(ID3D12Device* device, const ResourceHeapDescriptor& desc);
        D3D12_CPU_DESCRIPTOR_HANDLE CreateHeapTypeSampler(ID3D12Device* device, const ResourceHeapDescriptor& desc);

        void CreateConstantBufferViews(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
            std::size_t&                    bindingIndex
        );

        void CreateShaderResourceViews(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
            std::size_t&                    bindingIndex
        );

        void CreateUnorderedAccessViews(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle,
            std::size_t&                    bindingIndex
        );

        void CreateSamplers(
            ID3D12Device*                   device,
            const ResourceHeapDescriptor&   desc,
            D3D12_CPU_DESCRIPTOR_HANDLE&    cpuDescHandle
        );

        void AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap);

    private:

        ComPtr<ID3D12DescriptorHeap>    heapTypeCbvSrvUav_;
        ComPtr<ID3D12DescriptorHeap>    heapTypeSampler_;

        ID3D12DescriptorHeap*           descriptorHeaps_[2] = {};   // References to the ComPtr objects
        UINT                            numDescriptorHeaps_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
