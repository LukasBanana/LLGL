/*
 * D3D12PipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_PIPELINE_LAYOUT_H
#define LLGL_D3D12_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/SmallVector.h>
#include <LLGL/Container/ArrayView.h>
#include "../Shader/D3D12RootSignature.h"
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <memory>
#include <map>


namespace LLGL
{


class D3D12Shader;
class D3D12RootSignature;

// Resource view descriptor to root parameter table mapping structure (for D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE).
struct D3D12DescriptorHeapLocation
{
    D3D12_DESCRIPTOR_RANGE_TYPE type;
    UINT                        heap  :  1; // Descriptor heap index (0 = SBC/SRV/UAV, 1 = Sampler)
    UINT                        index : 31; // Descriptor index within its descriptor heap
    D3D12_RESOURCE_STATES       state;      // Target resource state
};

// Resource descriptor to root parameter/ descriptor table range mapping structure.
struct D3D12DescriptorLocation
{
    D3D12_ROOT_PARAMETER_TYPE   type;
    UINT                        index; // Root parameter index
    D3D12_RESOURCE_STATES       state; // Target resource state
};

// Uniform to root constant mapping structure.
struct D3D12RootConstantLocation
{
    UINT index          : 16;   // Root parameter index
    UINT num32BitValues : 16;   // Number of 32-bit values this constant occupies.
    UINT wordOffset;            // Destination offset (in 32-bit values) within root constant buffer
};

// Container structure of all numbers of root parameters within a D3D12 root signature.
struct D3D12RootSignatureLayout
{
    UINT numBufferCBV   = 0;
    UINT numBufferSRV   = 0;
    UINT numTextureSRV  = 0;
    UINT numBufferUAV   = 0;
    UINT numTextureUAV  = 0;
    UINT numSamplers    = 0;

    // Returns the current descriptor location for the specified descriptor range type.
    void GetDescriptorLocation(D3D12_DESCRIPTOR_RANGE_TYPE descRangeType, D3D12DescriptorHeapLocation& outLocation) const;

    // Returns the sum of all CBV and SRV subresource views.
    inline UINT SumCBVsAndSRVs() const
    {
        return (numBufferCBV + numBufferSRV + numTextureSRV);
    }

    // Returns the sum of all UAV subresource views.
    inline UINT SumUAVs() const
    {
        return (numBufferUAV + numTextureUAV);
    }

    // Returns the sum of all subresource views.
    inline UINT SumResourceViews() const
    {
        return (SumCBVsAndSRVs() + SumUAVs());
    }

    // Returns the sub of all samplers.
    inline UINT SumSamplers() const
    {
        return (numSamplers);
    }
};

// Container structure of all descriptor table indices to their root parameters. Invalid indices have value 0xFF.
struct D3D12RootParameterIndices
{
    static constexpr UINT8 invalidIndex = 0xFF;

    UINT8 rootParamDescriptorHeaps[2];
    UINT8 rootParamDescriptors[2];
};

// Container structure used by D3D12CommandContext.
struct D3D12DescriptorHeapSetLayout
{
    UINT numHeapResourceViews   = 0;
    UINT numHeapSamplers        = 0;
    UINT numResourceViews       = 0;
    UINT numSamplers            = 0;
};

class D3D12PipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D12PipelineLayout(long barrierFlags = 0);
        D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc);

        void CreateRootSignature(ID3D12Device* device, const PipelineLayoutDescriptor& desc);
        void ReleaseRootSignature();

        /*
        Creates a D3D12 root signature permutation that includes 32-bit root constants.
        These constants are derived from the uniform descriptors and the constant buffer reflection from the specified shaders.
        */
        ComPtr<ID3D12RootSignature> CreateRootSignatureWith32BitConstants(
            const ArrayView<D3D12Shader*>&          shaders,
            std::vector<D3D12RootConstantLocation>& outRootConstantMap
        ) const;

        // Returns the layout of the set of descriptor heaps.
        D3D12DescriptorHeapSetLayout GetDescriptorHeapSetLayout() const;

        // Returns the finalized native ID3D12RootSignature object as ComPtr.
        inline const ComPtr<ID3D12RootSignature>& GetFinalizedRootSignature() const
        {
            return finalizedRootSignature_;
        }

        // Returns the serialized blob of the root siganture.
        inline ID3DBlob* GetSerializedBlob() const
        {
            return serializedBlob_.Get();
        }

        // Returns the bitwise OR convoluted stage flags of all binding descriptors.
        inline long GetConvolutedStageFlags() const
        {
            return convolutedStageFlags_;
        }

        // Returns the root signature layout with all resource counters for a descriptor heap.
        inline const D3D12RootSignatureLayout& GetDescriptorHeapLayout() const
        {
            return descriptorHeapLayout_;
        }

        // Returns the heap binding to descriptor table range map. Index for 'heapBindings' -> location in descriptor table range.
        inline const SmallVector<D3D12DescriptorHeapLocation, 8>& GetDescriptorHeapMap() const
        {
            return descriptorHeapMap_;
        }

        // Returns the root signature layout with all resource counters for the root parameters.
        inline const D3D12RootSignatureLayout& GetDescriptorLayout() const
        {
            return descriptorLayout_;
        }

        // Returns the binding to dynamic descriptor table range map. Index for 'bindings' -> location in descriptor table range.
        inline const SmallVector<D3D12DescriptorHeapLocation, 8>& GetDescriptorMap() const
        {
            return descriptorMap_;
        }

        // Returns the binding to root parameter map. Index for 'bindings' -> location of root parameter.
        inline const SmallVector<D3D12DescriptorLocation, 8>& GetRootParameterMap() const
        {
            return rootParameterMap_;
        }

        // Returns the root parameter indices for this root signature.
        inline const D3D12RootParameterIndices& GetRootParameterIndices() const
        {
            return rootParameterIndices_;
        }

        // Returns true if this pipeline layout needs a permutation for root constants.
        inline bool NeedsRootConstantPermutation() const
        {
            return !uniforms_.empty();
        }

        // Returns the barrier flags this pipeline layout was created with. See PipelineLayoutDescriptor::barrierFlags.
        inline long GetBarrierFlags() const
        {
            return barrierFlags_;
        }

    private:

        void BuildRootSignature(
            D3D12RootSignature&             rootSignature,
            const PipelineLayoutDescriptor& desc
        );

        void BuildHeapRootParameterTables(
            D3D12RootSignature&                 rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE         descRangeType,
            const ArrayView<BindingDescriptor>& bindingDescs,
            const ResourceType                  resourceType,
            long                                bindFlags,
            UINT&                               outCounter
        );

        void BuildHeapRootParameterTableEntry(
            D3D12RootSignature&             rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const BindingDescriptor&        bindingDesc,
            UINT                            maxNumDescriptorRanges,
            D3D12DescriptorHeapLocation&    outLocation
        );

        void BuildRootParameterTables(
            D3D12RootSignature&                 rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE         descRangeType,
            const ArrayView<BindingDescriptor>& bindingDescs,
            const ResourceType                  resourceType,
            long                                bindFlags,
            UINT&                               outCounter
        );

        void BuildRootParameterTableEntry(
            D3D12RootSignature&             rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const BindingDescriptor&        bindingDesc,
            UINT                            maxNumDescriptorRanges,
            D3D12DescriptorHeapLocation&    outLocation
        );

        void BuildRootParameters(
            D3D12RootSignature&                 rootSignature,
            D3D12_ROOT_PARAMETER_TYPE           rootParamType,
            const ArrayView<BindingDescriptor>& bindingDescs,
            const ResourceType                  resourceType,
            long                                bindFlags
        );

        void BuildRootParameter(
            D3D12RootSignature&             rootSignature,
            D3D12_ROOT_PARAMETER_TYPE       rootParamType,
            const BindingDescriptor&        bindingDesc,
            D3D12DescriptorLocation&        outLocation
        );

        void BuildStaticSamplers(
            D3D12RootSignature&                         rootSignature,
            const ArrayView<StaticSamplerDescriptor>&   staticSamplerDescs,
            UINT&                                       outCounter
        );

    private:

        ID3D12Device*                               device_                 = nullptr;

        std::unique_ptr<D3D12RootSignature>         rootSignature_;
        ComPtr<ID3D12RootSignature>                 finalizedRootSignature_;
        ComPtr<ID3DBlob>                            serializedBlob_;

        D3D12RootSignatureLayout                    descriptorHeapLayout_;
        SmallVector<D3D12DescriptorHeapLocation, 8> descriptorHeapMap_;
        D3D12RootSignatureLayout                    descriptorLayout_;
        SmallVector<D3D12DescriptorHeapLocation, 8> descriptorMap_;
        SmallVector<D3D12DescriptorLocation, 8>     rootParameterMap_;

        UINT                                        numStaticSamplers_      = 0;
        D3D12RootParameterIndices                   rootParameterIndices_;
        long                                        convolutedStageFlags_   = 0;

        std::vector<UniformDescriptor>              uniforms_;

        long                                        barrierFlags_           = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
