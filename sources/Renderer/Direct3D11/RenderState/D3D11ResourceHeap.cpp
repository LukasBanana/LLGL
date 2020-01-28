/*
 * D3D11ResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ResourceHeap.h"
#include "D3D11PipelineLayout.h"
#include "../Buffer/D3D11Buffer.h"
#include "../Buffer/D3D11BufferWithRV.h"
#include "../Texture/D3D11Sampler.h"
#include "../Texture/D3D11Texture.h"
#include "../D3D11Types.h"
#include "../../CheckedCast.h"
#include "../../ResourceBindingIterator.h"
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include "../../../Core/Helper.h"
#include <LLGL/ResourceHeapFlags.h>


namespace LLGL
{


/*
 * Internal structures
 */

/*

The internal buffer of D3D11ResourceHeap is tightly packed which stores all segments of binding points consecutively.
Here is an illustration of the buffer layout for one Texture resouce (at binding point 4) and two StorageBuffer resources (at binding points 5 and 6)
on a 32-bit build, both for the fragment shader stage only:

Offset      Attribute                                   Value   Description                                         Segment
--------------------------------------------------------------------------------------------------------------------------------------------
0x00000000  D3DResourceViewHeapSegment1::segmentSize    16      Size of this segment                                \
0x00000004  D3DResourceViewHeapSegment1::startSlot      4       First binding point                                  |-- Texture segment
0x00000008  D3DResourceViewHeapSegment1::numSlots       1       Number of binding points                             |
0x0000000C  srv[0]                                      <ptr>   1st ID3D11ShaderResourceView for texture            /
0x00000010  D3DResourceViewHeapSegment2::segmentSize    20      Size of this segment                                \
0x00000014  D3DResourceViewHeapSegment2::offsetEnd0     20      Relative offset to initialCount[0] (at 0x00000028)   |
0x00000018  D3DResourceViewHeapSegment2::startSlot      5       First binding point                                  |
0x0000001C  D3DResourceViewHeapSegment2::numSlots       2       Number of binding points                             |-- StorageBuffer segment
0x00000020  uav[0]                                      <ptr>   1st ID3D11UnorderedAccessView for storage buffer     |
0x00000024  uav[1]                                      <ptr>   2nd ID3D11UnorderedAccessView for storage buffer     |
0x00000028  initialCount[0]                             0       1st initial count                                    |
0x0000002C  initialCount[1]                             0       2nd initial count                                   /

*/

/*
Resource view heap (RVH) segment structure with one dynamic sub-buffer
for <ID3D11SamplerState*>, <ID3D11ShaderResourceView*>, or <ID3D11Buffer*>
*/
struct D3DResourceViewHeapSegment1
{
    std::size_t segmentSize;    // TODO: maybe use std::uint16_t for optimization
    UINT        startSlot;      // TODO: maybe use std::uint8_t for optimization
    UINT        numViews;       // TODO: maybe use std::uint8_t for optimization
};

/*
Resource view heap (RVH) segment structure with two dynamic sub-buffers:
one for <ID3D11UnorderedAccessView*> and one for <UINT>
*/
struct D3DResourceViewHeapSegment2
{
    std::size_t segmentSize;    // TODO: maybe use std::uint16_t for optimization
    std::size_t offsetEnd0;     // TODO: maybe use std::uint16_t for optimization
    UINT        startSlot;      // TODO: maybe use std::uint8_t for optimization
    UINT        numViews;       // TODO: maybe use std::uint8_t for optimization
};

/*
Resource view heap (RVH) segment structure with three dynamic sub-buffers:
one for <ID3D11Buffer*> and two for <UINT>
*/
struct D3DResourceViewHeapSegment3
{
    std::size_t segmentSize;    // TODO: maybe use std::uint16_t for optimization
    std::size_t offsetEnd0;     // TODO: maybe use std::uint16_t for optimization
    std::size_t offsetEnd1;     // TODO: maybe use std::uint16_t for optimization
    UINT        startSlot;      // TODO: maybe use std::uint8_t for optimization
    UINT        numViews;       // TODO: maybe use std::uint8_t for optimization
};

struct D3DResourceBinding
{
    UINT                            slot;
    long                            stages; // bitwise OR combination of StageFlags entries
    union
    {
        ID3D11DeviceChild*          obj;
        ID3D11Buffer*               buffer;
        ID3D11UnorderedAccessView*  uav;
        ID3D11ShaderResourceView*   srv;
        ID3D11SamplerState*         sampler;
    };
    UINT                            initialCount;
    UINT                            firstConstant;
    UINT                            numConstants;
};

// Size (in bytes) of each constant register in a constant buffer
static const UINT g_cbufferRegisterSize = 16;


/*
 * D3D11ResourceHeap class
 */

