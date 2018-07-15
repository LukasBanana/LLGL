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


namespace LLGL
{


class D3D12RootSignature;

class D3D12PipelineLayout final : public PipelineLayout
{

    public:

        D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc);

        // Returns the native ID3D12RootSignature object.
        inline ID3D12RootSignature* GetRootSignature() const
        {
            return rootSignature_.Get();
        }

    private:

        void BuildRootParameter(
            D3D12RootSignature&             rootSignature,
            D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
            const PipelineLayoutDescriptor& layoutDesc,
            const ResourceType              resourceType
        );

        void BuildRootSignatureFlags(
            D3D12_ROOT_SIGNATURE_FLAGS&     signatureFlags,
            const PipelineLayoutDescriptor& layoutDesc
        );

        ComPtr<ID3D12RootSignature> rootSignature_;

};


} // /namespace LLGL


#endif



// ================================================================================
