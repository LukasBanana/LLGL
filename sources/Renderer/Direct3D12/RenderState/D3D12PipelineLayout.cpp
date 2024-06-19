/*
 * D3D12PipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12PipelineLayout.h"
#include "../Shader/D3D12RootSignature.h"
#include "../Shader/D3D12Shader.h"
#include "../Texture/D3D12Sampler.h"
#include "../D3DX12/d3dx12.h"
#include "../D3D12ObjectUtils.h"
#include "../../DXCommon/DXCore.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <string>
#include <algorithm>


namespace LLGL
{


static bool HasPipelineLayoutDescBindlessHeap(const PipelineLayoutDescriptor& desc)
{
    return (desc.heapBindings.size() == 1 && desc.heapBindings.front().type == ResourceType::Undefined);
}

D3D12PipelineLayout::D3D12PipelineLayout(long barrierFlags, bool hasBindlessHeap) :
    barrierFlags_    { barrierFlags    },
    hasBindlessHeap_ { hasBindlessHeap }

{
    rootParameterIndices_.rootParamDescriptorHeaps[0]   = D3D12RootParameterIndices::invalidIndex;
    rootParameterIndices_.rootParamDescriptorHeaps[1]   = D3D12RootParameterIndices::invalidIndex;
    rootParameterIndices_.rootParamDescriptors[0]       = D3D12RootParameterIndices::invalidIndex;
    rootParameterIndices_.rootParamDescriptors[1]       = D3D12RootParameterIndices::invalidIndex;
}

D3D12PipelineLayout::D3D12PipelineLayout(ID3D12Device* device, const PipelineLayoutDescriptor& desc) :
    D3D12PipelineLayout { desc.barrierFlags, HasPipelineLayoutDescBindlessHeap(desc) }
{
    CreateRootSignature(device, desc);
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D12PipelineLayout::SetDebugName(const char* name)
{
    D3D12SetObjectName(finalizedRootSignature_.Get(), name);
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
    return static_cast<std::uint32_t>(uniforms_.size());
}

static D3D12_ROOT_SIGNATURE_FLAGS GetD3DRootSignatureFlags(long convolutedStageFlags, bool hasBindlessHeap)
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

    /* Add bindless flags */
    if (hasBindlessHeap)
    {
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
        signatureFlags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;
    }

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
    /* Keep pointer to D3D12 device */
    device_ = device;

    /* Convolute all stage flags */
    convolutedStageFlags_ = ConvoluteLayoutStageFlags(desc);

    /* Create root signature */
    rootSignature_ = MakeUnique<D3D12RootSignature>();
    rootSignature_->Reset(static_cast<UINT>(desc.bindings.size()), 0);

    BuildRootSignature(*rootSignature_, desc);

    /* Finalize root signature if there is no permutation needed */
    if (!NeedsRootConstantPermutation())
    {
        const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = GetD3DRootSignatureFlags(GetConvolutedStageFlags(), HasBindlessHeap());
        finalizedRootSignature_ = rootSignature_->Finalize(device, rootSignatureFlags, &serializedBlob_);
        rootSignature_.reset();
    }
}

void D3D12PipelineLayout::ReleaseRootSignature()
{
    finalizedRootSignature_.Reset();
}

