/*
 * D3D12ResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RESOURCE_HEAP_H
#define LLGL_D3D12_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12ResourceHeap : public ResourceHeap
{

    public:

        D3D12ResourceHeap(ID3D12Device* device, const ResourceHeapDescriptor& desc);

        inline ID3D12DescriptorHeap* const* GetDescriptorHeaps() const
        {
            return descriptorHeaps_;
        }

        inline UINT GetNumDescriptorHeaps() const
        {
            return numDescriptorHeaps_;
        }

    private:

        D3D12_CPU_DESCRIPTOR_HANDLE CreateHeapTypeCbvSrvUav(ID3D12Device* device, const ResourceHeapDescriptor& desc);
        D3D12_CPU_DESCRIPTOR_HANDLE CreateHeapTypeSampler(ID3D12Device* device, const ResourceHeapDescriptor& desc);

        void CreateConstantBufferViews(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc);
        void CreateShaderResourceViews(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc);
        void CreateUnorderedAccessViews(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc);
        void CreateSamplers(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE& cpuDescHandle, const ResourceHeapDescriptor& desc);

        void AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap);

        ComPtr<ID3D12DescriptorHeap>    heapTypeCbvSrvUav_;
        ComPtr<ID3D12DescriptorHeap>    heapTypeSampler_;

        ID3D12DescriptorHeap*           descriptorHeaps_[2] = {};   // References to the ComPtr objects
        UINT                            numDescriptorHeaps_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
