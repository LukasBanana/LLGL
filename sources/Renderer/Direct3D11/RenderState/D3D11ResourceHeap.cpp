/*
 * D3D11ResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11ResourceHeap.h"
#include "D3D11PipelineLayout.h"
#include "../Buffer/D3D11Buffer.h"
#include "../Buffer/D3D11StorageBuffer.h"
#include "../Texture/D3D11Sampler.h"
#include "../Texture/D3D11Texture.h"
#include "../../CheckedCast.h"
#include "../../ResourceBindingIterator.h"
#include "../../../Core/Helper.h"


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
Resource view heap (RVH) segment structure with tow dynamic sub-buffers,
one for <ID3D11UnorderedAccessView*> and one for <UINT>
*/
struct D3DResourceViewHeapSegment2
{
    std::size_t segmentSize;    // TODO: maybe use std::uint16_t for optimization
    std::size_t offsetEnd0;     // TODO: maybe use std::uint16_t for optimization
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
};


/*
 * D3D11ResourceHeap class
 */

D3D11ResourceHeap::D3D11ResourceHeap(const ResourceHeapDescriptor& desc)
{
    /* Initialize segmentation header */
    InitMemory(segmentationHeader_);

    /* Get pipeline layout object */
    auto pipelineLayoutD3D = LLGL_CAST(D3D11PipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutD3D)
        throw std::invalid_argument("failed to create resource heap due to missing pipeline layout");

    /* Validate binding descriptors */
    const auto& bindings = pipelineLayoutD3D->GetBindings();
    if (desc.resourceViews.size() != bindings.size())
        throw std::invalid_argument("failed to create resource heap due to mismatch between number of resources and bindings");

    /* Build buffer segments (stage after stage, so the internal buffer is constructed in the correct order) */
    ResourceBindingIterator resourceIterator { desc.resourceViews, bindings };

    BuildSegmentsForStage(resourceIterator, StageFlags::VertexStage);
    BuildSegmentsForStage(resourceIterator, StageFlags::TessControlStage);
    BuildSegmentsForStage(resourceIterator, StageFlags::TessEvaluationStage);
    BuildSegmentsForStage(resourceIterator, StageFlags::GeometryStage);
    BuildSegmentsForStage(resourceIterator, StageFlags::FragmentStage);

    /* Store buffer offset for compute shader and check for boundary */
    if (buffer_.size() > static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max()))
        throw std::out_of_range("internal buffer for resource heap exceeded limit of 2^16 (65536) bytes");

    bufferOffsetCS_ = static_cast<std::uint16_t>(buffer_.size());
    BuildSegmentsForStage(resourceIterator, StageFlags::ComputeStage);

    StoreResourceUsage();
}

void D3D11ResourceHeap::BindForGraphicsPipeline(ID3D11DeviceContext* context)
{
    auto byteAlignedBuffer = buffer_.data();
    if (segmentationHeader_.hasVSResources) { BindVSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasHSResources) { BindHSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasDSResources) { BindDSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasGSResources) { BindGSResources(context, byteAlignedBuffer); }
    if (segmentationHeader_.hasPSResources) { BindPSResources(context, byteAlignedBuffer); }
}

void D3D11ResourceHeap::BindForComputePipeline(ID3D11DeviceContext* context)
{
    auto byteAlignedBuffer = buffer_.data();
    byteAlignedBuffer += bufferOffsetCS_;
    if (segmentationHeader_.hasCSResources) { BindCSResources(context, byteAlignedBuffer); }
}


/*
 * ======= Private: =======
 */

using D3DResourceBindingFunc = std::function<bool(D3DResourceBinding& binding, Resource* resource, std::uint32_t slot, long stageFlags)>;

static std::vector<D3DResourceBinding> CollectD3DResourceBindings(
    ResourceBindingIterator&        resourceIterator,
    const ResourceType              resourceType,
    long                            affectedStage,
    const D3DResourceBindingFunc&   resourceFunc)
{
    /* Collect all binding points of the specified resource type */
    BindingDescriptor bindingDesc;
    resourceIterator.Reset(resourceType, affectedStage);

    std::vector<D3DResourceBinding> resourceBindings;
    resourceBindings.reserve(resourceIterator.GetCount());

    while (auto resource = resourceIterator.Next(bindingDesc))
    {
        D3DResourceBinding binding = {};
        if (resourceFunc(binding, resource, bindingDesc.slot, bindingDesc.stageFlags))
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
    BuildConstantBufferSegments(resourceIterator, stage);
    BuildSamplerSegments(resourceIterator, stage);
    BuildShaderResourceViewSegments(resourceIterator, stage);
    BuildUnorderedAccessViewSegments(resourceIterator, stage);
}

void D3D11ResourceHeap::BuildConstantBufferSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all constant buffers */
    auto resourceBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::ConstantBuffer,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, std::uint32_t slot, long stageFlags) -> bool
        {
            auto bufferD3D = LLGL_CAST(D3D11Buffer*, resource);
            binding.slot    = slot;
            binding.stages  = stageFlags;
            binding.buffer  = bufferD3D->GetNative();
            return true;
        }
    );

    /* Build all resource segments for type <D3DResourceViewHeapSegment1> */
    std::uint8_t numSegments = 0;
    BuildAllSegmentsType1(resourceBindings, stage, numSegments);

    switch (stage)
    {
        case StageFlags::VertexStage:           segmentationHeader_.numVSConstantBufferSegments = numSegments; break;
        case StageFlags::TessControlStage:      segmentationHeader_.numHSConstantBufferSegments = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentationHeader_.numDSConstantBufferSegments = numSegments; break;
        case StageFlags::GeometryStage:         segmentationHeader_.numGSConstantBufferSegments = numSegments; break;
        case StageFlags::FragmentStage:         segmentationHeader_.numPSConstantBufferSegments = numSegments; break;
        case StageFlags::ComputeStage:          segmentationHeader_.numCSConstantBufferSegments = numSegments; break;
    }
}