D3D11ResourceHeap::D3D11ResourceHeap(const ResourceHeapDescriptor& desc, bool hasDeviceContextD3D11_1)
{
    /* Get pipeline layout object */
    auto pipelineLayoutD3D = LLGL_CAST(D3D11PipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutD3D)
        throw std::invalid_argument("failed to create resource heap due to missing pipeline layout");

    /* Validate binding descriptors */
    const auto& bindings            = pipelineLayoutD3D->GetBindings();
    const auto  numBindings         = bindings.size();
    const auto  numResourceViews    = desc.resourceViews.size();

    if (numBindings == 0)
        throw std::invalid_argument("cannot create resource heap without bindings in pipeline layout");
    if (numResourceViews % numBindings != 0)
        throw std::invalid_argument("failed to create resource heap because due to mismatch between number of resources and bindings");

    /* Build buffer segments (stage after stage, so the internal buffer is constructed in the correct order) */
    for (std::size_t i = 0; i < numResourceViews; i += numBindings)
    {
        /* Reset segment header, only one is required */
        ResourceBindingIterator resourceIterator { desc.resourceViews, bindings, i };
        InitMemory(segmentation_);

        /* Build resource view segments for GRAPHICS stages in current descriptor set */
        BuildSegmentsForStage(resourceIterator, StageFlags::VertexStage);
        BuildSegmentsForStage(resourceIterator, StageFlags::TessControlStage);
        BuildSegmentsForStage(resourceIterator, StageFlags::TessEvaluationStage);
        BuildSegmentsForStage(resourceIterator, StageFlags::GeometryStage);
        BuildSegmentsForStage(resourceIterator, StageFlags::FragmentStage);

        /* Build resource view segments for COMPUTE stage in current descriptor set */
        if (i == 0)
        {
            if (buffer_.size() > UINT16_MAX)
                throw std::out_of_range("internal buffer for resource heap exceeded limit of 2^16 (65536) bytes");
            bufferOffsetCS_ = static_cast<std::uint16_t>(buffer_.size());
        }

        BuildSegmentsForStage(resourceIterator, StageFlags::ComputeStage);
    }

    /* Store buffer stride */
    stride_ = buffer_.size() / (numResourceViews / numBindings);

    /* Store resource usage bits in segmentation header */
    StoreResourceUsage();

    /* Validate feature level supports all enabled features */
    if (!hasDeviceContextD3D11_1)
    {
        if (HasCbufferRanges())
            throw std::runtime_error("cannot create constant-buffer range for Direct3D API version prior to 11.1");
    }
}

std::uint32_t D3D11ResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(stride_ > 0 ? buffer_.size() / stride_ : 0);
}

void D3D11ResourceHeap::BindForGraphicsPipeline(ID3D11DeviceContext* context, std::uint32_t firstSet)
{
    /* Bind resource views to the graphics shader stages */
    auto byteAlignedBuffer = GetSegmentationHeapStart(firstSet);
    if (segmentation_.hasVSResources) { BindVSResources(context, byteAlignedBuffer); }
    if (segmentation_.hasHSResources) { BindHSResources(context, byteAlignedBuffer); }
    if (segmentation_.hasDSResources) { BindDSResources(context, byteAlignedBuffer); }
    if (segmentation_.hasGSResources) { BindGSResources(context, byteAlignedBuffer); }
    if (segmentation_.hasPSResources) { BindPSResources(context, byteAlignedBuffer); }
}

void D3D11ResourceHeap::BindForComputePipeline(ID3D11DeviceContext* context, std::uint32_t firstSet)
{
    /* Bind resource views to the compute shader stage */
    auto byteAlignedBuffer = GetSegmentationHeapStart(firstSet) + bufferOffsetCS_;
    if (segmentation_.hasCSResources) { BindCSResources(context, byteAlignedBuffer); }
}

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

void D3D11ResourceHeap::BindForGraphicsPipeline1(ID3D11DeviceContext1* context1, std::uint32_t firstSet)
{
    /* Bind resource views and constant-buffer ranges to the graphics shader stages */
    auto byteAlignedBuffer = GetSegmentationHeapStart(firstSet);
    if (segmentation_.hasVSResources)
    {
        BindVSConstantBuffersRange(context1, byteAlignedBuffer);
        BindVSResources(context1, byteAlignedBuffer);
    }
    if (segmentation_.hasHSResources)
    {
        BindHSConstantBuffersRange(context1, byteAlignedBuffer);
        BindHSResources(context1, byteAlignedBuffer);
    }
    if (segmentation_.hasDSResources)
    {
        BindDSConstantBuffersRange(context1, byteAlignedBuffer);
        BindDSResources(context1, byteAlignedBuffer);
    }
    if (segmentation_.hasGSResources)
    {
        BindGSConstantBuffersRange(context1, byteAlignedBuffer);
        BindGSResources(context1, byteAlignedBuffer);
    }
    if (segmentation_.hasPSResources)
    {
        BindPSConstantBuffersRange(context1, byteAlignedBuffer);
        BindPSResources(context1, byteAlignedBuffer);
    }
}

