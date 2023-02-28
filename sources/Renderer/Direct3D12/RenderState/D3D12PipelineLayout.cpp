/*
 * D3D12PipelineLayout.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12PipelineLayout.h"
#include "../Shader/D3D12RootSignature.h"
#include "../Texture/D3D12Sampler.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include <LLGL/Misc/ForRange.h>


namespace LLGL
{


D3D12PipelineLayout::D3D12PipelineLayout()
{
    rootParameterIndices_.rootParamDescriptorHeaps[0]   = D3D12RootParameterIndices::invalidIndex;
    rootParameterIndices_.rootParamDescriptorHeaps[1]   = D3D12RootParameterIndices::invalidIndex;
    rootParameterIndices_.rootParamDescriptors[0]       = D3D12RootParameterIndices::invalidIndex;
    rootParameterIndices_.rootParamDescriptors[1]       = D3D12RootParameterIndices::invalidIndex;
}

D3D12PipelineLayout::D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc) :
    D3D12PipelineLayout {}
{
    CreateRootSignature(device, desc);
}

void D3D12PipelineLayout::SetName(const char* name)
{
    D3D12SetObjectName(rootSignature_.Get(), name);
}

std::uint32_t D3D12PipelineLayout::GetNumHeapBindings() const
{
    return static_cast<std::uint32_t>(descriptorHeapMap_.size());
}

std::uint32_t D3D12PipelineLayout::GetNumBindings() const
{
    return static_cast<std::uint32_t>(descriptorMap_.size());
}

std::uint32_t D3D12PipelineLayout::GetNumStaticSamplers() const
{
    return numStaticSamplers_;
}

std::uint32_t D3D12PipelineLayout::GetNumUniforms() const
{
    return numUniforms_;
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

template <typename TBindings>
void ConvoluteStageFlags(long& convolutedStageFlags, const TBindings& bindings)
{
    for (const auto& binding : bindings)
        convolutedStageFlags |= binding.stageFlags;
}

// Convolutes all stage flags from the layout binding points
static long ConvoluteLayoutStageFlags(const PipelineLayoutDescriptor& desc)
{
    long convolutedStageFlags = 0;
    ConvoluteStageFlags(convolutedStageFlags, desc.heapBindings);
    ConvoluteStageFlags(convolutedStageFlags, desc.bindings);
    ConvoluteStageFlags(convolutedStageFlags, desc.staticSamplers);
    return convolutedStageFlags;
}

void D3D12PipelineLayout::CreateRootSignature(ID3D12Device* device, const PipelineLayoutDescriptor& desc)
{
    UINT descriptorTableCounter = 0;

    /* Convolute all stage flags */
    convolutedStageFlags_ = ConvoluteLayoutStageFlags(desc);

    /* Create root signature */
    D3D12RootSignature rootSignature;
    rootSignature.Reset(static_cast<UINT>(desc.bindings.size()), 0);

    /* Build root parameter table for each descriptor range type */
    descriptorHeapMap_.resize(desc.heapBindings.size());
    BuildHeapRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     desc, ResourceType::Buffer,  BindFlags::ConstantBuffer, descriptorHeapLayout_.numBufferCBV );
    BuildHeapRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Buffer,  BindFlags::Sampled,        descriptorHeapLayout_.numBufferSRV );
    BuildHeapRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Texture, BindFlags::Sampled,        descriptorHeapLayout_.numTextureSRV);
    BuildHeapRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Buffer,  BindFlags::Storage,        descriptorHeapLayout_.numBufferUAV );
    BuildHeapRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Texture, BindFlags::Storage,        descriptorHeapLayout_.numTextureUAV);
    BuildHeapRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, desc, ResourceType::Sampler, 0,                         descriptorHeapLayout_.numSamplers  );

    /* Build root parameter for each descriptor range type */
    descriptorMap_.resize(desc.bindings.size());
    BuildRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_CBV,     desc, ResourceType::Buffer,  BindFlags::ConstantBuffer, descriptorLayout_.numBufferCBV);
    BuildRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Buffer,  BindFlags::Sampled,        descriptorLayout_.numBufferSRV);
    BuildRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SRV,     desc, ResourceType::Texture, BindFlags::Sampled,        descriptorLayout_.numTextureSRV);
    BuildRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Buffer,  BindFlags::Storage,        descriptorLayout_.numBufferUAV);
    BuildRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_UAV,     desc, ResourceType::Texture, BindFlags::Storage,        descriptorLayout_.numTextureUAV);
    BuildRootParameterTables(rootSignature, D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, desc, ResourceType::Sampler, 0,                         descriptorLayout_.numSamplers);

    /* Build root parameter for each standalone descriptor */
    rootParameterMap_.resize(desc.bindings.size());
    BuildRootParameters(rootSignature, D3D12_ROOT_PARAMETER_TYPE_CBV, desc, ResourceType::Buffer, BindFlags::ConstantBuffer);
    BuildRootParameters(rootSignature, D3D12_ROOT_PARAMETER_TYPE_SRV, desc, ResourceType::Buffer, BindFlags::Sampled);
    BuildRootParameters(rootSignature, D3D12_ROOT_PARAMETER_TYPE_UAV, desc, ResourceType::Buffer, BindFlags::Storage);

    /* Build static samplers */
    BuildStaticSamplers(rootSignature, desc, numStaticSamplers_);

    /* Build uniforms */
    numUniforms_ = static_cast<UINT>(desc.uniforms.size());

    /* Build final root signature descriptor */
    rootSignature_ = rootSignature.Finalize(device, GetD3DRootSignatureFlags(GetConvolutedStageFlags()), &serializedBlob_);
}

