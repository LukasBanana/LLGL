/*
 * D3D12PipelineLayout.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12PipelineLayout.h"
#include "../D3DX12/d3dx12.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


struct D3DRootSignatureContext
{
    CD3DX12_ROOT_SIGNATURE_DESC             signatureDesc       = {};
    std::vector<CD3DX12_ROOT_PARAMETER>     rootParameters;
    std::vector<CD3DX12_DESCRIPTOR_RANGE>   descriptorRanges;
    std::size_t                             rangeOffset         = 0;
};

D3D12PipelineLayout::D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc)
{
    D3DRootSignatureContext signatureContext;
    BuildRootSignatureDesc(signatureContext, desc);
    CreateRootSignature(device, signatureContext.signatureDesc);
}


/*
 * ======= Private: =======
 */

void D3D12PipelineLayout::BuildRootSignatureDesc(
    D3DRootSignatureContext&        signatureContext,
    const PipelineLayoutDescriptor& layoutDesc)
{
    /* Build root parameter for each descriptor range type */
    signatureContext.descriptorRanges.reserve(layoutDesc.bindings.size());

    BuildRootParameter(signatureContext, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     layoutDesc, ResourceType::ConstantBuffer);
    BuildRootParameter(signatureContext, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     layoutDesc, ResourceType::Texture       );
    BuildRootParameter(signatureContext, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     layoutDesc, ResourceType::StorageBuffer );
    BuildRootParameter(signatureContext, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, layoutDesc, ResourceType::Sampler       );

    /* Get root signature flags */
    D3D12_ROOT_SIGNATURE_FLAGS signatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    BuildRootSignatureFlags(signatureFlags, layoutDesc);

    /* Build final root signature descriptor */
    if (!signatureContext.rootParameters.empty())
    {
        signatureContext.signatureDesc.Init(
            static_cast<UINT>(signatureContext.rootParameters.size()),
            signatureContext.rootParameters.data(),
            0,          // numStaticSamplers
            nullptr,    // pStaticSamplers
            signatureFlags
        );
    }
    else
        signatureContext.signatureDesc.Init(0, nullptr, 0, nullptr, signatureFlags);

}

static bool AreRangeTypesCompatible(D3D12_DESCRIPTOR_RANGE_TYPE lhs, D3D12_DESCRIPTOR_RANGE_TYPE rhs)
{
    /*
    Samplers are not allowed in the same descriptor table as CBVs, SRVs, and UAVs.
    see https://msdn.microsoft.com/en-us/library/windows/desktop/dn859382(v=vs.85).aspx
    */
    return
    (
        ( lhs == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER && rhs == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER ) ||
        ( lhs != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER && rhs != D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER )
    );
}

void D3D12PipelineLayout::BuildRootParameter(
    D3DRootSignatureContext&        signatureContext,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const PipelineLayoutDescriptor& layoutDesc,
    const ResourceType              resourceType)
{
    auto prevRangeCount = signatureContext.descriptorRanges.size();

    /*
    Add all binding points for current descriptor range type.
    NOTE: Samplers are not allowed in the same descriptor table as CBVs, SRVs, and UAVs
    see https://msdn.microsoft.com/en-us/library/windows/desktop/dn859382(v=vs.85).aspx
    */
    for (const auto& binding : layoutDesc.bindings)
    {
        if (binding.type == resourceType)
        {
            /* Append descriptor range */
            CD3DX12_DESCRIPTOR_RANGE descRange;
            descRange.Init(descRangeType, 1, binding.slot);
            signatureContext.descriptorRanges.push_back(descRange);
        }
    }

    /* Append root parameter (if any ranges are used) */
    if (signatureContext.descriptorRanges.size() > prevRangeCount)
    {
        /* Check if new descriptor ranges can be appended to previous root parameter */
        if ( !signatureContext.rootParameters.empty() &&
             signatureContext.rangeOffset < signatureContext.descriptorRanges.size() &&
             AreRangeTypesCompatible(signatureContext.descriptorRanges[signatureContext.rangeOffset].RangeType, descRangeType))
        {
            /* Re-initialize root parameter as descriptor table with new descriptor ranges */
            auto& rootParam = signatureContext.rootParameters.back();
            {
                auto numRanges = static_cast<UINT>(signatureContext.descriptorRanges.size() - signatureContext.rangeOffset);
                rootParam.InitAsDescriptorTable(numRanges, &signatureContext.descriptorRanges[signatureContext.rangeOffset]);
            }
        }
        else
        {
            /* Add new root parameter */
            CD3DX12_ROOT_PARAMETER rootParam;
            {
                auto numRanges = static_cast<UINT>(signatureContext.descriptorRanges.size() - prevRangeCount);
                rootParam.InitAsDescriptorTable(numRanges, &signatureContext.descriptorRanges[prevRangeCount]);
            }
            signatureContext.rootParameters.push_back(rootParam);

            /* Store offset to first descriptor range of the new root parameter */
            signatureContext.rangeOffset = prevRangeCount;
        }
    }
}

//TODO: properly enable D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT and D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT
void D3D12PipelineLayout::BuildRootSignatureFlags(
    D3D12_ROOT_SIGNATURE_FLAGS&     signatureFlags,
    const PipelineLayoutDescriptor& layoutDesc)
{
    signatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    /* Determine which shader stages are not used for any binding points */
    long stageFlags = 0;
    for (const auto& binding : layoutDesc.bindings)
        stageFlags |= binding.stageFlags;

    /* Deny access to root signature for shader stages that are not affected by any binding point */
    if ((stageFlags & StageFlags::VertexStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
    if ((stageFlags & StageFlags::TessControlStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
    if ((stageFlags & StageFlags::TessEvaluationStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
    if ((stageFlags & StageFlags::GeometryStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
    if ((stageFlags & StageFlags::FragmentStage) == 0)
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
}

void D3D12PipelineLayout::CreateRootSignature(ID3D12Device* device, const D3D12_ROOT_SIGNATURE_DESC& signatureDesc)
{
    /* Create serialized root signature */
    auto signature = SerializeRootSignature(signatureDesc, D3D_ROOT_SIGNATURE_VERSION_1);

    /* Create actual root signature */
    auto hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(rootSignature_.ReleaseAndGetAddressOf())
    );

    DXThrowIfFailed(hr, "failed to create D3D12 root signature");
}

ComPtr<ID3DBlob> D3D12PipelineLayout::SerializeRootSignature(
    const D3D12_ROOT_SIGNATURE_DESC& signatureDesc,
    const D3D_ROOT_SIGNATURE_VERSION signatureversion)
{
    ComPtr<ID3DBlob> signature, error;

    auto hr = D3D12SerializeRootSignature(
        &signatureDesc,
        signatureversion,
        signature.ReleaseAndGetAddressOf(),
        error.ReleaseAndGetAddressOf()
    );

    if (FAILED(hr))
    {
        if (error)
        {
            auto errorStr = DXGetBlobString(error.Get());
            throw std::runtime_error("failed to serialize D3D12 root signature: " + errorStr);
        }
        else
            DXThrowIfFailed(hr, "failed to serialize D3D12 root signature");
    }

    return signature;
}


} // /namespace LLGL



// ================================================================================
