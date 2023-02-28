/*
 * D3D12PipelineLayout.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_PIPELINE_LAYOUT_H
#define LLGL_D3D12_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Container/SmallVector.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>


namespace LLGL
{


class D3D12RootSignature;

// Resource view descriptor to root parameter table mapping structure (for D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE).
struct D3D12DescriptorHeapLocation
{
    D3D12_DESCRIPTOR_RANGE_TYPE type;
    UINT                        heap  :  1; // Descriptor heap index (0 = SBC/SRV/UAV, 1 = Sampler)
    UINT                        index : 31; // Descriptor index within its descriptor heap
};

// Resource descriptor to root parameter/ descriptor table range mapping structure.
struct D3D12DescriptorLocation
{
    D3D12_ROOT_PARAMETER_TYPE   type;
    UINT                        index; // Root parameter index
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

        void SetName(const char* name) override;

        std::uint32_t GetNumHeapBindings() const override;
        std::uint32_t GetNumBindings() const override;
        std::uint32_t GetNumStaticSamplers() const override;
        std::uint32_t GetNumUniforms() const override;

    public:

        D3D12PipelineLayout();
        D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc);

        void CreateRootSignature(ID3D12Device* device, const PipelineLayoutDescriptor& desc);
        void ReleaseRootSignature();

        // Returns the layout of the set of descriptor heaps.
        D3D12DescriptorHeapSetLayout GetDescriptorHeapSetLayout() const;

        // Returns the native ID3D12RootSignature object.
        inline ID3D12RootSignature* GetRootSignature() const
        {
            return rootSignature_.Get();
        }

        // Returns the native ID3D12RootSignature object as ComPtr.
        inline const ComPtr<ID3D12RootSignature>& GetSharedRootSignature() const
        {
            return rootSignature_;
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
        inline const SmallVector<D3D12DescriptorHeapLocation>& GetDescriptorHeapMap() const
        {
            return descriptorHeapMap_;
        }

        // Returns the root signature layout with all resource counters for the root parameters.
        inline const D3D12RootSignatureLayout& GetDescriptorLayout() const
        {
            return descriptorLayout_;
        }

        // Returns the binding to dynamic descriptor table range map. Index for 'bindings' -> location in descriptor table range.
        inline const SmallVector<D3D12DescriptorHeapLocation>& GetDescriptorMap() const
        {
            return descriptorMap_;
        }

        // Returns the binding to root parameter map. Index for 'bindings' -> location of root parameter.
        inline const SmallVector<D3D12DescriptorLocation>& GetRootParameterMap() const
        {
            return rootParameterMap_;
        }

        // Returns the root parameter indices for this root signature.
        inline const D3D12RootParameterIndices& GetRootParameterIndices() const
        {
            return rootParameterIndices_;
        }

    private:

        void BuildHeapRootParameterTables(
            D3D12RootSignature&             rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const PipelineLayoutDescriptor& layoutDesc,
            const ResourceType              resourceType,
            long                            bindFlags,
            UINT&                           outCounter
        );

        void BuildHeapRootParameterTableEntry(
            D3D12RootSignature&             rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const BindingDescriptor&        bindingDesc,
            UINT                            maxNumDescriptorRanges,
            D3D12DescriptorHeapLocation&    outLocation
        );

        void BuildRootParameterTables(
            D3D12RootSignature&             rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const PipelineLayoutDescriptor& layoutDesc,
            const ResourceType              resourceType,
            long                            bindFlags,
            UINT&                           outCounter
        );

        void BuildRootParameterTableEntry(
            D3D12RootSignature&             rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const BindingDescriptor&        bindingDesc,
            UINT                            maxNumDescriptorRanges,
            D3D12DescriptorHeapLocation&    outLocation
        );

        void BuildRootParameters(
            D3D12RootSignature&             rootSignature,
            D3D12_ROOT_PARAMETER_TYPE       rootParamType,
            const PipelineLayoutDescriptor& layoutDesc,
            const ResourceType              resourceType,
            long                            bindFlags
        );

        void BuildRootParameter(
            D3D12RootSignature&             rootSignature,
            D3D12_ROOT_PARAMETER_TYPE       rootParamType,
            const BindingDescriptor&        bindingDesc,
            D3D12DescriptorLocation&        outLocation
        );

        void BuildStaticSamplers(
            D3D12RootSignature&             rootSignature,
            const PipelineLayoutDescriptor& layoutDesc,
            UINT&                           outCounter
        );

    private:

        ComPtr<ID3D12RootSignature>                 rootSignature_;
        ComPtr<ID3DBlob>                            serializedBlob_;

        D3D12RootSignatureLayout                    descriptorHeapLayout_;
        SmallVector<D3D12DescriptorHeapLocation>    descriptorHeapMap_;
        D3D12RootSignatureLayout                    descriptorLayout_;
        SmallVector<D3D12DescriptorHeapLocation>    descriptorMap_;
        SmallVector<D3D12DescriptorLocation>        rootParameterMap_;

        UINT                                        numStaticSamplers_      = 0;
        UINT                                        numUniforms_            = 0;
        D3D12RootParameterIndices                   rootParameterIndices_;
        long                                        convolutedStageFlags_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
