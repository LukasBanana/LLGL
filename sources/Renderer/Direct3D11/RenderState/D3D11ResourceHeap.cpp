/*
 * D3D11ResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11ResourceHeap.h"
#include "D3D11PipelineLayout.h"
#include "D3D11ResourceType.h"
#include "D3D11BindingTable.h"
#include "../Buffer/D3D11Buffer.h"
#include "../Buffer/D3D11BufferWithRV.h"
#include "../Texture/D3D11Sampler.h"
#include "../Texture/D3D11Texture.h"
#include "../D3D11Types.h"
#include "../../CheckedCast.h"
#include "../../BindingDescriptorIterator.h"
#include "../../ResourceUtils.h"
#include "../../TextureUtils.h"
#include "../../BufferUtils.h"
#include "../../StaticAssertions.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>
#include <string.h>


namespace LLGL
{


/*
 * Internal structures
 */

/*

The internal buffer of D3D11ResourceHeap is tightly packed which stores all segments of binding points consecutively.
Here is an illustration of the buffer layout for one Texture resouce (at binding point 4) and two StorageBuffer resources (at binding points 5 and 6)
on a 32-bit build, both for the fragment shader stage only:

Offset      Attribute                              Value   Description                                         Segment
--------------------------------------------------------------------------------------------------------------------------------------------
0x00000000  D3DResourceHeapSegment::size              16   Size of this segment                                \
0x00000004  D3DResourceHeapSegment::startSlot          4   First binding point                                  |
0x00000008  D3DResourceHeapSegment::numSlots           1   Number of binding points                             |-- Texture/Buffer SRV segment
0x0000000C  srv[0]                                 <ptr>   1st ID3D11ShaderResourceView for texture             |
0x00000010  srvIndex[0]                                0   Index to subresource SRV list                       /
0x00000012  D3DResourceHeapSegment::size              20   Size of this segment                                \
0x00000016  D3DResourceHeapSegment::data1Offset       20   Relative offset to initialCount[0] (at 0x00000028)   |
0x0000001A  D3DResourceHeapSegment::startSlot          5   First binding point                                  |
0x0000001E  D3DResourceHeapSegment::numSlots           2   Number of binding points                             |
0x00000022  uav[0]                                 <ptr>   1st ID3D11UnorderedAccessView for storage buffer     |
0x00000026  uav[1]                                 <ptr>   2nd ID3D11UnorderedAccessView for storage buffer     |-- Texture/Buffer UAV segment
0x0000002A  initialCount[0]                            0   1st initial count                                    |
0x0000002E  initialCount[1]                            0   2nd initial count                                    |
0x00000030  uavIndex[0]                                0   Index to subresource UAV list                        |
0x00000032  uavIndex[1]                                1   Index to subresource UAV list                       /

*/

// Resource segment flags. Bits can be shared as they are only used for certain segment types.
enum D3DResourceFlags : std::uint32_t
{
    D3DResourceFlags_HasBufferRange  = (1 << 0),
};

// Resource view heap (RVH) segment structure with up to three dynamic sub-buffers.
struct alignas(sizeof(std::uintptr_t)) D3DResourceHeapSegment
{
    std::uint32_t   size            : 28;
    std::uint32_t   flags           :  1; // D3DResourceFlags
    D3DResourceType type            :  3;
    UINT            startSlot       : 16;
    UINT            numViews        : 16;
    std::uint32_t   data1Offset     : 16;
    std::uint32_t   data2Offset     : 16;
    std::uint32_t   locatorOffset   : 16;
    std::uint32_t   rangeOffset     : 16;
};

LLGL_ASSERT_POD_TYPE(D3DResourceHeapSegment);

// Size (in bytes) of each constant register in a constant buffer
static constexpr UINT g_cbufferRegisterSize = 16;

#define D3DRESOURCEHEAP_SEGMENT(PTR)                    reinterpret_cast<D3DResourceHeapSegment*>(PTR)
#define D3DRESOURCEHEAP_CONST_SEGMENT(PTR)              reinterpret_cast<const D3DResourceHeapSegment*>(PTR)
#define D3DRESOURCEHEAP_DATA0(PTR, TYPE)                reinterpret_cast<TYPE*>((PTR) + sizeof(D3DResourceHeapSegment))
#define D3DRESOURCEHEAP_DATA0_COMPTR_CONST(PTR, TYPE)   reinterpret_cast<TYPE* const*>((PTR) + sizeof(D3DResourceHeapSegment))
#define D3DRESOURCEHEAP_DATA0_CBV_CONST(PTR)            D3DRESOURCEHEAP_DATA0_COMPTR_CONST(PTR, ID3D11Buffer)
#define D3DRESOURCEHEAP_DATA0_SRV_CONST(PTR)            D3DRESOURCEHEAP_DATA0_COMPTR_CONST(PTR, ID3D11ShaderResourceView)
#define D3DRESOURCEHEAP_DATA0_UAV_CONST(PTR)            D3DRESOURCEHEAP_DATA0_COMPTR_CONST(PTR, ID3D11UnorderedAccessView)
#define D3DRESOURCEHEAP_DATA0_SAMPLER_CONST(PTR)        D3DRESOURCEHEAP_DATA0_COMPTR_CONST(PTR, ID3D11SamplerState)
#define D3DRESOURCEHEAP_DATA0_COMPTR(PTR, TYPE)         reinterpret_cast<TYPE**>((PTR) + sizeof(D3DResourceHeapSegment))
#define D3DRESOURCEHEAP_DATA0_CBV(PTR)                  D3DRESOURCEHEAP_DATA0_COMPTR(PTR, ID3D11Buffer)
#define D3DRESOURCEHEAP_DATA0_SRV(PTR)                  D3DRESOURCEHEAP_DATA0_COMPTR(PTR, ID3D11ShaderResourceView)
#define D3DRESOURCEHEAP_DATA0_UAV(PTR)                  D3DRESOURCEHEAP_DATA0_COMPTR(PTR, ID3D11UnorderedAccessView)
#define D3DRESOURCEHEAP_DATA0_SAMPLER(PTR)              D3DRESOURCEHEAP_DATA0_COMPTR(PTR, ID3D11SamplerState)
#define D3DRESOURCEHEAP_DATA1(PTR, TYPE)                reinterpret_cast<TYPE*>((PTR) + D3DRESOURCEHEAP_CONST_SEGMENT(PTR)->data1Offset)
#define D3DRESOURCEHEAP_DATA2(PTR, TYPE)                reinterpret_cast<TYPE*>((PTR) + D3DRESOURCEHEAP_CONST_SEGMENT(PTR)->data2Offset)
#define D3DRESOURCEHEAP_LOCATOR(PTR)                    reinterpret_cast<D3D11BindingLocator**>((PTR) + D3DRESOURCEHEAP_SEGMENT(PTR)->locatorOffset)
#define D3DRESOURCEHEAP_RANGE(PTR)                      reinterpret_cast<D3D11SubresourceRange*>((PTR) + D3DRESOURCEHEAP_SEGMENT(PTR)->rangeOffset)
#define D3DRESOURCEHEAP_LOCATOR_CONST(PTR)              reinterpret_cast<D3D11BindingLocator* const*>((PTR) + D3DRESOURCEHEAP_CONST_SEGMENT(PTR)->locatorOffset)
#define D3DRESOURCEHEAP_RANGE_CONST(PTR)                reinterpret_cast<const D3D11SubresourceRange*>((PTR) + D3DRESOURCEHEAP_CONST_SEGMENT(PTR)->rangeOffset)