void D3D11ResourceHeap::BindForComputePipeline1(ID3D11DeviceContext1* context1, std::uint32_t firstSet)
{
    /* Bind resource views and constant-buffer ranges to the compute shader stage */
    auto byteAlignedBuffer = GetSegmentationHeapStart(firstSet) + bufferOffsetCS_;
    if (segmentation_.hasCSResources)
    {
        BindCSConstantBuffersRange(context1, byteAlignedBuffer);
        BindCSResources(context1, byteAlignedBuffer);
    }
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL

bool D3D11ResourceHeap::HasCbufferRanges() const
{
    return (segmentation_.hasConstantBufferRanges != 0);
}


/*
 * ======= Private: =======
 */

using D3DResourceBindingFunc = std::function<
    bool(
        D3DResourceBinding&             binding,
        Resource*                       resource,
        const ResourceViewDescriptor&   rvDesc,
        std::uint32_t                   slot,
        long                            stageFlags
    )
>;

static std::vector<D3DResourceBinding> CollectD3DResourceBindings(
    ResourceBindingIterator&        resourceIterator,
    const ResourceType              resourceType,
    long                            bindFlags,
    long                            affectedStage,
    const D3DResourceBindingFunc&   resourceFunc)
{
    /* Collect all binding points of the specified resource type */
    const BindingDescriptor* bindingDesc = nullptr;
    const ResourceViewDescriptor* rvDesc = nullptr;
    resourceIterator.Reset(resourceType, bindFlags, affectedStage);

    std::vector<D3DResourceBinding> resourceBindings;
    resourceBindings.reserve(resourceIterator.GetCount());

    while (auto resource = resourceIterator.Next(&bindingDesc, &rvDesc))
    {
        D3DResourceBinding binding = {};
        if (resourceFunc(binding, resource, *rvDesc, bindingDesc->slot, bindingDesc->stageFlags))
            resourceBindings.push_back(binding);
    }

    /* Sort resources by slot index */
    std::sort(
        resourceBindings.begin(), resourceBindings.end(),
        [](const D3DResourceBinding& lhs, const D3DResourceBinding& rhs)
        {
            return (lhs.slot < rhs.slot);
        }
    );

    return resourceBindings;
}

void D3D11ResourceHeap::BuildSegmentsForStage(ResourceBindingIterator& resourceIterator, long stage)
{
    BuildConstantBufferRangeSegments(resourceIterator, stage);
    BuildConstantBufferSegments(resourceIterator, stage);
    BuildSamplerSegments(resourceIterator, stage);
    BuildShaderResourceViewSegments(resourceIterator, stage);
    BuildUnorderedAccessViewSegments(resourceIterator, stage);
}

void D3D11ResourceHeap::BuildConstantBufferRangeSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all constant buffers with a non-default range */
    auto resourceBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Buffer,
        BindFlags::ConstantBuffer,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot, long stageFlags) -> bool
        {
            if (IsBufferViewEnabled(rvDesc.bufferView))
            {
                /* Store binding for constant-buffer range */
                auto bufferD3D = LLGL_CAST(D3D11Buffer*, resource);
                binding.slot            = slot;
                binding.stages          = stageFlags;
                binding.buffer          = bufferD3D->GetNative();
                binding.firstConstant   = static_cast<UINT>(rvDesc.bufferView.offset / g_cbufferRegisterSize);
                binding.numConstants    = static_cast<UINT>(rvDesc.bufferView.size / g_cbufferRegisterSize);
                return true;
            }
            return false;
        }
    );

    /* Build all resource segments for type <D3DResourceViewHeapSegment3> */
    std::uint8_t numSegments = 0;
    BuildAllSegmentsType3(resourceBindings, stage, numSegments);

    switch (stage)
    {
        case StageFlags::VertexStage:           segmentation_.numVSConstantBufferRangeSegments = numSegments; break;
        case StageFlags::TessControlStage:      segmentation_.numHSConstantBufferRangeSegments = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentation_.numDSConstantBufferRangeSegments = numSegments; break;
        case StageFlags::GeometryStage:         segmentation_.numGSConstantBufferRangeSegments = numSegments; break;
        case StageFlags::FragmentStage:         segmentation_.numPSConstantBufferRangeSegments = numSegments; break;
        case StageFlags::ComputeStage:          segmentation_.numCSConstantBufferRangeSegments = numSegments; break;
    }
}

void D3D11ResourceHeap::BuildConstantBufferSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all constant buffers with a default range */
    auto resourceBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Buffer,
        BindFlags::ConstantBuffer,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot, long stageFlags) -> bool
        {
            if (!IsBufferViewEnabled(rvDesc.bufferView))
            {
                auto bufferD3D = LLGL_CAST(D3D11Buffer*, resource);
                binding.slot    = slot;
                binding.stages  = stageFlags;
                binding.buffer  = bufferD3D->GetNative();
                return true;
            }
            return false;
        }
    );

    /* Build all resource segments for type <D3DResourceViewHeapSegment1> */
    std::uint8_t numSegments = 0;
    BuildAllSegmentsType1(resourceBindings, stage, numSegments);

    switch (stage)
    {
        case StageFlags::VertexStage:           segmentation_.numVSConstantBufferSegments = numSegments; break;
        case StageFlags::TessControlStage:      segmentation_.numHSConstantBufferSegments = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentation_.numDSConstantBufferSegments = numSegments; break;
        case StageFlags::GeometryStage:         segmentation_.numGSConstantBufferSegments = numSegments; break;
        case StageFlags::FragmentStage:         segmentation_.numPSConstantBufferSegments = numSegments; break;
        case StageFlags::ComputeStage:          segmentation_.numCSConstantBufferSegments = numSegments; break;
    }
}

