/*
 * D3D12ResourceHeap.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RESOURCE_HEAP_H
#define LLGL_D3D12_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Container/ArrayView.h>
#include "D3D12PipelineLayout.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <vector>
#include <cstddef>


namespace LLGL
{


struct ResourceHeapDescriptor;

class D3D12ResourceHeap final : public ResourceHeap
{

    public:

        void SetName(const char* name) override;

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        D3D12ResourceHeap(
            ID3D12Device*                               device,
            const ResourceHeapDescriptor&               desc,
            const ArrayView<ResourceViewDescriptor>&    initialResourceViews = {}
        );

        // Creates resource view handles (SRV/UAV/CBV/Sampler) for the specified resource views in the D3D12 descriptor heaps.
        std::uint32_t CreateResourceViewHandles(
            ID3D12Device*                               device,
            std::uint32_t                               firstDescriptor,
            const ArrayView<ResourceViewDescriptor>&    resourceViews
        );

        void SetGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* commandList, std::uint32_t descriptorSet);
        void SetComputeRootDescriptorTables(ID3D12GraphicsCommandList* commandList, std::uint32_t descriptorSet);

        // Inserts the resource barriers for the specified descritpor set into the command list.
        void InsertResourceBarriers(ID3D12GraphicsCommandList* commandList, std::uint32_t descriptorSet);

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

        struct BindingHandleLocation
        {
            UINT heapIndex      :  1;
            UINT handleOffset   : 31;
        };

    private:

        void CreateDescriptorHeap(
            ID3D12Device*                   device,
            D3D12_DESCRIPTOR_HEAP_TYPE      heapType,
            UINT                            numDescriptors,
            ComPtr<ID3D12DescriptorHeap>&   outDescritporHeap
        );

        void AppendDescriptorHeapToArray(ID3D12DescriptorHeap* descriptorHeap);

        bool CreateShaderResourceView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc);
        bool CreateUnorderedAccessView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc);
        bool CreateConstantBufferView(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc);
        bool CreateSampler(ID3D12Device* device, D3D12_CPU_DESCRIPTOR_HANDLE cpuDescHandle, const ResourceViewDescriptor& desc);

        void ExchangeUAVResource(
            const D3D12DescriptorHandleLocation&    descriptorHandle,
            std::uint32_t                           descriptorSet,
            Resource&                               resource,
            std::uint32_t                           (&setRange)[2]
        );

        void EmplaceD3DUAVResource(
            const D3D12DescriptorHandleLocation&    descriptorHandle,
            std::uint32_t                           descriptorSet,
            ID3D12Resource*                         resource,
            std::uint32_t                           (&setRange)[2]
        );

        void UpdateBarriers(std::uint32_t descriptorSet);

        inline bool HasBarriers() const
        {
            return (barrierStride_ > 0);
        }

    private:

        ComPtr<ID3D12DescriptorHeap>                descriptorHeapResourceViews_;
        ComPtr<ID3D12DescriptorHeap>                descriptorHeapSamplers_;

        ID3D12DescriptorHeap*                       descriptorHeaps_[2]         = {};   // References to the ComPtr objects
        UINT                                        descriptorHandleStrides_[2] = {};
        UINT                                        descriptorSetStrides_[2]    = {};
        UINT                                        numDescriptorHeaps_         = 0;    // Sizes of descriptor heaps array
        UINT                                        numDescriptorSets_          = 0;    // Only used for 'GetNumDescriptorSets'

        SmallVector<D3D12DescriptorHandleLocation>  descriptorHandleMap_;

        std::vector<ID3D12Resource*>                uavResourceHeap_;                   // Heap of UAV resources that require a barrier
        UINT                                        uavResourceSetStride_       = 0;    // Number of (potential) UAV resources per descriptor set
        UINT                                        uavResourceIndexOffset_     = 0;    // Subtracted offset for 'D3D12DescriptorHandleLocation::index'
        std::vector<char>                           barriers_;                          // Packed buffer for dyanmic struct { UINT N; D3D12_RESOURCE_BARRIER[N]; }
        UINT                                        barrierStride_              = 0;

        bool                                        hasGraphicsDescriptors_     = false;
        bool                                        hasComputeDescriptors_      = false;

};


} // /namespace LLGL


#endif



// ================================================================================
