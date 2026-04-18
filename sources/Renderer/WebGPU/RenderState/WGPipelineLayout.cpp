/*
 * WGPipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGPipelineLayout.h"
#include "../WGCore.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


static std::uint32_t AccumulateUniformSizes(ArrayView<UniformDescriptor> uniformDescs)
{
    std::uint32_t totalSize = 0;
    for (const auto& uniformDesc : uniformDescs)
    {
        const std::uint32_t uniformSize = GetUniformTypeSize(uniformDesc.type, uniformDesc.arraySize);
        LLGL_ASSERT(uniformSize > 0, "invalid uniform type (0x%08X) in pipeline layout descriptor", static_cast<std::uint32_t>(uniformDesc.type));
        totalSize += uniformSize;
    }
    return totalSize;
}

static WGPUShaderStage ToWGShaderStage(long inStage)
{
    WGPUShaderStage outStage = WGPUShaderStage_None;

    if ((inStage & StageFlags::VertexStage) != 0)
        outStage |= WGPUShaderStage_Vertex;
    if ((inStage & StageFlags::FragmentStage) != 0)
        outStage |= WGPUShaderStage_Fragment;
    if ((inStage & StageFlags::ComputeStage) != 0)
        outStage |= WGPUShaderStage_Compute;

    return outStage;
}

static WGPUBufferBindingType ToWGBufferBindingType(long bindFlags)
{
    if ((bindFlags & BindFlags::ConstantBuffer) != 0)
        return WGPUBufferBindingType_Uniform;
    if ((bindFlags & BindFlags::Storage) != 0)
        return WGPUBufferBindingType_Storage; // Precedence over WGPUBufferBindingType_ReadOnlyStorage
    if ((bindFlags & BindFlags::Sampled) != 0)
        return WGPUBufferBindingType_ReadOnlyStorage;
    return WGPUBufferBindingType_Undefined;
}

static void ConvertBindGroupLayoutEntry(WGPUBindGroupLayoutEntry& dst, const BindingDescriptor& src)
{
    dst.nextInChain         = nullptr;
    dst.binding             = src.slot.index;
    dst.visibility          = ToWGShaderStage(src.stageFlags);
    dst.bindingArraySize    = src.arraySize;

    if (src.type == ResourceType::Buffer)
    {
        dst.buffer.nextInChain      = nullptr;
        dst.buffer.type             = ToWGBufferBindingType(src.bindFlags);
        dst.buffer.hasDynamicOffset = WGPU_FALSE; //???
        dst.buffer.minBindingSize   = WGPU_WHOLE_SIZE; //???
    }
    else if (src.type == ResourceType::Sampler)
    {
        dst.sampler.nextInChain = nullptr;
        dst.sampler.type        = WGPUSamplerBindingType_Filtering; //TODO: must be generic for filtered, non-filtered, and shadow samplers
    }
    else if (src.type == ResourceType::Texture)
    {
        if ((src.bindFlags & BindFlags::Storage) != 0)
        {
            dst.storageTexture.nextInChain      = nullptr;
            dst.storageTexture.access           = WGPUStorageTextureAccess_ReadWrite;
            dst.storageTexture.format           = WGPUTextureFormat_RGBA8Unorm; //TODO: must be generic
            dst.storageTexture.viewDimension    = WGPUTextureViewDimension_2D; //TODO: must be generic
        }
        else
        {
            dst.texture.nextInChain     = nullptr;
            dst.texture.sampleType      = WGPUTextureSampleType_Float; //TODO: must be generic
            dst.texture.viewDimension   = WGPUTextureViewDimension_2D; //TODO: must be generic
            dst.texture.multisampled    = WGPU_FALSE; //TODO: must be generic
        }
    }
}

WGPipelineLayout::WGPipelineLayout(WGPUDevice device, const PipelineLayoutDescriptor& desc) :
    numHeapBindings_    { static_cast<std::uint32_t>(desc.heapBindings.size())   },
    numBindings_        { static_cast<std::uint32_t>(desc.bindings.size())       },
    numStaticSamplers_  { static_cast<std::uint32_t>(desc.staticSamplers.size()) },
    numUniforms_        { static_cast<std::uint32_t>(desc.uniforms.size())       }
{
    /*
    Create one bind group layout per PipelineLayout.
    This can be improved by either pooling and auto-derivation or new binding flags from the LLGL interface.
    */
    std::vector<WGPUBindGroupLayoutEntry> bindGroupEntries;
    bindGroupEntries.resize(desc.heapBindings.size() + desc.bindings.size() + desc.staticSamplers.size());
    std::size_t bindGroupEntryIndex = 0;

    if (!desc.bindings.empty())
    {
        for (const BindingDescriptor& binding : desc.bindings)
            ConvertBindGroupLayoutEntry(bindGroupEntries[bindGroupEntryIndex++], binding);
    }

    //TODO: heap bindings and static samplers
    if (!desc.heapBindings.empty())
    {
        LLGL_TRAP_NOT_IMPLEMENTED("heap bindings");
    }
    if (!desc.staticSamplers.empty())
    {
        LLGL_TRAP_NOT_IMPLEMENTED("static samplers");
    }

    if (!bindGroupEntries.empty())
    {
        WGPUBindGroupLayoutDescriptor wgpuBindGroupDesc;
        {
            wgpuBindGroupDesc.nextInChain   = nullptr;
            wgpuBindGroupDesc.label         = ToWGStringView(desc.debugName);
            wgpuBindGroupDesc.entryCount    = bindGroupEntries.size();
            wgpuBindGroupDesc.entries       = bindGroupEntries.data();
        }
        bindGroupLayout_ = wgpuDeviceCreateBindGroupLayout(device, &wgpuBindGroupDesc);
        LLGL_ASSERT_PTR(bindGroupLayout_);
    }

    /* Create native WebGPU pipeline layout wiht a single bind group */
    WGPUPipelineLayoutDescriptor wgpuLayoutDesc;
    {
        wgpuLayoutDesc.nextInChain  = nullptr;
        wgpuLayoutDesc.label        = ToWGStringView(desc.debugName);
        if (bindGroupLayout_ != nullptr)
        {
            wgpuLayoutDesc.bindGroupLayoutCount = 1;
            wgpuLayoutDesc.bindGroupLayouts     = &bindGroupLayout_;
        }
        else
        {
            wgpuLayoutDesc.bindGroupLayoutCount = 0;
            wgpuLayoutDesc.bindGroupLayouts     = nullptr;
        }
        wgpuLayoutDesc.immediateSize = AccumulateUniformSizes(desc.uniforms);
    }
    pipelineLayout_ = wgpuDeviceCreatePipelineLayout(device, &wgpuLayoutDesc);
    LLGL_ASSERT_PTR(pipelineLayout_);
}

WGPipelineLayout::~WGPipelineLayout()
{
    if (bindGroupLayout_ != nullptr)
        wgpuBindGroupLayoutRelease(bindGroupLayout_);
    wgpuPipelineLayoutRelease(pipelineLayout_);
}

std::uint32_t WGPipelineLayout::GetNumHeapBindings() const
{
    return numHeapBindings_;
}

std::uint32_t WGPipelineLayout::GetNumBindings() const
{
    return numBindings_;
}

std::uint32_t WGPipelineLayout::GetNumStaticSamplers() const
{
    return numStaticSamplers_;
}

std::uint32_t WGPipelineLayout::GetNumUniforms() const
{
    return numUniforms_;
}


} // /namespace LLGL



// ================================================================================
