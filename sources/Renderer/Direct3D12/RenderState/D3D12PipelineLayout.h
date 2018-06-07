/*
 * D3D12PipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_PIPELINE_LAYOUT_H
#define LLGL_D3D12_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include "../../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <vector>


struct CD3DX12_ROOT_SIGNATURE_DESC;
struct CD3DX12_ROOT_PARAMETER;
struct CD3DX12_DESCRIPTOR_RANGE;

namespace LLGL
{


class D3D12PipelineLayout : public PipelineLayout
{

    public:

        D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc);

        // Returns the native ID3D12RootSignature object.
        inline ID3D12RootSignature* GetNative() const
        {
            return rootSignature_.Get();
        }

    private:

        void BuildRootSignatureDesc(
            CD3DX12_ROOT_SIGNATURE_DESC&    signatureDesc,
            const PipelineLayoutDescriptor& layoutDesc
        );

        void BuildRootParameter(
            std::vector<CD3DX12_ROOT_PARAMETER>&    rootParameters,
            std::vector<CD3DX12_DESCRIPTOR_RANGE>   descriptorRanges,
            D3D12_DESCRIPTOR_RANGE_TYPE             descRangeType,
            const PipelineLayoutDescriptor&         layoutDesc,
            const ResourceType                      resourceType
        );

        void BuildRootSignatureFlags(
            D3D12_ROOT_SIGNATURE_FLAGS&     signatureFlags,
            const PipelineLayoutDescriptor& layoutDesc
        );

        void CreateRootSignature(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& signatureDesc);

        ComPtr<ID3DBlob> SerializeRootSignature(
            const D3D12_ROOT_SIGNATURE_DESC& signatureDesc,
            const D3D_ROOT_SIGNATURE_VERSION signatureversion
        );

        ComPtr<ID3D12RootSignature> rootSignature_;

};


} // /namespace LLGL


#endif



// ================================================================================