/*
 * D3D11ResourceHeap class
 */

D3D11ResourceHeap::D3D11ResourceHeap(
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
{
    /* Get pipeline layout object */
    auto* pipelineLayoutD3D = LLGL_CAST(D3D11PipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutD3D)
        LLGL_TRAP("failed to create resource heap due to missing pipeline layout");

    /* Get and validate number of bindings and resource views */
    const auto&         bindings            = pipelineLayoutD3D->GetHeapBindings();
    const std::uint32_t numBindings         = static_cast<std::uint32_t>(bindings.size());
    const std::uint32_t numResourceViews    = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    /* Allocate array to map binding index to descriptor index */
    bindingMap_.resize(numBindings);

    /* Build buffer segments (stage after stage, so the internal buffer is constructed in the correct order) */
    BindingDescriptorIterator bindingIter{ bindings };

    /* Build resource view segments for GRAPHICS stages in current descriptor set */
    AllocStageSegments(bindingIter, StageFlags::VertexStage);
    AllocStageSegments(bindingIter, StageFlags::TessControlStage);
    AllocStageSegments(bindingIter, StageFlags::TessEvaluationStage);
    AllocStageSegments(bindingIter, StageFlags::GeometryStage);
    AllocStageSegments(bindingIter, StageFlags::FragmentStage);

    /* Store offset to compute stage segments */
    heapOffsetCS_ = static_cast<std::uint32_t>(heap_.Size());
    AllocStageSegments(bindingIter, StageFlags::ComputeStage);

    /* Store resource usage bits in segmentation header */
    CacheResourceUsage();

    /* Finalize segments in buffer */
    const std::uint32_t numSegmentSets = (numResourceViews / numBindings);
    heap_.FinalizeSegments(numSegmentSets);

    /* Write initial resource views */
    if (!initialResourceViews.empty())
        WriteResourceViews(0, initialResourceViews);
}

std::uint32_t D3D11ResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(heap_.NumSets());
}

std::uint32_t D3D11ResourceHeap::WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    /* Quit if there's nothing to do */
    if (resourceViews.empty())
        return 0;

    const std::uint32_t numSets         = GetNumDescriptorSets();
    const std::uint32_t numBindings     = static_cast<std::uint32_t>(bindingMap_.size());
    const std::uint32_t numDescriptors  = numSets * numBindings;

    /* Silently quit on out of bounds; debug layer must report these errors */
    if (firstDescriptor >= numDescriptors)
        return 0;
    if (firstDescriptor + resourceViews.size() > numDescriptors)
        return 0;

    /* Write each resource view into respective segment */
    std::uint32_t numWritten = 0;

    for (const ResourceViewDescriptor& desc : resourceViews)
    {
        /* Skip over empty resource descriptors */
        if (desc.resource == nullptr)
            continue;

        /* Get binding information and heap start for descriptor set */
        const BindingSegmentLocation& binding = bindingMap_[firstDescriptor % numBindings];

        const std::uint32_t descriptorSet = firstDescriptor / numBindings;
        char* heapStartPtr = heap_.SegmentData(descriptorSet);

        /* Get SRV and UAV objects for textures and buffers */
        ID3D11ShaderResourceView* srv = nullptr;
        ID3D11UnorderedAccessView* uav = nullptr;
        D3DSubresourceLocator subresourceLocator;
        SubresourceIndexContext subresourceContext;

        if (binding.type == D3DResourceType_SRV)
        {
            srv = GetOrCreateSRV(desc, subresourceLocator);
            if (srv == nullptr)
                continue;
        }
        else if (binding.type == D3DResourceType_UAV)
        {
            uav = GetOrCreateUAV(desc, subresourceLocator);
            if (uav == nullptr)
                continue;
        }

        subresourceContext.newIndex = subresourceLocator.index;

        /* Write descriptor into respective heap segment for each affected shader stage */
        for_range(stage, static_cast<int>(D3DShaderStage_Count))
        {
            const std::uint32_t offset = binding.stages[stage].segmentOffset;
            if (offset == BindingSegmentLocation::invalidOffset)
                continue;

            char*                           heapPtr = heapStartPtr + offset;
            const D3DResourceHeapSegment*   segment = D3DRESOURCEHEAP_CONST_SEGMENT(heapPtr);
            const std::uint32_t             index   = binding.stages[stage].descriptorIndex;

            switch (segment->type)
            {
                case D3DResourceType_CBV:
                    WriteResourceViewCBV(desc, heapPtr, index);
                    break;
                case D3DResourceType_SRV:
                    WriteResourceViewSRV(srv, subresourceLocator.locator, subresourceLocator.range, heapPtr, index, subresourceContext);
                    break;
                case D3DResourceType_UAV:
                    WriteResourceViewUAV(uav, subresourceLocator.locator, subresourceLocator.range, heapPtr, index, static_cast<UINT>(desc.initialCount), subresourceContext);
                    break;
                case D3DResourceType_Sampler:
                    WriteResourceViewSampler(desc, heapPtr, index);
                    break;
            }
        }

        /* Delete old subresource and move new subresource (if created) */
        if (binding.type == D3DResourceType_SRV)
            D3D11ResourceHeap::GarbageCollectSubresource(subresourceSRVs_, subresourceContext);
        else if (binding.type == D3DResourceType_UAV)
            D3D11ResourceHeap::GarbageCollectSubresource(subresourceUAVs_, subresourceContext);

        ++numWritten;
        ++firstDescriptor;
    }

    return numWritten;
}

