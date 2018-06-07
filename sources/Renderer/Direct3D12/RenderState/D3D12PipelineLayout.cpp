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


D3D12PipelineLayout::D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc)
{
    CD3DX12_ROOT_SIGNATURE_DESC signatureDesc;
    BuildRootSignatureDesc(signatureDesc, desc);
    CreateRootSignature(device, signatureDesc);
}


/*
 * ======= Private: =======
 */

void D3D12PipelineLayout::BuildRootSignatureDesc(
    CD3DX12_ROOT_SIGNATURE_DESC&    signatureDesc,
    const PipelineLayoutDescriptor& layoutDesc)
{
    /* Build root parameter for each descriptor range type */
    std::vector<CD3DX12_ROOT_PARAMETER> rootParameters;
    std::vector<CD3DX12_DESCRIPTOR_RANGE> descriptorRanges;

    descriptorRanges.reserve(layoutDesc.bindings.size());

    BuildRootParameter(rootParameters, descriptorRanges, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     layoutDesc, ResourceType::Texture       );
    BuildRootParameter(rootParameters, descriptorRanges, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     layoutDesc, ResourceType::StorageBuffer );
    BuildRootParameter(rootParameters, descriptorRanges, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     layoutDesc, ResourceType::ConstantBuffer);
    BuildRootParameter(rootParameters, descriptorRanges, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, layoutDesc, ResourceType::Sampler       );

    /* Get root signature flags */
    D3D12_ROOT_SIGNATURE_FLAGS signatureFlags = D3D12_ROOT_SIGNATURE_FLAG_NONE;
    BuildRootSignatureFlags(signatureFlags, layoutDesc);

    /* Build final root signature descriptor */
    if (!rootParameters.empty())
    {
        signatureDesc.Init(
            static_cast<UINT>(rootParameters.size()),
            rootParameters.data(),
            0,          // numStaticSamplers
            nullptr,    // pStaticSamplers
            signatureFlags
        );
    }
    else
        signatureDesc.Init(0, nullptr, 0, nullptr, signatureFlags);

}

void D3D12PipelineLayout::BuildRootParameter(
    std::vector<CD3DX12_ROOT_PARAMETER>&    rootParameters,
    std::vector<CD3DX12_DESCRIPTOR_RANGE>   descriptorRanges,
    D3D12_DESCRIPTOR_RANGE_TYPE             descRangeType,
    const PipelineLayoutDescriptor&         layoutDesc,
    const ResourceType                      resourceType)
{
    auto firstDescRange = descriptorRanges.size();

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
            descriptorRanges.push_back(descRange);
        }
    }

    /* Append root parameter (if any ranges are used) */
    if (descriptorRanges.size() > firstDescRange)
    {
        CD3DX12_ROOT_PARAMETER rootParam;
        {
            auto numRanges = static_cast<UINT>(descriptorRanges.size() - firstDescRange);
            rootParam.InitAsDescriptorTable(numRanges, &descriptorRanges[firstDescRange]);
        }
        rootParameters.push_back(rootParam);
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
