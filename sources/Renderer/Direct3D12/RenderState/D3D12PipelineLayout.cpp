/*
 * D3D12PipelineLayout.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12PipelineLayout.h"
#include "../Shader/D3D12RootSignature.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include <LLGL/Misc/ForRange.h>


namespace LLGL
{


D3D12PipelineLayout::D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc)
{
    /* Convolute all stage flags */
    for (const auto& binding : desc.bindings)
        convolutedStageFlags_ |= binding.stageFlags;

    /* Create root signature */
    descriptorHandleMap_.resize(desc.bindings.size());
    CreateRootSignature(device, desc);
}

void D3D12PipelineLayout::SetName(const char* name)
{
    D3D12SetObjectName(rootSignature_.Get(), name);
}

std::uint32_t D3D12PipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(descriptorHandleMap_.size());
}

static D3D12_ROOT_SIGNATURE_FLAGS GetD3DRootSignatureFlags(long convolutedStageFlags)
{
    D3D12_ROOT_SIGNATURE_FLAGS signatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    /* Always allow vertex input layout and stream output */
    signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;

    /* Deny access to root signature for shader stages that are not affected by any binding point */
    if ((convolutedStageFlags & StageFlags::VertexStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    if ((convolutedStageFlags & StageFlags::TessControlStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if ((convolutedStageFlags & StageFlags::TessEvaluationStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if ((convolutedStageFlags & StageFlags::GeometryStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    if ((convolutedStageFlags & StageFlags::FragmentStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    return signatureFlags;
}

void D3D12PipelineLayout::CreateRootSignature(ID3D12Device* device, const PipelineLayoutDescriptor& desc)
{
    D3D12RootSignatureBuilder rootSignature;
    rootSignature.Reset(static_cast<UINT>(desc.bindings.size()), 0);

    /* Build root parameter for each descriptor range type */
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     desc, ResourceType::Buffer,  BindFlags::ConstantBuffer, descriptorHeapLayout_.numBufferCBV );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Buffer,  BindFlags::Sampled,        descriptorHeapLayout_.numBufferSRV );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Texture, BindFlags::Sampled,        descriptorHeapLayout_.numTextureSRV);
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Buffer,  BindFlags::Storage,        descriptorHeapLayout_.numBufferUAV );
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Texture, BindFlags::Storage,        descriptorHeapLayout_.numTextureUAV);
    BuildRootParameter(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, desc, ResourceType::Sampler, 0,                         descriptorHeapLayout_.numSamplers  );

    /* Build final root signature descriptor */
    rootSignature_ = rootSignature.Finalize(device, GetD3DRootSignatureFlags(GetConvolutedStageFlags()), &serializedBlob_);
}

void D3D12PipelineLayout::ReleaseRootSignature()
{
    rootSignature_.Reset();
}


/*
 * ======= Private: =======
 */

void D3D12PipelineLayout::BuildRootParameter(
    D3D12RootSignatureBuilder&      rootSignature,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const PipelineLayoutDescriptor& layoutDesc,
    const ResourceType              resourceType,
    long                            bindFlags,
    UINT&                           numResourceViews)
{
    for_range(i, layoutDesc.bindings.size())
    {
        const auto& binding = layoutDesc.bindings[i];
        if (binding.type == resourceType && (bindFlags == 0 || (binding.bindFlags & bindFlags) != 0))
        {
            if (auto rootParam = rootSignature.FindCompatibleRootParameter(descRangeType))
            {
                /* Append descriptor range to previous root parameter */
                rootParam->AppendDescriptorTableRange(descRangeType, binding.slot, binding.arraySize);
            }
            else
            {
                /* Create new root parameter and append descriptor range */
                rootParam = rootSignature.AppendRootParameter();
                rootParam->InitAsDescriptorTable(static_cast<UINT>(layoutDesc.bindings.size()));
                rootParam->AppendDescriptorTableRange(descRangeType, binding.slot, binding.arraySize);
            }

            /* Cache binding flags in the same order root parameters are build */
            auto& mapping = descriptorHandleMap_[i];
            {
                if (descRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
                {
                    mapping.heap    = 1;
                    mapping.index   = descriptorHeapLayout_.SumSamplers();
                }
                else
                {
                    mapping.heap    = 0;
                    mapping.index   = descriptorHeapLayout_.SumResourceViews();
                }
                mapping.type = descRangeType;
            }

            /* Increment number of resource views for output parameter to build root parameter layout */
            ++numResourceViews;
        }
    }
}


} // /namespace LLGL



// ================================================================================