void D3D12PipelineLayout::ReleaseRootSignature()
{
    rootSignature_.Reset();
}

D3D12DescriptorHeapSetLayout D3D12PipelineLayout::GetDescriptorHeapSetLayout() const
{
    D3D12DescriptorHeapSetLayout layout;

    layout.numHeapResourceViews = descriptorHeapLayout_.SumResourceViews();
    layout.numHeapSamplers      = descriptorHeapLayout_.SumSamplers();
    layout.numResourceViews     = descriptorLayout_.SumResourceViews();
    layout.numSamplers          = descriptorLayout_.SumSamplers();

    return layout;
}


/*
 * ======= Private: =======
 */

static bool IsFilteredBinding(const BindingDescriptor& bindingDesc, const ResourceType resourceType, long bindFlags)
{
    return (bindingDesc.type == resourceType && (bindFlags == 0 || (bindingDesc.bindFlags & bindFlags) != 0));
}

void D3D12PipelineLayout::BuildHeapRootParameterTables(
    D3D12RootSignature&             rootSignature,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const PipelineLayoutDescriptor& layoutDesc,
    const ResourceType              resourceType,
    long                            bindFlags,
    UINT&                           outCounter)
{
    for_range(i, layoutDesc.heapBindings.size())
    {
        const auto& binding = layoutDesc.heapBindings[i];
        if (IsFilteredBinding(binding, resourceType, bindFlags))
        {
            /* Build root parameter table entry for currently seelcted resource binding */
            BuildHeapRootParameterTableEntry(
                /*rootSignature:*/          rootSignature,
                /*descRangeType:*/          descRangeType,
                /*bindingDesc:*/            binding,
                /*maxNumDescriptorRanges:*/ static_cast<UINT>(layoutDesc.heapBindings.size()),
                /*outLocation:*/            descriptorHeapMap_[i]
            );

            /* Increment number of resource views for output parameter to build root parameter layout */
            ++outCounter;
        }
    }
}

static int GetDescriptorTypeShift(D3D12_DESCRIPTOR_RANGE_TYPE type)
{
    return (type == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER ? 1 : 0);
}

void D3D12PipelineLayout::BuildHeapRootParameterTableEntry(
    D3D12RootSignature&             rootSignature,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const BindingDescriptor&        bindingDesc,
    UINT                            maxNumDescriptorRanges,
    D3D12DescriptorHeapLocation&    outLocation)
{
    if (auto rootParam = rootSignature.FindCompatibleRootParameter(descRangeType))
    {
        /* Append descriptor range to previous root parameter */
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, bindingDesc.arraySize);
    }
    else
    {
        /* Create new root parameter and append descriptor range */
        UINT rootParamIndex = 0;
        rootParam = rootSignature.AppendRootParameter(&rootParamIndex);
        rootParam->InitAsDescriptorTable(maxNumDescriptorRanges);
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, bindingDesc.arraySize);

        /* Store root parameter index */
        rootParameterIndices_.rootParamDescriptorHeaps[GetDescriptorTypeShift(descRangeType)] = rootParamIndex;
    }

    /* Cache binding flags in the same order root parameters are build */
    descriptorHeapLayout_.GetDescriptorLocation(descRangeType, outLocation);
}

static bool CanResourceHaveRootParameter(const ResourceType resourceType, long bindFlags)
{
    return (resourceType == ResourceType::Buffer);
}

void D3D12PipelineLayout::BuildRootParameterTables(
    D3D12RootSignature&             rootSignature,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const PipelineLayoutDescriptor& layoutDesc,
    const ResourceType              resourceType,
    long                            bindFlags,
    UINT&                           outCounter)
{
    for_range(i, layoutDesc.bindings.size())
    {
        const auto& binding = layoutDesc.bindings[i];
        if (IsFilteredBinding(binding, resourceType, bindFlags))
        {
            /* If resource binding cannot have its own root parameter, it must be put into a descriptor table */
            if (!CanResourceHaveRootParameter(resourceType, bindFlags))
            {
                BuildRootParameterTableEntry(
                    /*rootSignature:*/  rootSignature,
                    /*rootParamType:*/  descRangeType,
                    /*layoutDesc:*/     binding,
                    /*resourceType:*/   static_cast<UINT>(layoutDesc.bindings.size()),
                    /*bindFlags:*/      descriptorMap_[i]
                );

                /* Increment number of resources for output parameter to build root parameter layout */
                ++outCounter;
            }
        }
    }
}