ComPtr<ID3D12RootSignature> D3D12PipelineLayout::CreateRootSignatureWith32BitConstants(
    const ArrayView<D3D12Shader*>&          shaders,
    std::vector<D3D12RootConstantLocation>& outRootConstantMap) const
{
    if (!rootSignature_)
        return nullptr;

    /* Reflect all constant buffers from all shaders */
    long cbufferStageFlags = 0;
    std::vector<const D3D12ConstantBufferReflection*> cbufferReflections;

    auto FindCbufferField = [&cbufferReflections, &cbufferStageFlags](const std::string& name) -> std::pair<const D3D12_ROOT_CONSTANTS*, const D3D12ConstantReflection*>
    {
        for (const D3D12ConstantBufferReflection* cbuffer : cbufferReflections)
        {
            for (const D3D12ConstantReflection& field : cbuffer->fields)
            {
                if (field.name == name)
                {
                    cbufferStageFlags |= cbuffer->stageFlags;
                    return { &(cbuffer->rootConstants), &field };
                }
            }
        }
        return { nullptr, nullptr };
    };

    for (D3D12Shader* shader : shaders)
    {
        LLGL_ASSERT_PTR(shader);

        /* Get cached cbuffer reflection from shader */
        const std::vector<D3D12ConstantBufferReflection>* currentCbufferReflections = nullptr;
        HRESULT hr = shader->ReflectAndCacheConstantBuffers(&currentCbufferReflections);
        DXThrowIfFailed(hr, "failed to reflect constant buffers in D3D12 shader");
        LLGL_ASSERT_PTR(currentCbufferReflections);

        cbufferReflections.reserve(cbufferReflections.size() + currentCbufferReflections->size());
        for (const D3D12ConstantBufferReflection& cbufferReflection : *currentCbufferReflections)
            cbufferReflections.push_back(&cbufferReflection);
    }

    /* Create root signature copy and append parameters to permutation */
    D3D12RootSignature rootSignaturePermutation = *rootSignature_;
    outRootConstantMap.resize(uniforms_.size());

    const std::size_t rootParamOffset = rootSignaturePermutation.GetNumRootParameters();

    auto FindOrAppendRootParameter = [&rootSignaturePermutation, rootParamOffset](const D3D12_ROOT_CONSTANTS& rootConstants) -> UINT
    {
        UINT rootParamIndex = -1;
        if (rootSignaturePermutation.FindCompatibleRootParameter(rootConstants, rootParamOffset, &rootParamIndex) == nullptr)
        {
            D3D12RootParameter* rootParam = rootSignaturePermutation.AppendRootParameter(&rootParamIndex);
            rootParam->InitAsConstants(rootConstants);
        }
        return rootParamIndex;
    };

    for_range(i, uniforms_.size())
    {
        /* Find constant buffer field for specified uniform name */
        const auto field = FindCbufferField(uniforms_[i].name);
        const D3D12_ROOT_CONSTANTS*     rootConstants   = field.first;
        const D3D12ConstantReflection*  fieldReflection = field.second;
        LLGL_ASSERT_PTR(rootConstants);
        LLGL_ASSERT_PTR(fieldReflection);

        /* Find or append root parameter for root constants */
        UINT                rootParamIndex  = FindOrAppendRootParameter(*rootConstants);
        D3D12RootParameter& rootParam       = rootSignaturePermutation[rootParamIndex];

        /* Build root constant map for current uniform descriptor */
        D3D12RootConstantLocation& location = outRootConstantMap[i];
        {
            location.index          = rootParamIndex;
            location.num32BitValues = std::max(1u, GetAlignedSize(fieldReflection->size, 4u) / 4u);
            location.wordOffset     = fieldReflection->offset / 4;
        }
    }

    /* Finalize permutated root signature and append stage flags of all cbuffers used for uniforms */
    const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags = GetD3DRootSignatureFlags(GetConvolutedStageFlags() | cbufferStageFlags, HasBindlessHeap());
    return rootSignaturePermutation.Finalize(device_, rootSignatureFlags);
}

D3D12DescriptorHeapSetLayout D3D12PipelineLayout::GetDescriptorHeapSetLayout() const
{
    D3D12DescriptorHeapSetLayout layout;
    {
        layout.numHeapResourceViews = descriptorHeapLayout_.SumResourceViews();
        layout.numHeapSamplers      = descriptorHeapLayout_.SumSamplers();
        layout.numResourceViews     = descriptorLayout_.SumResourceViews();
        layout.numSamplers          = descriptorLayout_.SumSamplers();
    }
    return layout;
}


/*
 * ======= Private: =======
 */

