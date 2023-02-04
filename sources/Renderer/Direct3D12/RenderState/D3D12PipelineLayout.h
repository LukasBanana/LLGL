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


class D3D12RootSignatureBuilder;

// Container structure of all numbers of root parameters within a D3D12 root signature.
struct D3D12DescriptorHeapLayout
{
    UINT numBufferCBV   = 0;
    UINT numBufferSRV   = 0;
    UINT numTextureSRV  = 0;
    UINT numBufferUAV   = 0;
    UINT numTextureUAV  = 0;
    UINT numSamplers    = 0;

    // Returns the sum of all subresource views.
    inline UINT SumResourceViews() const
    {
        return (numBufferCBV + numBufferSRV + numTextureSRV + numBufferUAV + numTextureUAV);
    }

    // Returns the sub of all samplers.
    inline UINT SumSamplers() const
    {
        return (numSamplers);
    }
};

// Resource view descriptor to root parameter mapping structure.
struct D3D12DescriptorHandleLocation
{
    UINT                        heap  :  1; // Descriptor heap index (0 = SBC/SRV/UAV, 1 = Sampler)
    UINT                        index : 31; // Descriptor index within its descriptor heap
    D3D12_DESCRIPTOR_RANGE_TYPE type;
};

class D3D12PipelineLayout final : public PipelineLayout
{

    public:

        void SetName(const char* name) override;

        std::uint32_t GetNumBindings() const override;

    public:

        D3D12PipelineLayout() = default;
        D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc);

        void CreateRootSignature(ID3D12Device* device, const PipelineLayoutDescriptor& desc);
        void ReleaseRootSignature();

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

        // Returns the root parameter layout with all resource counters.
        inline const D3D12DescriptorHeapLayout& GetDescriptorHeapLayout() const
        {
            return descriptorHeapLayout_;
        }

        // Returns the binding to root-parameter map/
        inline const SmallVector<D3D12DescriptorHandleLocation>& GetDescriptorHandleMap() const
        {
            return descriptorHandleMap_;
        }

    private:

        void BuildRootParameter(
            D3D12RootSignatureBuilder&      rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const PipelineLayoutDescriptor& layoutDesc,
            const ResourceType              resourceType,
            long                            bindFlags,
            UINT&                           numResourceViews
        );

    private:

        ComPtr<ID3D12RootSignature>                 rootSignature_;
        ComPtr<ID3DBlob>                            serializedBlob_;
        D3D12DescriptorHeapLayout                   descriptorHeapLayout_;
        SmallVector<D3D12DescriptorHandleLocation>  descriptorHandleMap_;
        long                                        convolutedStageFlags_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