// Returns the index *after* the last root parameter for heap resources, i.e. 'rootParamHeapResourceViews' and 'rootParamHeapSamplers'.
static UINT GetRootParameterIndexAfterHeapResources(const D3D12RootParameterIndices& indices)
{
    return std::max(
        (indices.rootParamDescriptorHeaps[0] == D3D12RootParameterIndices::invalidIndex ? 0u : indices.rootParamDescriptorHeaps[0]),
        (indices.rootParamDescriptorHeaps[1] == D3D12RootParameterIndices::invalidIndex ? 0u : indices.rootParamDescriptorHeaps[1])
    ) + 1u;
}

void D3D12PipelineLayout::BuildRootParameterTableEntry(
    D3D12RootSignature&             rootSignature,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const BindingDescriptor&        bindingDesc,
    UINT                            maxNumDescriptorRanges,
    D3D12DescriptorHeapLocation&    outLocation)
{
    /* Find compatible root parameter after root parameter for heap resources */
    const auto rootParamOffset = GetRootParameterIndexAfterHeapResources(rootParameterIndices_);
    if (auto rootParam = rootSignature.FindCompatibleRootParameter(descRangeType, rootParamOffset))
    {
        /* Append descriptor range to previous root parameter */
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, bindingDesc.arraySize);
    }
    else
    {
        /* Create new root parameter and append descriptor range */
        UINT rootParamIndex = 0;
        rootParam = rootSignature.AppendRootParameter(&rootParamIndex);
        rootParam->InitAsDescriptorTable(maxNumDescriptorRanges);
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, bindingDesc.arraySize);

        /* Store root parameter index */
        rootParameterIndices_.rootParamDescriptors[GetDescriptorTypeShift(descRangeType)] = rootParamIndex;
    }

    /* Cache binding flags in the same order root parameters are build */
    descriptorLayout_.GetDescriptorLocation(descRangeType, outLocation);
}

void D3D12PipelineLayout::BuildRootParameters(
    D3D12RootSignature&             rootSignature,
    D3D12_ROOT_PARAMETER_TYPE       rootParamType,
    const PipelineLayoutDescriptor& layoutDesc,
    const ResourceType              resourceType,
    long                            bindFlags)
{
    for_range(i, layoutDesc.bindings.size())
    {
        const auto& binding = layoutDesc.bindings[i];
        if (IsFilteredBinding(binding, resourceType, bindFlags))
        {
            /* If resource binding cannot have its own root parameter, it must be put into a descriptor table */
            if (CanResourceHaveRootParameter(resourceType, bindFlags))
            {
                BuildRootParameter(
                    /*rootSignature:*/  rootSignature,
                    /*rootParamType:*/  rootParamType,
                    /*bindingDesc:*/    binding,
                    /*outLocation:*/    rootParameterMap_[i]
                );
            }
        }
    }
}

void D3D12PipelineLayout::BuildRootParameter(
    D3D12RootSignature&             rootSignature,
    D3D12_ROOT_PARAMETER_TYPE       rootParamType,
    const BindingDescriptor&        bindingDesc,
    D3D12DescriptorLocation&        outLocation)
{
    /* Create new root parameter and append descriptor range */
    UINT rootParamIndex = 0;
    auto* rootParam = rootSignature.AppendRootParameter(&rootParamIndex);
    rootParam->InitAsDescriptor(rootParamType, bindingDesc.slot);//, D3D12RootParameter::FindSuitableVisibility(binding.stageFlags));

    /* Cache binding flags in the same order root parameters are build */
    outLocation.type    = rootParamType;
    outLocation.index   = rootParamIndex;
}

void D3D12PipelineLayout::BuildStaticSamplers(
    D3D12RootSignature&             rootSignature,
    const PipelineLayoutDescriptor& layoutDesc,
    UINT&                           numStaticSamplers)
{
    for (const auto& staticSamplerDesc : layoutDesc.staticSamplers)
    {
        auto nativeStaticSampler = rootSignature.AppendStaticSampler();
        D3D12Sampler::ConvertDesc(*nativeStaticSampler, staticSamplerDesc);
    }
    numStaticSamplers = static_cast<UINT>(layoutDesc.staticSamplers.size());
}


/*
 * D3D12RootSignatureLayout struct
 */

void D3D12RootSignatureLayout::GetDescriptorLocation(D3D12_DESCRIPTOR_RANGE_TYPE descRangeType, D3D12DescriptorHeapLocation& outLocation) const
{
    outLocation.type = descRangeType;
    if (descRangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
    {
        outLocation.heap    = 1;
        outLocation.index   = SumSamplers();
    }
    else
    {
        outLocation.heap    = 0;
        outLocation.index   = SumResourceViews();
    }
}


} // /namespace LLGL



// ================================================================================