void D3D11ResourceHeap::BuildShaderResourceViewSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all shader resource views for textures */
    auto textureBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Texture,
        BindFlags::Sampled,
        stage,
        [this](D3DResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot, long stageFlags) -> bool
        {
            auto textureD3D = LLGL_CAST(D3D11Texture*, resource);
            if (auto srv = this->GetOrCreateTextureSRV(*textureD3D, rvDesc.textureView))
            {
                binding.slot    = slot;
                binding.stages  = stageFlags;
                binding.srv     = srv;
                return true;
            }
            return false;
        }
    );

    /* Collect all shader resource views for storage buffers */
    auto bufferBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Buffer,
        BindFlags::Sampled,
        stage,
        [this](D3DResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot, long stageFlags) -> bool
        {
            auto bufferD3D = LLGL_CAST(D3D11Buffer*, resource);
            if ((bufferD3D->GetBindFlags() & BindFlags::Sampled) != 0)
            {
                auto bufferWithRVD3D = LLGL_CAST(D3D11BufferWithRV*, bufferD3D);
                if (auto srv = this->GetOrCreateBufferSRV(*bufferWithRVD3D, rvDesc.bufferView))
                {
                    binding.slot    = slot;
                    binding.stages  = stageFlags;
                    binding.srv     = srv;
                    return true;
                }
            }
            return false;
        }
    );

    /* Build all resource segments for type <D3DResourceViewHeapSegment1> */
    std::uint8_t numSegments = 0;
    BuildAllSegmentsType1(textureBindings, stage, numSegments);
    BuildAllSegmentsType1(bufferBindings, stage, numSegments);

    switch (stage)
    {
        case StageFlags::VertexStage:           segmentation_.numVSShaderResourceViewSegments = numSegments; break;
        case StageFlags::TessControlStage:      segmentation_.numHSShaderResourceViewSegments = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentation_.numDSShaderResourceViewSegments = numSegments; break;
        case StageFlags::GeometryStage:         segmentation_.numGSShaderResourceViewSegments = numSegments; break;
        case StageFlags::FragmentStage:         segmentation_.numPSShaderResourceViewSegments = numSegments; break;
        case StageFlags::ComputeStage:          segmentation_.numCSShaderResourceViewSegments = numSegments; break;
    }
}

void D3D11ResourceHeap::BuildUnorderedAccessViewSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all unordered acces views for textures */
    auto textureBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Texture,
        BindFlags::Storage,
        stage,
        [this](D3DResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot, long stageFlags) -> bool
        {
            auto textureD3D = LLGL_CAST(D3D11Texture*, resource);
            if (auto uav = this->GetOrCreateTextureUAV(*textureD3D, rvDesc.textureView))
            {
                binding.slot            = slot;
                binding.stages          = stageFlags;
                binding.uav             = uav;
                binding.initialCount    = 0;
                return true;
            }
            return false;
        }
    );

    /* Collect all unordered acces views for storage buffers */
    auto bufferBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Buffer,
        BindFlags::Storage,
        stage,
        [this](D3DResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot, long stageFlags) -> bool
        {
            auto bufferD3D = LLGL_CAST(D3D11Buffer*, resource);
            if ((bufferD3D->GetBindFlags() & BindFlags::Storage) != 0)
            {
                auto bufferWithRVD3D = LLGL_CAST(D3D11BufferWithRV*, bufferD3D);
                if (auto uav = this->GetOrCreateBufferUAV(*bufferWithRVD3D, rvDesc.bufferView))
                {
                    binding.slot            = slot;
                    binding.stages          = stageFlags;
                    binding.uav             = uav;
                    binding.initialCount    = 0;
                    return true;
                }
            }
            return false;
        }
    );

    /* Build all resource segments for type <D3DResourceViewHeapSegment2> */
    std::uint8_t numSegments = 0;
    BuildAllSegmentsType2(textureBindings, stage, numSegments);
    BuildAllSegmentsType2(bufferBindings, stage, numSegments);

    switch (stage)
    {
        case StageFlags::FragmentStage: segmentation_.numPSUnorderedAccessViewSegments = numSegments; break;
        case StageFlags::ComputeStage:  segmentation_.numCSUnorderedAccessViewSegments = numSegments; break;
        default:                        break;
    }
}