void D3D11ResourceHeap::BindForGraphicsPipeline(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet)
{
    /* Bind resource views to the graphics shader stages */
    const char* heapPtr = heap_.SegmentData(descriptorSet);
    if (segmentation_.hasResourcesVS)
        heapPtr = BindVSResources(context, bindingTable, heapPtr);
    if (segmentation_.hasResourcesHS)
        heapPtr = BindHSResources(context, bindingTable, heapPtr);
    if (segmentation_.hasResourcesDS)
        heapPtr = BindDSResources(context, bindingTable, heapPtr);
    if (segmentation_.hasResourcesGS)
        heapPtr = BindGSResources(context, bindingTable, heapPtr);
    if (segmentation_.hasResourcesPS)
        heapPtr = BindPSResources(context, bindingTable, heapPtr);
}

void D3D11ResourceHeap::BindForComputePipeline(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet)
{
    /* Bind resource views to the compute shader stage */
    const char* heapPtr = heap_.SegmentData(descriptorSet) + heapOffsetCS_;
    if (segmentation_.hasResourcesCS)
        BindCSResources(context, bindingTable, heapPtr);
}

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1

void D3D11ResourceHeap::BindForGraphicsPipeline1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet)
{
    /* Bind resource views and constant-buffer ranges to the graphics shader stages */
    const char* heapPtr = heap_.SegmentData(descriptorSet);
    if (segmentation_.hasResourcesVS)
        heapPtr = BindVSResources1(context1, bindingTable, heapPtr);
    if (segmentation_.hasResourcesHS)
        heapPtr = BindHSResources1(context1, bindingTable, heapPtr);
    if (segmentation_.hasResourcesDS)
        heapPtr = BindDSResources1(context1, bindingTable, heapPtr);
    if (segmentation_.hasResourcesGS)
        heapPtr = BindGSResources1(context1, bindingTable, heapPtr);
    if (segmentation_.hasResourcesPS)
        heapPtr = BindPSResources1(context1, bindingTable, heapPtr);
}