void D3D11ResourceHeap::BuildShaderResourceViewSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all shader resource views for textures */
    auto textureBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Texture,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, std::uint32_t slot, long stageFlags) -> bool
        {
            auto textureD3D = LLGL_CAST(D3D11Texture*, resource);
            if ((stageFlags & StageFlags::BindUnorderedAccess) == 0)
            {
                if (auto srv = textureD3D->GetSRV())
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

    /* Collect all shader resource views for storage buffers */
    auto bufferBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::StorageBuffer,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, std::uint32_t slot, long stageFlags) -> bool
        {
            auto bufferD3D = LLGL_CAST(D3D11StorageBuffer*, resource);
            if ((stageFlags & StageFlags::BindUnorderedAccess) == 0)
            {
                if (auto srv = bufferD3D->GetSRV())
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
        case StageFlags::VertexStage:           segmentationHeader_.numVSShaderResourceViewSegments = numSegments; break;
        case StageFlags::TessControlStage:      segmentationHeader_.numHSShaderResourceViewSegments = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentationHeader_.numDSShaderResourceViewSegments = numSegments; break;
        case StageFlags::GeometryStage:         segmentationHeader_.numGSShaderResourceViewSegments = numSegments; break;
        case StageFlags::FragmentStage:         segmentationHeader_.numPSShaderResourceViewSegments = numSegments; break;
        case StageFlags::ComputeStage:          segmentationHeader_.numCSShaderResourceViewSegments = numSegments; break;
    }
}

void D3D11ResourceHeap::BuildUnorderedAccessViewSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all unordered acces views for textures */
    auto textureBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Texture,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, std::uint32_t slot, long stageFlags) -> bool
        {
            auto textureD3D = LLGL_CAST(D3D11Texture*, resource);
            if ((stageFlags & StageFlags::BindUnorderedAccess) != 0)
            {
                if (auto uav = textureD3D->GetUAV())
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

    /* Collect all unordered acces views for storage buffers */
    auto bufferBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::StorageBuffer,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, std::uint32_t slot, long stageFlags) -> bool
        {
            auto bufferD3D = LLGL_CAST(D3D11StorageBuffer*, resource);
            if ((stageFlags & StageFlags::BindUnorderedAccess) != 0)
            {
                if (auto uav = bufferD3D->GetUAV())
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
        case StageFlags::FragmentStage: segmentationHeader_.numPSUnorderedAccessViewSegments = numSegments; break;
        case StageFlags::ComputeStage:  segmentationHeader_.numCSUnorderedAccessViewSegments = numSegments; break;
        default:                        break;
    }
}

void D3D11ResourceHeap::BuildSamplerSegments(ResourceBindingIterator& resourceIterator, long stage)
{
    /* Collect all sampler states */
    auto resourceBindings = CollectD3DResourceBindings(
        resourceIterator,
        ResourceType::Sampler,
        stage,
        [](D3DResourceBinding& binding, Resource* resource, std::uint32_t slot, long stageFlags) -> bool
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
        case StageFlags::VertexStage:           segmentationHeader_.numVSSamplerSegments = numSegments; break;
        case StageFlags::TessControlStage:      segmentationHeader_.numHSSamplerSegments = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentationHeader_.numDSSamplerSegments = numSegments; break;
        case StageFlags::GeometryStage:         segmentationHeader_.numGSSamplerSegments = numSegments; break;
        case StageFlags::FragmentStage:         segmentationHeader_.numPSSamplerSegments = numSegments; break;
        case StageFlags::ComputeStage:          segmentationHeader_.numCSSamplerSegments = numSegments; break;
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

void D3D11ResourceHeap::BuildAllSegmentsType1(const std::vector<D3DResourceBinding>& resourceBindings, long affectedStage, std::uint8_t& numSegments)
{
    BuildAllSegments(
        resourceBindings,
        std::bind(&D3D11ResourceHeap::BuildSegment1, this, std::placeholders::_1, std::placeholders::_2),
        affectedStage,
        numSegments
    );
}

void D3D11ResourceHeap::BuildAllSegmentsType2(const std::vector<D3DResourceBinding>& resourceBindings, long affectedStage, std::uint8_t& numSegments)
{
    BuildAllSegments(
        resourceBindings,
        std::bind(&D3D11ResourceHeap::BuildSegment2, this, std::placeholders::_1, std::placeholders::_2),
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

    /* Write first part of segment body (of type <GLTextureTarget>) */
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

template <typename T>
T* const* CastToD3DViews(const std::int8_t* byteAlignedBuffer)
{
    return reinterpret_cast<T* const*>(byteAlignedBuffer + sizeof(D3DResourceViewHeapSegment1));
}

static ID3D11Buffer* const* CastToD3D11Buffers(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11Buffer>(byteAlignedBuffer);
}

static ID3D11SamplerState* const* CastToD3D11SamplerStates(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11SamplerState>(byteAlignedBuffer);
}

static ID3D11ShaderResourceView* const* CastToD3D11ShaderResourceViews(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11ShaderResourceView>(byteAlignedBuffer);
}

static ID3D11UnorderedAccessView* const* CastToD3D11UnorderedAccessView(const std::int8_t* byteAlignedBuffer)
{
    return CastToD3DViews<ID3D11UnorderedAccessView>(byteAlignedBuffer);
}

void D3D11ResourceHeap::StoreResourceUsage()
{
    /* Store information for which stages any resources have been specified */
    #define LLGL_STORE_STAGE_RESOURCE_USAGE(STAGE)                              \
        if ( segmentationHeader_.num##STAGE##SamplerSegments            > 0 ||  \
             segmentationHeader_.num##STAGE##ConstantBufferSegments     > 0 ||  \
             segmentationHeader_.num##STAGE##ShaderResourceViewSegments > 0 )   \
        {                                                                       \
            segmentationHeader_.has##STAGE##Resources = 1;                      \
        }

    LLGL_STORE_STAGE_RESOURCE_USAGE(VS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(HS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(DS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(GS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(PS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(CS);

    #undef LLGL_STORE_STAGE_RESOURCE_USAGE

    /* Extend the determination for unordered access views */
    if (segmentationHeader_.numPSUnorderedAccessViewSegments > 0)
        segmentationHeader_.hasPSResources = 1;
    if (segmentationHeader_.numCSUnorderedAccessViewSegments > 0)
        segmentationHeader_.hasCSResources = 1;
}

void D3D11ResourceHeap::BindVSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numVSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numVSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numVSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->VSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindHSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numHSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numHSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numHSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->HSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindDSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numDSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numDSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numDSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->DSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindGSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numGSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numGSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numGSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->GSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindPSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->PSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all unordered access views (UAV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numPSUnorderedAccessViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment2*>(byteAlignedBuffer);
        context->OMSetRenderTargetsAndUnorderedAccessViews(
            D3D11_KEEP_RENDER_TARGETS_AND_DEPTH_STENCIL,
            nullptr,
            nullptr,
            segment->startSlot,
            segment->numViews,
            reinterpret_cast<ID3D11UnorderedAccessView* const*>(byteAlignedBuffer + sizeof(D3DResourceViewHeapSegment2)),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}

void D3D11ResourceHeap::BindCSResources(ID3D11DeviceContext* context, std::int8_t*& byteAlignedBuffer)
{
    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSConstantBufferSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetConstantBuffers(segment->startSlot, segment->numViews, CastToD3D11Buffers(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSSamplerSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetSamplers(segment->startSlot, segment->numViews, CastToD3D11SamplerStates(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all shader resource views (SRV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSShaderResourceViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment1*>(byteAlignedBuffer);
        context->CSSetShaderResources(segment->startSlot, segment->numViews, CastToD3D11ShaderResourceViews(byteAlignedBuffer));
        byteAlignedBuffer += segment->segmentSize;
    }

    /* Bind all unordered access views (UAV) */
    for (std::uint8_t i = 0; i < segmentationHeader_.numCSUnorderedAccessViewSegments; ++i)
    {
        auto segment = reinterpret_cast<const D3DResourceViewHeapSegment2*>(byteAlignedBuffer);
        context->CSSetUnorderedAccessViews(
            segment->startSlot,
            segment->numViews,
            reinterpret_cast<ID3D11UnorderedAccessView* const*>(byteAlignedBuffer + sizeof(D3DResourceViewHeapSegment2)),
            reinterpret_cast<const UINT*>(byteAlignedBuffer + segment->offsetEnd0)
        );
        byteAlignedBuffer += segment->segmentSize;
    }
}


} // /namespace LLGL



// ================================================================================