void D3D11ResourceHeap::BuildSamplerSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all sampler states */
    auto resourceBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Sampler,
        0,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& /*rvDesc*/, std::uint32_t slot, long stageFlags) -> bool
        {
            auto samplerD3D = LLGL_CAST(D3D11Sampler*, resource);
            binding.slot    = slot;
            binding.stages  = stageFlags;
            binding.sampler = samplerD3D->GetNative();
            return true;
        }
    );

    /* Build all resource segments for type <D3DResourceViewHeapSegment1> */
    std::uint8_t numSegments = 0;
    BuildAllSegmentsType1(resourceBindings, stage, numSegments);

    switch (stage)
    {
        case StageFlags::VertexStage:           segmentation_.numVSSamplerSegments = numSegments; break;
        case StageFlags::TessControlStage:      segmentation_.numHSSamplerSegments = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentation_.numDSSamplerSegments = numSegments; break;
        case StageFlags::GeometryStage:         segmentation_.numGSSamplerSegments = numSegments; break;
        case StageFlags::FragmentStage:         segmentation_.numPSSamplerSegments = numSegments; break;
        case StageFlags::ComputeStage:          segmentation_.numCSSamplerSegments = numSegments; break;
    }
}

void D3D11ResourceHeap::BuildAllSegments(
    const std::vector<D3DResourceBinding>&  resourceBindings,
    const BuildSegmentFunc&                 buildSegmentFunc,
    long                                    affectedStage,
    std::uint8_t&                           numSegments)
{
    if (!resourceBindings.empty())
    {
        /* Initialize iterators for sub-ranges of input bindings */
        auto itStart = resourceBindings.begin();
        auto itPrev  = itStart;
        auto it      = itStart;
        UINT count   = 0;

        for (++it, ++count; it != resourceBindings.end(); ++it, ++count)
        {
            if (it->slot > itPrev->slot + 1)
            {
                /* Build next segment */
                buildSegmentFunc(itStart, count);
                ++numSegments;
                count   = 0;
                itStart = it;
            }
            itPrev = it;
        }

        if (itStart != resourceBindings.end())
        {
            /* Add last segment */
            buildSegmentFunc(itStart, count);
            ++numSegments;
        }
    }
}

void D3D11ResourceHeap::BuildAllSegmentsType1(
    const std::vector<D3DResourceBinding>&  resourceBindings,
    long                                    affectedStage,
    std::uint8_t&                           numSegments)
{
    BuildAllSegments(
        resourceBindings,
        std::bind(&D3D11ResourceHeap::BuildSegment1, this, std::placeholders::_1, std::placeholders::_2),
        affectedStage,
        numSegments
    );
}

void D3D11ResourceHeap::BuildAllSegmentsType2(
    const std::vector<D3DResourceBinding>&  resourceBindings,
    long                                    affectedStage,
    std::uint8_t&                           numSegments)
{
    BuildAllSegments(
        resourceBindings,
        std::bind(&D3D11ResourceHeap::BuildSegment2, this, std::placeholders::_1, std::placeholders::_2),
        affectedStage,
        numSegments
    );
}

void D3D11ResourceHeap::BuildAllSegmentsType3(
    const std::vector<D3DResourceBinding>&  resourceBindings,
    long                                    affectedStage,
    std::uint8_t&                           numSegments)
{
    BuildAllSegments(
        resourceBindings,
        std::bind(&D3D11ResourceHeap::BuildSegment3, this, std::placeholders::_1, std::placeholders::_2),
        affectedStage,
        numSegments
    );
}

void D3D11ResourceHeap::BuildSegment1(D3DResourceBindingIter it, UINT count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const std::size_t segmentSize = sizeof(D3DResourceViewHeapSegment1) + sizeof(ID3D11DeviceChild*) * count;
    buffer_.resize(startOffset + segmentSize);

    /* Write segment header */
    auto segment = reinterpret_cast<D3DResourceViewHeapSegment1*>(&buffer_[startOffset]);
    {
        segment->segmentSize    = segmentSize;
        segment->startSlot      = it->slot;
        segment->numViews       = count;
    }

    /* Write segment body */
    auto segmentObjs = reinterpret_cast<ID3D11DeviceChild**>(&buffer_[startOffset + sizeof(D3DResourceViewHeapSegment1)]);
    for (UINT i = 0; i < count; ++i, ++it)
        segmentObjs[i] = it->obj;
}

void D3D11ResourceHeap::BuildSegment2(D3DResourceBindingIter it, UINT count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const std::size_t segmentOffsetEnd0 = sizeof(D3DResourceViewHeapSegment2) + sizeof(ID3D11UnorderedAccessView*) * count;
    const std::size_t segmentSize       = segmentOffsetEnd0 + sizeof(UINT) * count;
    buffer_.resize(startOffset + segmentSize);

    /* Write segment header */
    auto segment = reinterpret_cast<D3DResourceViewHeapSegment2*>(&buffer_[startOffset]);
    {
        segment->segmentSize    = segmentSize;
        segment->offsetEnd0     = segmentOffsetEnd0;
        segment->startSlot      = it->slot;
        segment->numViews       = count;
    }

    /* Write first part of segment body (of type <ID3D11UnorderedAccessView*>) */
    auto segmentUAVs = reinterpret_cast<ID3D11UnorderedAccessView**>(&buffer_[startOffset + sizeof(D3DResourceViewHeapSegment2)]);
    auto begin = it;
    for (UINT i = 0; i < count; ++i, ++it)
        segmentUAVs[i] = it->uav;

    /* Write second part of segment body (of type <UINT>) */
    auto segmentInitialCounts = reinterpret_cast<UINT*>(&buffer_[startOffset + segmentOffsetEnd0]);
    it = begin;
    for (UINT i = 0; i < count; ++i, ++it)
        segmentInitialCounts[i] = it->initialCount;
}