void D3D12PipelineLayout::BuildRootSignature(
    D3D12RootSignature&             rootSignature,
    const PipelineLayoutDescriptor& desc)
{
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

    //TODO: re-enable until LLGL can translate restrictions on root parameters; see CanResourceHaveRootParameter()
    //BuildRootParameters(rootSignature, D3D12_ROOT_PARAMETER_TYPE_SRV, desc, ResourceType::Buffer, BindFlags::Sampled);
    //BuildRootParameters(rootSignature, D3D12_ROOT_PARAMETER_TYPE_UAV, desc, ResourceType::Buffer, BindFlags::Storage);

    /* Build static samplers */
    BuildStaticSamplers(rootSignature, desc, numStaticSamplers_);

    /* Cache uniform descriptors */
    uniforms_ = desc.uniforms;
}

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
        const BindingDescriptor& binding = layoutDesc.heapBindings[i];
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
    if (D3D12RootParameter* rootParam = rootSignature.FindCompatibleRootParameter(descRangeType))
    {
        /* Append descriptor range to previous root parameter */
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, std::max(1u, bindingDesc.arraySize));
    }
    else
    {
        /* Create new root parameter and append descriptor range */
        UINT rootParamIndex = 0;
        rootParam = rootSignature.AppendRootParameter(&rootParamIndex);
        rootParam->InitAsDescriptorTable(maxNumDescriptorRanges);
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, std::max(1u, bindingDesc.arraySize));

        /* Store root parameter index */
        UINT8& rootParamIndexStored = rootParameterIndices_.rootParamDescriptorHeaps[GetDescriptorTypeShift(descRangeType)];
        LLGL_ASSERT(rootParamIndexStored == D3D12RootParameterIndices::invalidIndex || rootParamIndexStored == rootParamIndex);
        rootParamIndexStored = rootParamIndex;
    }

    /* Cache binding flags in the same order root parameters are build */
    descriptorHeapLayout_.GetDescriptorLocation(descRangeType, outLocation);
}

static bool CanResourceHaveRootParameter(const ResourceType resourceType, long bindFlags)
{
    /* Only raw or structured buffers can be used as SRV and UAV, so only allow CBV until LLGL can translate those restrictions */
    return (resourceType == ResourceType::Buffer && (bindFlags & BindFlags::ConstantBuffer) != 0);
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
        const BindingDescriptor& binding = layoutDesc.bindings[i];
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
        (indices.rootParamDescriptorHeaps[0] == D3D12RootParameterIndices::invalidIndex ? 0u : indices.rootParamDescriptorHeaps[0] + 1u),
        (indices.rootParamDescriptorHeaps[1] == D3D12RootParameterIndices::invalidIndex ? 0u : indices.rootParamDescriptorHeaps[1] + 1u)
    );
}

void D3D12PipelineLayout::BuildRootParameterTableEntry(
    D3D12RootSignature&             rootSignature,
    D3D12_DESCRIPTOR_RANGE_TYPE     descRangeType,
    const BindingDescriptor&        bindingDesc,
    UINT                            maxNumDescriptorRanges,
    D3D12DescriptorHeapLocation&    outLocation)
{
    /* Find compatible root parameter after root parameter for heap resources */
    const UINT rootParamOffset = GetRootParameterIndexAfterHeapResources(rootParameterIndices_);
    if (D3D12RootParameter* rootParam = rootSignature.FindCompatibleRootParameter(descRangeType, rootParamOffset))
    {
        /* Append descriptor range to previous root parameter */
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, std::max(1u, bindingDesc.arraySize));
    }
    else
    {
        /* Create new root parameter and append descriptor range */
        UINT rootParamIndex = 0;
        rootParam = rootSignature.AppendRootParameter(&rootParamIndex);
        rootParam->InitAsDescriptorTable(maxNumDescriptorRanges);
        rootParam->AppendDescriptorTableRange(descRangeType, bindingDesc.slot, std::max(1u, bindingDesc.arraySize));

        /* Store root parameter index */
        UINT8& rootParamIndexStored = rootParameterIndices_.rootParamDescriptors[GetDescriptorTypeShift(descRangeType)];
        LLGL_ASSERT(rootParamIndexStored == D3D12RootParameterIndices::invalidIndex || rootParamIndexStored == rootParamIndex);
        rootParamIndexStored = rootParamIndex;
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
        const BindingDescriptor& binding = layoutDesc.bindings[i];
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
    D3D12RootParameter* rootParam = rootSignature.AppendRootParameter(&rootParamIndex);
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
    for (const StaticSamplerDescriptor& staticSamplerDesc : layoutDesc.staticSamplers)
    {
        D3D12_STATIC_SAMPLER_DESC* nativeStaticSampler = rootSignature.AppendStaticSampler();
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