void D3D11ResourceHeap::BindForComputePipeline1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, std::uint32_t descriptorSet)
{
    /* Bind resource views and constant-buffer ranges to the compute shader stage */
    const char* heapPtr = heap_.SegmentData(descriptorSet) + heapOffsetCS_;
    if (segmentation_.hasResourcesCS)
        BindCSResources1(context1, bindingTable, heapPtr);
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL


/*
 * ======= Private: =======
 */

#define BIND_SEGMENT_ALLOCATOR(FUNC, STAGE, TYPE, ...)          \
    std::bind(                                                  \
        &FUNC,                                                  \
        this,                                                   \
        D3D11ResourceHeap::StageFlagsToD3DShaderStage(STAGE),   \
        TYPE,                                                   \
        std::placeholders::_1, std::placeholders::_2,           \
        __VA_ARGS__                                             \
    )

std::vector<D3D11ResourceHeap::D3DResourceBinding> D3D11ResourceHeap::FilterAndSortD3DBindingSlots(
    BindingDescriptorIterator&                  bindingIter,
    const std::initializer_list<ResourceType>&  resourceTypes,
    long                                        resourceBindFlags,
    long                                        affectedStage)
{
    /* Collect all binding points of the specified resource types */
    std::vector<D3DResourceBinding> resourceBindings;
    resourceBindings.reserve(bindingIter.GetCount());

    for (ResourceType type : resourceTypes)
    {
        bindingIter.Reset(type, resourceBindFlags, affectedStage);
        for (std::size_t index = 0; const BindingDescriptor* bindingDesc = bindingIter.Next(&index);)
        {
            D3DResourceBinding resourceBinding;
            {
                resourceBinding.slot    = bindingDesc->slot.index;
                resourceBinding.stages  = affectedStage;
                resourceBinding.index   = index;
            }
            resourceBindings.push_back(resourceBinding);
        }
    }

    /* Sort resources by slot index */
    std::sort(
        resourceBindings.begin(),
        resourceBindings.end(),
        [](const D3DResourceBinding& lhs, const D3DResourceBinding& rhs)
        {
            return (lhs.slot < rhs.slot);
        }
    );

    return resourceBindings;
}

void D3D11ResourceHeap::AllocStageSegments(BindingDescriptorIterator& bindingIter, long stage)
{
    AllocConstantBufferSegments(bindingIter, stage);
    AllocSamplerSegments(bindingIter, stage);
    AllocShaderResourceViewSegments(bindingIter, stage);

    /* UAVs must be collected for all graphics stages but can only be bound to the pixel stage (and compute stage) */
    switch (stage)
    {
        case StageFlags::FragmentStage:
            AllocUnorderedAccessViewSegments(bindingIter, stage, StageFlags::AllGraphicsStages);
            break;
        case StageFlags::ComputeStage:
            AllocUnorderedAccessViewSegments(bindingIter, stage, StageFlags::ComputeStage);
            break;
    }
}

void D3D11ResourceHeap::AllocConstantBufferSegments(BindingDescriptorIterator& bindingIter, long stage)
{
    /* Collect all constant buffer views */
    auto cbvBindingSlots = D3D11ResourceHeap::FilterAndSortD3DBindingSlots(
        bindingIter,
        { ResourceType::Buffer },
        BindFlags::ConstantBuffer,
        stage
    );

    /* Build all resource segments for ranged and unragned CBVs */
    const std::uint32_t numSegments = D3D11ResourceHeap::ConsolidateSegments(
        cbvBindingSlots,
        BIND_SEGMENT_ALLOCATOR(
            D3D11ResourceHeap::Alloc3PartSegment, stage, D3DResourceType_CBV,
            sizeof(ID3D11Buffer*), sizeof(UINT), sizeof(UINT),
            /*payload2Initial:*/ 0
        )
    );

    /* Store number of segments for stage */
    switch (stage)
    {
        case StageFlags::VertexStage:           segmentation_.numCBVSegmentsVS = numSegments; break;
        case StageFlags::TessControlStage:      segmentation_.numCBVSegmentsHS = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentation_.numCBVSegmentsDS = numSegments; break;
        case StageFlags::GeometryStage:         segmentation_.numCBVSegmentsGS = numSegments; break;
        case StageFlags::FragmentStage:         segmentation_.numCBVSegmentsPS = numSegments; break;
        case StageFlags::ComputeStage:          segmentation_.numCBVSegmentsCS = numSegments; break;
    }
}

void D3D11ResourceHeap::AllocShaderResourceViewSegments(BindingDescriptorIterator& bindingIter, long stage)
{
    /* Collect all shader resource view (SRV) slots for sampled buffers and textures */
    auto srvBindingSlots = D3D11ResourceHeap::FilterAndSortD3DBindingSlots(
        bindingIter,
        { ResourceType::Buffer, ResourceType::Texture },
        BindFlags::Sampled,
        stage
    );

    /* Build all resource segments for SRVs */
    const int initialSubresourceIndices = -1;
    const std::uint32_t numSegments = D3D11ResourceHeap::ConsolidateSegments(
        srvBindingSlots,
        BIND_SEGMENT_ALLOCATOR(
            D3D11ResourceHeap::Alloc2PartSegment, stage, D3DResourceType_SRV,
            sizeof(ID3D11ShaderResourceView*), sizeof(std::uint16_t),
            /*payload1Initial:*/ initialSubresourceIndices
        )
    );

    /* Store number of segments for stage */
    switch (stage)
    {
        case StageFlags::VertexStage:           segmentation_.numSRVSegmentsVS = numSegments; break;
        case StageFlags::TessControlStage:      segmentation_.numSRVSegmentsHS = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentation_.numSRVSegmentsDS = numSegments; break;
        case StageFlags::GeometryStage:         segmentation_.numSRVSegmentsGS = numSegments; break;
        case StageFlags::FragmentStage:         segmentation_.numSRVSegmentsPS = numSegments; break;
        case StageFlags::ComputeStage:          segmentation_.numSRVSegmentsCS = numSegments; break;
    }
}

void D3D11ResourceHeap::AllocUnorderedAccessViewSegments(BindingDescriptorIterator& bindingIter, long stage, long affectedStages)
{
    /* Collect all unordered access view (UAV) slots for storage buffers and textures */
    auto uavBindingSlots = D3D11ResourceHeap::FilterAndSortD3DBindingSlots(
        bindingIter,
        { ResourceType::Buffer, ResourceType::Texture },
        BindFlags::Storage,
        affectedStages
    );

    /* Build all resource segments for UAVs */
    const int initialSubresourceIndices = -1;
    const std::uint32_t numSegments = D3D11ResourceHeap::ConsolidateSegments(
        uavBindingSlots,
        BIND_SEGMENT_ALLOCATOR(
            D3D11ResourceHeap::Alloc3PartSegment, stage, D3DResourceType_UAV,
            sizeof(ID3D11UnorderedAccessView*), sizeof(UINT), sizeof(std::uint16_t),
            /*payload2Initial:*/ initialSubresourceIndices
        )
    );

    /* Store number of segments for stage */
    switch (affectedStages)
    {
        case StageFlags::ComputeStage:  segmentation_.numUAVSegmentsCS = numSegments; break;
        default:                        segmentation_.numUAVSegmentsPS = numSegments; break;
    }
}

void D3D11ResourceHeap::AllocSamplerSegments(BindingDescriptorIterator& bindingIter, long stage)
{
    /* Collect all sampler-state slots */
    auto samplerBindingSlots = D3D11ResourceHeap::FilterAndSortD3DBindingSlots(
        bindingIter,
        { ResourceType::Sampler },
        0,
        stage
    );

    /* Build all resource segments for samplers */
    const std::uint32_t numSegments = D3D11ResourceHeap::ConsolidateSegments(
        samplerBindingSlots,
        BIND_SEGMENT_ALLOCATOR(
            D3D11ResourceHeap::Alloc1PartSegment, stage, D3DResourceType_Sampler,
            sizeof(ID3D11SamplerState*)
        )
    );

    /* Store number of segments for stage */
    switch (stage)
    {
        case StageFlags::VertexStage:           segmentation_.numSamplerSegmentsVS = numSegments; break;
        case StageFlags::TessControlStage:      segmentation_.numSamplerSegmentsHS = numSegments; break;
        case StageFlags::TessEvaluationStage:   segmentation_.numSamplerSegmentsDS = numSegments; break;
        case StageFlags::GeometryStage:         segmentation_.numSamplerSegmentsGS = numSegments; break;
        case StageFlags::FragmentStage:         segmentation_.numSamplerSegmentsPS = numSegments; break;
        case StageFlags::ComputeStage:          segmentation_.numSamplerSegmentsCS = numSegments; break;
    }
}

void D3D11ResourceHeap::Alloc1PartSegment(
    D3DShaderStage              stage,
    D3DResourceType             type,
    const D3DResourceBinding*   first,
    UINT                        count,
    std::size_t                 payload0Stride)
{
    /* Write binding map entries */
    WriteBindingMappings(stage, type, first, count);

    /* Allocate space for segment */
    const std::uint32_t payloadLocatorOffset    = static_cast<std::uint32_t>(payload0Stride * count);
    const std::uint32_t payloadRangeOffset      = static_cast<std::uint32_t>(sizeof(D3D11BindingLocator*) * count + payloadLocatorOffset);
    const std::uint32_t payloadSize             = static_cast<std::uint32_t>(sizeof(D3D11SubresourceRange) * count + payloadRangeOffset);
    auto                segmentAlloc            = heap_.AllocSegment<D3DResourceHeapSegment>(payloadSize);

    /* Write segment header */
    D3DResourceHeapSegment* header = segmentAlloc.Header();
    {
        header->size            = segmentAlloc.Size();
        header->type            = type;
        header->startSlot       = first->slot;
        header->numViews        = count;
        header->data1Offset     = 0;
        header->data2Offset     = 0;
        header->locatorOffset   = segmentAlloc.PayloadOffset() + payloadLocatorOffset;
        header->rangeOffset     = segmentAlloc.PayloadOffset() + payloadRangeOffset;
    }
}

void D3D11ResourceHeap::Alloc2PartSegment(
    D3DShaderStage              stage,
    D3DResourceType             type,
    const D3DResourceBinding*   first,
    UINT                        count,
    std::size_t                 payload0Stride,
    std::size_t                 payload1Stride,
    int                         payload1Initial)
{
    /* Write binding map entries */
    WriteBindingMappings(stage, type, first, count);

    /* Allocate space for segment */
    const std::uint32_t payloadData1Offset      = static_cast<std::uint32_t>(payload0Stride * count);
    const std::uint32_t payloadLocatorOffset    = static_cast<std::uint32_t>(payload1Stride * count + payloadData1Offset);
    const std::uint32_t payloadRangeOffset      = static_cast<std::uint32_t>(sizeof(D3D11BindingLocator*) * count + payloadLocatorOffset);
    const std::uint32_t payloadSize             = static_cast<std::uint32_t>(sizeof(D3D11SubresourceRange) * count + payloadRangeOffset);
    auto                segmentAlloc            = heap_.AllocSegment<D3DResourceHeapSegment>(payloadSize);

    /* Write segment header */
    D3DResourceHeapSegment* header = segmentAlloc.Header();
    {
        header->size            = segmentAlloc.Size();
        header->type            = type;
        header->startSlot       = first->slot;
        header->numViews        = count;
        header->data1Offset     = segmentAlloc.PayloadOffset() + payloadData1Offset;
        header->data2Offset     = 0;
        header->locatorOffset   = segmentAlloc.PayloadOffset() + payloadLocatorOffset;
        header->rangeOffset     = segmentAlloc.PayloadOffset() + payloadRangeOffset;
    }

    /* Initialize payload data if specified */
    if (payload1Initial != 0)
        ::memset(segmentAlloc.Payload<char>(payloadData1Offset), payload1Initial, payload1Stride * count);
}

void D3D11ResourceHeap::Alloc3PartSegment(
    D3DShaderStage              stage,
    D3DResourceType             type,
    const D3DResourceBinding*   first,
    UINT                        count,
    std::size_t                 payload0Stride,
    std::size_t                 payload1Stride,
    std::size_t                 payload2Stride,
    int                         payload2Initial)
{
    /* Write binding map entries */
    WriteBindingMappings(stage, type, first, count);

    /* Allocate space for segment */
    const std::uint32_t payloadData1Offset      = static_cast<std::uint32_t>(payload0Stride * count);
    const std::uint32_t payloadData2Offset      = static_cast<std::uint32_t>(payload1Stride * count + payloadData1Offset);
    const std::uint32_t payloadLocatorOffset    = static_cast<std::uint32_t>(payload2Stride * count + payloadData2Offset);
    const std::uint32_t payloadRangeOffset      = static_cast<std::uint32_t>(sizeof(D3D11BindingLocator*) * count + payloadLocatorOffset);
    const std::uint32_t payloadSize             = static_cast<std::uint32_t>(sizeof(D3D11SubresourceRange) * count + payloadRangeOffset);
    auto                segmentAlloc            = heap_.AllocSegment<D3DResourceHeapSegment>(payloadSize);

    /* Write segment header */
    D3DResourceHeapSegment* header = segmentAlloc.Header();
    {
        header->size            = segmentAlloc.Size();
        header->type            = type;
        header->startSlot       = first->slot;
        header->numViews        = count;
        header->data1Offset     = segmentAlloc.PayloadOffset() + payloadData1Offset;
        header->data2Offset     = segmentAlloc.PayloadOffset() + payloadData2Offset;
        header->locatorOffset   = segmentAlloc.PayloadOffset() + payloadLocatorOffset;
        header->rangeOffset     = segmentAlloc.PayloadOffset() + payloadRangeOffset;
    }

    /* Initialize payload data if specified */
    if (payload2Initial != 0)
        ::memset(segmentAlloc.Payload<char>(payloadData2Offset), payload2Initial, payload2Stride * count);
}

void D3D11ResourceHeap::WriteBindingMappings(D3DShaderStage stage, D3DResourceType type, const D3DResourceBinding* first, UINT count)
{
    for_range(i, count)
    {
        LLGL_ASSERT(first[i].index < bindingMap_.size());
        BindingSegmentLocation& mapping = bindingMap_[first[i].index];
        mapping.stages[stage].segmentOffset     = static_cast<std::uint32_t>(heap_.Size());
        mapping.stages[stage].descriptorIndex   = i; // Index within the segment
        mapping.type                            = type;
    }
}

void D3D11ResourceHeap::CacheResourceUsage()
{
    /* Store information for which stages any resources have been specified */
    #define LLGL_STORE_STAGE_RESOURCE_USAGE(STAGE)          \
        if ( segmentation_.numSamplerSegments##STAGE > 0 || \
             segmentation_.numCBVSegments##STAGE     > 0 || \
             segmentation_.numSRVSegments##STAGE     > 0 )  \
        {                                                   \
            segmentation_.hasResources##STAGE = 1;          \
        }

    LLGL_STORE_STAGE_RESOURCE_USAGE(VS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(HS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(DS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(GS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(PS);
    LLGL_STORE_STAGE_RESOURCE_USAGE(CS);

    #undef LLGL_STORE_STAGE_RESOURCE_USAGE

    /* Extend the determination for unordered access views */
    if (segmentation_.numUAVSegmentsPS > 0)
        segmentation_.hasResourcesPS = 1;
    if (segmentation_.numUAVSegmentsCS > 0)
        segmentation_.hasResourcesCS = 1;
}

#define D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1(CONTEXT, PTR, STAGE)      \
    for_range(i, segmentation_.numCBVSegments##STAGE)                   \
    {                                                                   \
        auto* segment = D3DRESOURCEHEAP_CONST_SEGMENT(PTR);             \
        if ((segment->flags & D3DResourceFlags_HasBufferRange) != 0)    \
        {                                                               \
            CONTEXT->STAGE##SetConstantBuffers1(                        \
                segment->startSlot,                                     \
                segment->numViews,                                      \
                D3DRESOURCEHEAP_DATA0_CBV_CONST(PTR),                   \
                D3DRESOURCEHEAP_DATA1(PTR, const UINT),                 \
                D3DRESOURCEHEAP_DATA2(PTR, const UINT)                  \
            );                                                          \
        }                                                               \
        else                                                            \
        {                                                               \
            CONTEXT->STAGE##SetConstantBuffers(                         \
                segment->startSlot,                                     \
                segment->numViews,                                      \
                D3DRESOURCEHEAP_DATA0_CBV_CONST(PTR)                    \
            );                                                          \
        }                                                               \
        PTR += segment->size;                                           \
    }

#define D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_PS(TABLE, PTR)    \
    for_range(i, segmentation_.numUAVSegmentsPS)                    \
    {                                                               \
        auto* segment = D3DRESOURCEHEAP_CONST_SEGMENT(PTR);         \
        TABLE.SetUnorderedAccessViews(                              \
            segment->startSlot,                                     \
            segment->numViews,                                      \
            D3DRESOURCEHEAP_DATA0_UAV_CONST(PTR),                   \
            D3DRESOURCEHEAP_DATA1(PTR, const UINT),                 \
            D3DRESOURCEHEAP_LOCATOR_CONST(PTR),                     \
            D3DRESOURCEHEAP_RANGE_CONST(PTR),                       \
            StageFlags::AllGraphicsStages                           \
        );                                                          \
        PTR += segment->size;                                       \
    }

#define D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_CS(TABLE, PTR)    \
    for_range(i, segmentation_.numUAVSegmentsCS)                    \
    {                                                               \
        auto* segment = D3DRESOURCEHEAP_CONST_SEGMENT(PTR);         \
        TABLE.SetUnorderedAccessViews(                              \
            segment->startSlot,                                     \
            segment->numViews,                                      \
            D3DRESOURCEHEAP_DATA0_UAV_CONST(PTR),                   \
            D3DRESOURCEHEAP_DATA1(PTR, const UINT),                 \
            D3DRESOURCEHEAP_LOCATOR_CONST(PTR),                     \
            D3DRESOURCEHEAP_RANGE_CONST(PTR),                       \
            StageFlags::ComputeStage                                \
        );                                                          \
        PTR += segment->size;                                       \
    }

#define D3DRESOURCEHEAP_BIND_RESOURCE_GENERIC(CONTEXT, PTR, COUNT, FUNC, DATA)  \
    for_range(i, segmentation_.COUNT)                                           \
    {                                                                           \
        auto* segment = D3DRESOURCEHEAP_CONST_SEGMENT(PTR);                     \
        CONTEXT->FUNC(segment->startSlot, segment->numViews, DATA(PTR));        \
        heapPtr += segment->size;                                               \
    }

#define D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS(CONTEXT, PTR, STAGE) \
    D3DRESOURCEHEAP_BIND_RESOURCE_GENERIC(CONTEXT, PTR, numCBVSegments##STAGE, STAGE##SetConstantBuffers, D3DRESOURCEHEAP_DATA0_CBV_CONST)

#define D3DRESOURCEHEAP_BIND_SAMPLERS(CONTEXT, PTR, STAGE) \
    D3DRESOURCEHEAP_BIND_RESOURCE_GENERIC(CONTEXT, PTR, numSamplerSegments##STAGE, STAGE##SetSamplers, D3DRESOURCEHEAP_DATA0_SAMPLER_CONST)

#define D3DRESOURCEHEAP_BIND_SHADERRESOURCES(TABLE, PTR, STAGE, FLAGS)  \
    for_range(i, segmentation_.numSRVSegments##STAGE)                   \
    {                                                                   \
        auto* segment = D3DRESOURCEHEAP_CONST_SEGMENT(PTR);             \
        TABLE.SetShaderResourceViews(                                   \
            segment->startSlot,                                         \
            segment->numViews,                                          \
            D3DRESOURCEHEAP_DATA0_SRV_CONST(PTR),                       \
            D3DRESOURCEHEAP_LOCATOR_CONST(PTR),                         \
            D3DRESOURCEHEAP_RANGE_CONST(PTR),                           \
            FLAGS                                                       \
        );                                                              \
        heapPtr += segment->size;                                       \
    }

const char* D3D11ResourceHeap::BindVSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all vertex-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS(context, heapPtr, VS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context, heapPtr, VS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, VS, StageFlags::VertexStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindHSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all hull-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS(context, heapPtr, HS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context, heapPtr, HS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, HS, StageFlags::TessControlStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindDSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all domain-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS(context, heapPtr, DS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context, heapPtr, DS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, DS, StageFlags::TessEvaluationStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindGSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all geometry-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS(context, heapPtr, GS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context, heapPtr, GS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, GS, StageFlags::GeometryStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindPSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all pixel-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS(context, heapPtr, PS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context, heapPtr, PS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, PS, StageFlags::FragmentStage);
    D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_PS(bindingTable, heapPtr);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindCSResources(ID3D11DeviceContext* context, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all pixel-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS(context, heapPtr, CS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context, heapPtr, CS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, CS, StageFlags::ComputeStage);
    D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_CS(bindingTable, heapPtr);
    return heapPtr;
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

const char* D3D11ResourceHeap::BindVSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all vertex-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1(context1, heapPtr, VS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context1, heapPtr, VS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, VS, StageFlags::VertexStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindHSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all hull-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1(context1, heapPtr, HS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context1, heapPtr, HS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, HS, StageFlags::TessControlStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindDSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all domain-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1(context1, heapPtr, DS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context1, heapPtr, DS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, DS, StageFlags::TessEvaluationStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindGSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all geometry-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1(context1, heapPtr, GS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context1, heapPtr, GS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, GS, StageFlags::GeometryStage);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindPSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all pixel-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1(context1, heapPtr, PS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context1, heapPtr, PS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, PS, StageFlags::FragmentStage);
    D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_PS(bindingTable, heapPtr);
    return heapPtr;
}

const char* D3D11ResourceHeap::BindCSResources1(ID3D11DeviceContext1* context1, D3D11BindingTable& bindingTable, const char* heapPtr)
{
    /* Bind all pixel-stage resources: constant buffers, samplers, shader resource views (SRVs) */
    D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1(context1, heapPtr, CS);
    D3DRESOURCEHEAP_BIND_SAMPLERS(context1, heapPtr, CS);
    D3DRESOURCEHEAP_BIND_SHADERRESOURCES(bindingTable, heapPtr, CS, StageFlags::ComputeStage);
    D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_CS(bindingTable, heapPtr);
    return heapPtr;
}

#endif // /LLGL_D3D11_ENABLE_FEATURELEVEL

void D3D11ResourceHeap::WriteResourceViewCBV(
    const ResourceViewDescriptor&   desc,
    char*                           heapPtr,
    std::uint32_t                   index)
{
    /* Get buffer resource and its size parameter */
    auto* bufferD3D = LLGL_CAST(D3D11Buffer*, GetAsExpectedBuffer(desc.resource, BindFlags::ConstantBuffer));

    const UINT bufferSize = bufferD3D->GetSize();

    /* Write D3D ComPtr */
    D3DRESOURCEHEAP_DATA0_CBV(heapPtr)[index] = bufferD3D->GetNative();

    /* Write first constant index and number of constants */
    if (IsBufferViewEnabled(desc.bufferView))
    {
        /* If one buffer view uses a buffe range, the whole segment must be bound with ranged buffers */
        D3DRESOURCEHEAP_SEGMENT(heapPtr)->flags |= D3DResourceFlags_HasBufferRange;

        D3DRESOURCEHEAP_DATA1(heapPtr, UINT)[index] = static_cast<UINT>(desc.bufferView.offset) / g_cbufferRegisterSize;
        D3DRESOURCEHEAP_DATA2(heapPtr, UINT)[index] = static_cast<UINT>(desc.bufferView.size) / g_cbufferRegisterSize;
    }
    else
    {
        D3DRESOURCEHEAP_DATA1(heapPtr, UINT)[index] = 0;
        D3DRESOURCEHEAP_DATA2(heapPtr, UINT)[index] = bufferSize / g_cbufferRegisterSize;
    }

    //TODO: update segment if there are no more buffer ranges left
}

void D3D11ResourceHeap::WriteResourceViewSRV(
    ID3D11ShaderResourceView*       srv,
    D3D11BindingLocator*            locator,
    const D3D11SubresourceRange&    range,
    char*                           heapPtr,
    std::uint32_t                   index,
    SubresourceIndexContext&        subresourceContext)
{
    /* Write D3D ComPtr and index to intermediate SRV object */
    D3DRESOURCEHEAP_DATA0_SRV(heapPtr)[index] = srv;
    D3DRESOURCEHEAP_LOCATOR  (heapPtr)[index] = locator;
    D3DRESOURCEHEAP_RANGE    (heapPtr)[index] = range;

    /* Store new index to intermediate SRV object and collect old one */
    subresourceContext.Exchange(D3DRESOURCEHEAP_DATA1(heapPtr, std::uint16_t)[index]);
}

void D3D11ResourceHeap::WriteResourceViewUAV(
    ID3D11UnorderedAccessView*      uav,
    D3D11BindingLocator*            locator,
    const D3D11SubresourceRange&    range,
    char*                           heapPtr,
    std::uint32_t                   index,
    UINT                            initialCount,
    SubresourceIndexContext&        subresourceContext)
{
    /* Write D3D ComPtr and index to intermediate SRV object */
    D3DRESOURCEHEAP_DATA0_UAV(heapPtr      )[index] = uav;
    D3DRESOURCEHEAP_DATA1    (heapPtr, UINT)[index] = initialCount;
    D3DRESOURCEHEAP_LOCATOR  (heapPtr      )[index] = locator;
    D3DRESOURCEHEAP_RANGE    (heapPtr      )[index] = range;

    /* Store new index to intermediate SRV object and collect old one */
    subresourceContext.Exchange(D3DRESOURCEHEAP_DATA2(heapPtr, std::uint16_t)[index]);
}

void D3D11ResourceHeap::WriteResourceViewSampler(
    const ResourceViewDescriptor&   desc,
    char*                           heapPtr,
    std::uint32_t                   index)
{
    /* Get sampler resource */
    auto* samplerD3D = LLGL_CAST(D3D11Sampler*, GetAsExpectedSampler(desc.resource));

    /* Write D3D ComPtr */
    D3DRESOURCEHEAP_DATA0_SAMPLER(heapPtr)[index] = samplerD3D->GetNative();
}

ID3D11ShaderResourceView* D3D11ResourceHeap::GetOrCreateSRV(const ResourceViewDescriptor& desc, D3DSubresourceLocator& outLocator)
{
    Resource& resource = *(desc.resource);
    if (resource.GetResourceType() == ResourceType::Buffer)
    {
        auto& buffer = LLGL_CAST(Buffer&, resource);
        if ((buffer.GetBindFlags() & BindFlags::Sampled) != 0)
            return GetOrCreateBufferSRV(LLGL_CAST(D3D11BufferWithRV&, buffer), desc.bufferView, outLocator);
    }
    else if (resource.GetResourceType() == ResourceType::Texture)
    {
        auto& texture = LLGL_CAST(Texture&, resource);
        if ((texture.GetBindFlags() & BindFlags::Sampled) != 0)
            return GetOrCreateTextureSRV(LLGL_CAST(D3D11Texture&, texture), desc.textureView, outLocator);
    }
    return nullptr;
}

ID3D11UnorderedAccessView* D3D11ResourceHeap::GetOrCreateUAV(const ResourceViewDescriptor& desc, D3DSubresourceLocator& outLocator)
{
    Resource& resource = *(desc.resource);
    if (resource.GetResourceType() == ResourceType::Buffer)
    {
        auto& buffer = LLGL_CAST(Buffer&, resource);
        if ((buffer.GetBindFlags() & BindFlags::Storage) != 0)
            return GetOrCreateBufferUAV(LLGL_CAST(D3D11BufferWithRV&, buffer), desc.bufferView, outLocator);
    }
    else if (resource.GetResourceType() == ResourceType::Texture)
    {
        auto& texture = LLGL_CAST(Texture&, resource);
        if ((texture.GetBindFlags() & BindFlags::Storage) != 0)
            return GetOrCreateTextureUAV(LLGL_CAST(D3D11Texture&, texture), desc.textureView, outLocator);
    }
    return nullptr;
}

static D3D11SubresourceRange GetD3D11TextureSubresourceRange(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc)
{
    const TextureSubresource& subresource = textureViewDesc.subresource;
    D3D11SubresourceRange range;
    {
        range.begin = textureD3D.CalcSubresource(subresource.baseMipLevel, subresource.baseArrayLayer);
        range.end   = textureD3D.CalcSubresource(subresource.baseMipLevel + subresource.numMipLevels, subresource.baseArrayLayer + subresource.numArrayLayers);
    }
    return range;
}

ID3D11ShaderResourceView* D3D11ResourceHeap::GetOrCreateTextureSRV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc, D3DSubresourceLocator& outLocator)
{
    outLocator.locator  = textureD3D.GetBindingLocator();
    outLocator.range    = GetD3D11TextureSubresourceRange(textureD3D, textureViewDesc);

    if (IsTextureViewEnabled(textureViewDesc))
    {
        /* Create an SRV for the specified texture subresource */
        ComPtr<ID3D11ShaderResourceView> srv;
        textureD3D.CreateSubresourceSRV(
            nullptr,
            srv.GetAddressOf(),
            textureViewDesc.type,
            DXTypes::ToDXGIFormat(textureViewDesc.format),
            textureViewDesc.subresource.baseMipLevel,
            textureViewDesc.subresource.numMipLevels,
            textureViewDesc.subresource.baseArrayLayer,
            textureViewDesc.subresource.numArrayLayers
        );

        /* Store SRV in container to release together with resource heap */
        return subresourceSRVs_.Emplace(std::move(srv), &(outLocator.index));
    }
    else
    {
        /* Return standard SRV of this texture */
        return textureD3D.GetSRV();
    }
}

ID3D11UnorderedAccessView* D3D11ResourceHeap::GetOrCreateTextureUAV(D3D11Texture& textureD3D, const TextureViewDescriptor& textureViewDesc, D3DSubresourceLocator& outLocator)
{
    outLocator.locator  = textureD3D.GetBindingLocator();
    outLocator.range    = GetD3D11TextureSubresourceRange(textureD3D, textureViewDesc);

    if (IsTextureViewEnabled(textureViewDesc))
    {
        /* Create a UAV for the specified texture subresource */
        ComPtr<ID3D11UnorderedAccessView> uav;
        textureD3D.CreateSubresourceUAV(
            nullptr,
            uav.GetAddressOf(),
            textureViewDesc.type,
            DXTypes::ToDXGIFormat(textureViewDesc.format),
            textureViewDesc.subresource.baseMipLevel,
            textureViewDesc.subresource.baseArrayLayer,
            textureViewDesc.subresource.numArrayLayers
        );

        /* Store UAV in container to release together with resource heap */
        return subresourceUAVs_.Emplace(std::move(uav), &(outLocator.index));
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
    const FormatAttributes& formatAttribs = GetFormatAttribs(format);
    const UINT stride = (formatAttribs.bitSize / formatAttribs.blockWidth / formatAttribs.blockHeight / 8);
    LLGL_ASSERT(stride > 0, "cannot create buffer subresource with format stride of 0");
    return stride;
}

ID3D11ShaderResourceView* D3D11ResourceHeap::GetOrCreateBufferSRV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc, D3DSubresourceLocator& outLocator)
{
    outLocator.locator  = bufferD3D.GetBindingLocator();
    outLocator.range    = { 0, 1 };

    if (IsBufferViewEnabled(bufferViewDesc))
    {
        /* Get buffer stride by format */
        const UINT stride = GetFormatBufferStride(bufferViewDesc.format);

        /* Create a UAV for the specified texture subresource */
        ComPtr<ID3D11ShaderResourceView> srv;
        bufferD3D.CreateSubresourceSRV(
            nullptr,
            srv.GetAddressOf(),
            DXTypes::ToDXGIFormat(bufferViewDesc.format),
            static_cast<UINT>(bufferViewDesc.offset / stride),
            static_cast<UINT>(bufferViewDesc.size / stride)
        );

        /* Store SRV in container to release together with resource heap */
        return subresourceSRVs_.Emplace(std::move(srv), &(outLocator.index));
    }
    else
    {
        /* Return standard SRV of this buffer */
        return bufferD3D.GetSRV();
    }
}

ID3D11UnorderedAccessView* D3D11ResourceHeap::GetOrCreateBufferUAV(D3D11BufferWithRV& bufferD3D, const BufferViewDescriptor& bufferViewDesc, D3DSubresourceLocator& outLocator)
{
    outLocator.locator  = bufferD3D.GetBindingLocator();
    outLocator.range    = { 0, 1 };

    if (IsBufferViewEnabled(bufferViewDesc))
    {
        /* Get buffer stride by format */
        const UINT stride = GetFormatBufferStride(bufferViewDesc.format);

        /* Create a UAV for the specified texture subresource */
        ComPtr<ID3D11UnorderedAccessView> uav;
        bufferD3D.CreateSubresourceUAV(
            nullptr,
            uav.GetAddressOf(),
            DXTypes::ToDXGIFormat(bufferViewDesc.format),
            static_cast<UINT>(bufferViewDesc.offset / stride),
            static_cast<UINT>(bufferViewDesc.size / stride)
        );

        /* Store UAV in container to release together with resource heap */
        return subresourceUAVs_.Emplace(std::move(uav), &(outLocator.index));
    }
    else
    {
        /* Return standard UAV of this buffer */
        return bufferD3D.GetUAV();
    }
}

std::uint32_t D3D11ResourceHeap::ConsolidateSegments(
    const ArrayView<D3DResourceBinding>&    bindingSlots,
    const AllocSegmentFunc&                 allocSegmentFunc)
{
    return ConsolidateConsecutiveSequences<std::uint32_t>(
        bindingSlots.begin(),
        bindingSlots.end(),
        allocSegmentFunc,
        [](const D3DResourceBinding& entry) -> UINT
        {
            return entry.slot;
        }
    );
}

D3D11ResourceHeap::D3DShaderStage D3D11ResourceHeap::StageFlagsToD3DShaderStage(long stage)
{
    switch (stage)
    {
        case StageFlags::VertexStage:           return D3DShaderStage_VS;
        case StageFlags::TessControlStage:      return D3DShaderStage_DS;
        case StageFlags::TessEvaluationStage:   return D3DShaderStage_HS;
        case StageFlags::GeometryStage:         return D3DShaderStage_GS;
        case StageFlags::FragmentStage:         return D3DShaderStage_PS;
        case StageFlags::ComputeStage:          return D3DShaderStage_CS;
        default:                                return D3DShaderStage_Count;
    }
}

template <typename T>
void D3D11ResourceHeap::GarbageCollectSubresource(DXManagedComPtrArray<T>& subresources, SubresourceIndexContext& subresourceContext)
{
    if (subresourceContext.oldIndex != -1)
    {
        if (subresourceContext.newIndex != -1)
        {
            /* Release old subresource by moving new subresource at old location */
            subresources.Exchange(subresourceContext.oldIndex, subresources[subresourceContext.newIndex]);
            subresources.Remove(subresourceContext.newIndex);
        }
        else
        {
            /* Release old subresource by setting ComPtr to null */
            subresources.Remove(subresourceContext.oldIndex);
        }
    }
}


/*
 * SubresourceIndexContext struct
 */

void D3D11ResourceHeap::SubresourceIndexContext::Exchange(std::uint16_t& storage)
{
    /* Keep track of old index; they must be the same throughout all stages per descriptor */
    if (oldIndex == -1 && storage != static_cast<std::uint16_t>(-1))
        oldIndex = storage;

    /*
    If we created a new subresource and there was an old one,
    keep the old index in storage as we'll move the new subresource in place of the old one (every write always creates a new SRV)
    */
    if (newIndex == -1 || oldIndex == -1)
        storage = static_cast<std::uint16_t>(newIndex);
}

#undef BIND_SEGMENT_ALLOCATOR
#undef D3DRESOURCEHEAP_SEGMENT
#undef D3DRESOURCEHEAP_CONST_SEGMENT
#undef D3DRESOURCEHEAP_DATA0
#undef D3DRESOURCEHEAP_DATA0_COMPTR_CONST
#undef D3DRESOURCEHEAP_DATA0_CBV_CONST
#undef D3DRESOURCEHEAP_DATA0_SRV_CONST
#undef D3DRESOURCEHEAP_DATA0_UAV_CONST
#undef D3DRESOURCEHEAP_DATA0_SAMPLER_CONST
#undef D3DRESOURCEHEAP_DATA0_COMPTR
#undef D3DRESOURCEHEAP_DATA0_CBV
#undef D3DRESOURCEHEAP_DATA0_SRV
#undef D3DRESOURCEHEAP_DATA0_UAV
#undef D3DRESOURCEHEAP_DATA0_SAMPLER
#undef D3DRESOURCEHEAP_DATA1
#undef D3DRESOURCEHEAP_DATA2
#undef D3DRESOURCEHEAP_LOCATOR
#undef D3DRESOURCEHEAP_RANGE
#undef D3DRESOURCEHEAP_LOCATOR_CONST
#undef D3DRESOURCEHEAP_RANGE_CONST

#undef D3DRESOURCEHEAP_BIND_RESOURCE_GENERIC
#undef D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS1
#undef D3DRESOURCEHEAP_BIND_CONSTANTBUFFERS
#undef D3DRESOURCEHEAP_BIND_SAMPLERS
#undef D3DRESOURCEHEAP_BIND_SHADERRESOURCES
#undef D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_PS
#undef D3DRESOURCEHEAP_BIND_UNORDEREDACCESSVIEWS_CS


} // /namespace LLGL



// ================================================================================
