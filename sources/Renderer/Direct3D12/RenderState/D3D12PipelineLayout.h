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
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


class D3D12RootSignatureBuilder;

struct D3D12RootParameterLayout
{
    UINT numBufferCBV   = 0;
    UINT numBufferSRV   = 0;
    UINT numTextureSRV  = 0;
    UINT numBufferUAV   = 0;
    UINT numTextureUAV  = 0;
    UINT numSamplers    = 0;
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

        // Returns the binding flags for the specified resource index; this is not necessarily the binding slot!
        long GetBindFlagsByIndex(std::size_t idx) const;

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

        // Returns the bitwise OR combined stage flags of all resource view descriptors.
        inline long GetCombinedStageFlags() const
        {
            return combinedStageFlags_;
        }

        inline const D3D12RootParameterLayout& GetRootParameterLayout() const
        {
            return rootParameterLayout_;
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

        ComPtr<ID3D12RootSignature> rootSignature_;
        ComPtr<ID3DBlob>            serializedBlob_;
        std::vector<long>           bindFlags_;
        long                        combinedStageFlags_     = 0;
        D3D12RootParameterLayout    rootParameterLayout_;

};


} // /namespace LLGL


#endif



// ================================================================================