void D3D11ResourceHeap::BuildSegment3(D3DResourceBindingIter it, UINT count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const std::size_t segmentOffsetEnd0 = sizeof(D3DResourceViewHeapSegment3) + sizeof(ID3D11Buffer*) * count;
    const std::size_t segmentOffsetEnd1 = segmentOffsetEnd0 + sizeof(UINT) * count;
    const std::size_t segmentSize       = segmentOffsetEnd1 + sizeof(UINT) * count;
    buffer_.resize(startOffset + segmentSize);

    /* Write segment header */
    auto segment = reinterpret_cast<D3DResourceViewHeapSegment3*>(&buffer_[startOffset]);
    {
        segment->segmentSize    = segmentSize;
        segment->offsetEnd0     = segmentOffsetEnd0;
        segment->offsetEnd1     = segmentOffsetEnd1;
        segment->startSlot      = it->slot;
        segment->numViews       = count;
    }

    /* Write first part of segment body (of type <ID3D11Buffer*>) */
    auto segmentBuffers = reinterpret_cast<ID3D11Buffer**>(&buffer_[startOffset + sizeof(D3DResourceViewHeapSegment3)]);
    auto begin = it;
    for (UINT i = 0; i < count; ++i, ++it)
        segmentBuffers[i] = it->buffer;

    /* Write second part of segment body (of type <UINT>) */
    auto segmentConstants0 = reinterpret_cast<UINT*>(&buffer_[startOffset + segmentOffsetEnd0]);
    it = begin;
    for (UINT i = 0; i < count; ++i, ++it)
        segmentConstants0[i] = it->firstConstant;

    /* Write second part of segment body (of type <UINT>) */
    auto segmentConstants1 = reinterpret_cast<UINT*>(&buffer_[startOffset + segmentOffsetEnd1]);
    it = begin;
    for (UINT i = 0; i < count; ++i, ++it)
        segmentConstants1[i] = it->numConstants;
}

template <typename T, typename TSegment>
T* const* CastToD3DViews(const std::int8_t* byteAlignedBuffer)
{
    return reinterpret_cast<T* const*>(byteAlignedBuffer + sizeof(TSegment));
}

static ID3D11Buffer* const* CastToD3D11Buffers(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11Buffer, D3DResourceViewHeapSegment1>(byteAlignedBuffer);
}

static ID3D11SamplerState* const* CastToD3D11SamplerStates(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11SamplerState, D3DResourceViewHeapSegment1>(byteAlignedBuffer);
}

static ID3D11ShaderResourceView* const* CastToD3D11ShaderResourceViews(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11ShaderResourceView, D3DResourceViewHeapSegment1>(byteAlignedBuffer);
}

static ID3D11UnorderedAccessView* const* CastToD3D11UnorderedAccessView(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11UnorderedAccessView, D3DResourceViewHeapSegment2>(byteAlignedBuffer);
}

static ID3D11Buffer* const* CastToD3D11BufferRanges(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11Buffer, D3DResourceViewHeapSegment3>(byteAlignedBuffer);
}

void D3D11ResourceHeap::StoreResourceUsage()
{
    /* Store information for which stages any resources have been specified */
    #define LLGL_STORE_STAGE_RESOURCE_USAGE(STAGE)                          \
        if ( segmentation_.num##STAGE##SamplerSegments              > 0 ||  \
             segmentation_.num##STAGE##ConstantBufferRangeSegments  > 0 ||  \
             segmentation_.num##STAGE##ConstantBufferSegments       > 0 ||  \
             segmentation_.num##STAGE##ShaderResourceViewSegments   > 0 )   \
        {                                                                   \
            segmentation_.has##STAGE##Resources = 1;                        \
        }

    LLGL_STORE_STAGE_RESOURCE_USAGE(VS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(HS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(DS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(GS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(PS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(CS);

    #undef LLGL_STORE_STAGE_RESOURCE_USAGE

    /* Extend the determination for unordered access views */
    if (segmentation_.numPSUnorderedAccessViewSegments > 0)
        segmentation_.hasPSResources = 1;
    if (segmentation_.numCSUnorderedAccessViewSegments > 0)
        segmentation_.hasCSResources = 1;

    /* Store boolean if constant-buffer ranges are used (requires feature level D3D_FEATURE_LEVEL_11_1) */
    const bool hasCbufferRanges =
    (
        segmentation_.numVSConstantBufferRangeSegments > 0 ||
        segmentation_.numHSConstantBufferRangeSegments > 0 ||
        segmentation_.numDSConstantBufferRangeSegments > 0 ||
        segmentation_.numGSConstantBufferRangeSegments > 0 ||
        segmentation_.numPSConstantBufferRangeSegments > 0 ||
        segmentation_.numCSConstantBufferRangeSegments > 0
    );
    segmentation_.hasConstantBufferRanges = (hasCbufferRanges ? 1 : 0);
}

void D3D11ResourceHeap::BindVSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint32_t i = 0; i < segmentation_.numVSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint32_t i = 0; i < segmentation_.numVSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint32_t i = 0; i < segmentation_.numVSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindHSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint32_t i = 0; i < segmentation_.numHSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint32_t i = 0; i < segmentation_.numHSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint32_t i = 0; i < segmentation_.numHSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindDSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint32_t i = 0; i < segmentation_.numDSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint32_t i = 0; i < segmentation_.numDSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint32_t i = 0; i < segmentation_.numDSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindGSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint32_t i = 0; i < segmentation_.numGSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint32_t i = 0; i < segmentation_.numGSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint32_t i = 0; i < segmentation_.numGSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindPSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint32_t i = 0; i < segmentation_.numPSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint32_t i = 0; i < segmentation_.numPSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint32_t i = 0; i < segmentation_.numPSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all unordered access views (UAV) */
    for (std::uint32_t i = 0; i < segmentation_.numPSUnorderedAccessViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment2*>(byteAlignedBuffer);
        context->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
            nullptr,
            nullptr,
            segment->startSlot,
            segment->numViews,
            CastToD3D11UnorderedAccessView(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindCSResources(ID3D11DeviceContext* context, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint32_t i = 0; i < segmentation_.numCSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint32_t i = 0; i < segmentation_.numCSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint32_t i = 0; i < segmentation_.numCSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all unordered access views (UAV) */
    for (std::uint32_t i = 0; i < segmentation_.numCSUnorderedAccessViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment2*>(byteAlignedBuffer);
        context->CSSetUnorderedAccessViews(
            segment->startSlot,
            segment->numViews,
            CastToD3D11UnorderedAccessView(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

#if 0 //WIP
/*
Workaround for emulated command lists
See https://docs.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1#remarks
*/
static void VSSetConstantBuffersNull(ID3D11DeviceContext* context, UINT startSlot, UINT numBuffers)
{
    ID3D11Buffer* nullBuffer[2] = { nullptr, nullptr };
    if (numBuffers > 2)
    {
        context->VSSetConstantBuffers(startSlot, 1, &(nullBuffer[0]));
        context->VSSetConstantBuffers(startSlot + numBuffers - 1, 1, &(nullBuffer[1]));
    }
    else if (numBuffers == 2)
        context->VSSetConstantBuffers(startSlot, 2, nullBuffer);
    else
        context->VSSetConstantBuffers(startSlot, 1, nullBuffer);
}
#endif

void D3D11ResourceHeap::BindVSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers in their given range */
    for (std::uint32_t i = 0; i < segmentation_.numVSConstantBufferRangeSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment3*>(byteAlignedBuffer);
        context1->VSSetConstantBuffers1(
            segment->startSlot,
            segment->numViews,
            CastToD3D11BufferRanges(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd1)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindHSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers in their given range */
    for (std::uint32_t i = 0; i < segmentation_.numHSConstantBufferRangeSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment3*>(byteAlignedBuffer);
        context1->HSSetConstantBuffers1(
            segment->startSlot,
            segment->numViews,
            CastToD3D11BufferRanges(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd1)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindDSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers in their given range */
    for (std::uint32_t i = 0; i < segmentation_.numDSConstantBufferRangeSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment3*>(byteAlignedBuffer);
        context1->DSSetConstantBuffers1(
            segment->startSlot,
            segment->numViews,
            CastToD3D11BufferRanges(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd1)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindGSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers in their given range */
    for (std::uint32_t i = 0; i < segmentation_.numGSConstantBufferRangeSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment3*>(byteAlignedBuffer);
        context1->GSSetConstantBuffers1(
            segment->startSlot,
            segment->numViews,
            CastToD3D11BufferRanges(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd1)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindPSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers in their given range */
    for (std::uint32_t i = 0; i < segmentation_.numPSConstantBufferRangeSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment3*>(byteAlignedBuffer);
        context1->PSSetConstantBuffers1(
            segment->startSlot,
            segment->numViews,
            CastToD3D11BufferRanges(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd1)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindCSConstantBuffersRange(ID3D11DeviceContext1* context1, const std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers in their given range */
    for (std::uint32_t i = 0; i < segmentation_.numCSConstantBufferRangeSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment3*>(byteAlignedBuffer);
        context1->CSSetConstantBuffers1(
            segment->startSlot,
            segment->numViews,
            CastToD3D11BufferRanges(byteAlignedBuffer),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd1)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL

ID3D11ShaderResourceView* D3D11ResourceHeap::GetOrCreateTextureSRV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc)
{
    if (IsTextureViewEnabled(textureViewDesc))
    {
        /* Create an SRV for the specified texture subresource */
        ComPtr<ID3D11Device> device;
        textureD3D.GetNative().resource->GetDevice(device.GetAddressOf());

        ComPtr<ID3D11ShaderResourceView> srv;
        textureD3D.CreateSubresourceSRV(
            device.Get(),
            srv.GetAddressOf(),
            textureViewDesc.type,
            D3D11Types::Map(textureViewDesc.format),
            textureViewDesc.subresource.baseMipLevel,
            textureViewDesc.subresource.numMipLevels,
            textureViewDesc.subresource.baseArrayLayer,
            textureViewDesc.subresource.numArrayLayers
        );

        /* Store SRV in container to release together with resource heap */
        srvs_.push_back(std::move(srv));
        return srvs_.back().Get();
    }
    else
    {
        /* Return standard SRV of this texture */
        return textureD3D.GetSRV();
    }
}

ID3D11UnorderedAccessView* D3D11ResourceHeap::GetOrCreateTextureUAV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc)
{
    if (IsTextureViewEnabled(textureViewDesc))
    {
        /* Create a UAV for the specified texture subresource */
        ComPtr<ID3D11Device> device;
        textureD3D.GetNative().resource->GetDevice(device.GetAddressOf());

        ComPtr<ID3D11UnorderedAccessView> uav;
        textureD3D.CreateSubresourceUAV(
            device.Get(),
            uav.GetAddressOf(),
            textureViewDesc.type,
            D3D11Types::Map(textureViewDesc.format),
            textureViewDesc.subresource.baseMipLevel,
            textureViewDesc.subresource.baseArrayLayer,
            textureViewDesc.subresource.numArrayLayers
        );

        /* Store UAV in container to release together with resource heap */
        uavs_.push_back(std::move(uav));
        return uavs_.back().Get();
    }
    else
    {
        /* Return standard UAV of this texture */
        return textureD3D.GetUAV();
    }
}

// Returns the buffer stride (in bytes) of the specified format
static UINT GetFormatBufferStride(const Format format)
{
    /* Get buffer stride by format */
    const auto& formatAttribs = GetFormatAttribs(format);
    const UINT stride = (formatAttribs.bitSize / formatAttribs.blockWidth / formatAttribs.blockHeight / 8);
    if (stride == 0)
        throw std::runtime_error("cannot create buffer subresource with format stride of 0");
    return stride;
}

ID3D11ShaderResourceView* D3D11ResourceHeap::GetOrCreateBufferSRV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc)
{
    if (IsBufferViewEnabled(bufferViewDesc))
    {
        /* Get buffer stride by format */
        const UINT stride = GetFormatBufferStride(bufferViewDesc.format);

        /* Create a UAV for the specified texture subresource */
        ComPtr<ID3D11Device> device;
        bufferD3D.GetNative()->GetDevice(device.GetAddressOf());

        ComPtr<ID3D11ShaderResourceView> srv;
        bufferD3D.CreateSubresourceSRV(
            device.Get(),
            srv.GetAddressOf(),
            D3D11Types::Map(bufferViewDesc.format),
            static_cast<UINT>(bufferViewDesc.offset / stride),
            static_cast<UINT>(bufferViewDesc.size / stride)
        );

        /* Store SRV in container to release together with resource heap */
        srvs_.push_back(std::move(srv));
        return srvs_.back().Get();
    }
    else
    {
        /* Return standard SRV of this buffer */
        return bufferD3D.GetSRV();
    }
}

ID3D11UnorderedAccessView* D3D11ResourceHeap::GetOrCreateBufferUAV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc)
{
    if (IsBufferViewEnabled(bufferViewDesc))
    {
        /* Get buffer stride by format */
        const UINT stride = GetFormatBufferStride(bufferViewDesc.format);

        /* Create a UAV for the specified texture subresource */
        ComPtr<ID3D11Device> device;
        bufferD3D.GetNative()->GetDevice(device.GetAddressOf());

        ComPtr<ID3D11UnorderedAccessView> uav;
        bufferD3D.CreateSubresourceUAV(
            device.Get(),
            uav.GetAddressOf(),
            D3D11Types::Map(bufferViewDesc.format),
            static_cast<UINT>(bufferViewDesc.offset / stride),
            static_cast<UINT>(bufferViewDesc.size / stride)
        );

        /* Store UAV in container to release together with resource heap */
        uavs_.push_back(std::move(uav));
        return uavs_.back().Get();
    }
    else
    {
        /* Return standard UAV of this buffer */
        return bufferD3D.GetUAV();
    }
}

const std::int8_t* D3D11ResourceHeap::GetSegmentationHeapStart(std::uint32_t firstSet) const
{
    return (buffer_.data() + stride_ * firstSet);
}


} // /namespace LLGL



// ================================================================================
