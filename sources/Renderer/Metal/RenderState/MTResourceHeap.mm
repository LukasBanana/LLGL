/*
 * MTResourceHeap.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTResourceHeap.h"
#include "MTPipelineLayout.h"
#include "../MTTypes.h"
#include "../Buffer/MTBuffer.h"
#include "../Texture/MTSampler.h"
#include "../Texture/MTTexture.h"
#include "../../TextureUtils.h"
#include "../../CheckedCast.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


/*
 * Internal structures
 */

/*

The internal buffer of MTResourceHeap is tightly packed which stores all segments of binding points consecutively.
Here is an illustration of the buffer layout for one Texture resource (at binding point 4) and two Sampler resources (at binding points 5 and 6) on a 64-bit build:

Offset      Attribute                               Value   Description                                         Segment
----------------------------------------------------------------------------------------------------------------------------------------
0x00000000  MTResourceHeapSegment::size                20   Size of this segment                                \
0x00000004  MTResourceHeapSegment::range::location      4   First binding point                                  |
0x00000008  MTResourceHeapSegment::range::length        1   Number of binding points                             |-- Texture segment
0x0000000C  MTResourceHeapSegment::data1Offset          0   <unused>                                             |
0x00000010  texture[0]                                  1   1st Metal texture ID (of type id<MTLTexture>)        /
0x00000018  MTResourceHeapSegment::size                20   Size of this segment                                \
0x0000001C  MTResourceHeapSegment::range::location      5   First binding point                                  |
0x00000020  MTResourceHeapSegment::range::length        2   Number of binding points                             |-- Sampler segment
0x00000024  sampler[0]                                  1   1st Metal sampler ID (of type id<MTLSamplerState>)   |
0x0000002C  sampler[1]                                  2   2nd Metal sampler ID (of type id<MTLSamplerState>)  /

*/

// Internal enumeration for Metal resource heap segments.
enum MTResourceType : std::uint32_t
{
    MTResourceType_Buffer = 0,
    MTResourceType_Texture,
    MTResourceType_SamplerState,
};

// Resource view heap (RVH) segment structure with up to two dynamic sub-buffer (id<MTLBuffer>, id<MTLTexture>, or id<MTLSamplerState>).
struct MTResourceHeapSegment
{
    std::uint32_t   size        : 30;
    MTResourceType  type        :  2;
    NSRange         range;
    std::uint32_t   data1Offset;
};

#define MTRESOURCEHEAP_SEGMENT(PTR)                     reinterpret_cast<MTResourceHeapSegment*>(PTR)
#define MTRESOURCEHEAP_CONST_SEGMENT(PTR)               reinterpret_cast<const MTResourceHeapSegment*>(PTR)
#define MTRESOURCEHEAP_DATA0(PTR, TYPE)                 reinterpret_cast<TYPE*>((PTR) + sizeof(MTResourceHeapSegment))
#define MTRESOURCEHEAP_DATA0_MTLBUFFER_CONST(PTR)       MTRESOURCEHEAP_DATA0(PTR, const id<MTLBuffer>)
#define MTRESOURCEHEAP_DATA0_MTLTEXTURE_CONST(PTR)      MTRESOURCEHEAP_DATA0(PTR, const id<MTLTexture>)
#define MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE_CONST(PTR) MTRESOURCEHEAP_DATA0(PTR, const id<MTLSamplerState>)
#define MTRESOURCEHEAP_DATA0_MTLBUFFER(PTR)             MTRESOURCEHEAP_DATA0(PTR, id<MTLBuffer>)
#define MTRESOURCEHEAP_DATA0_MTLTEXTURE(PTR)            MTRESOURCEHEAP_DATA0(PTR, id<MTLTexture>)
#define MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE(PTR)       MTRESOURCEHEAP_DATA0(PTR, id<MTLSamplerState>)
#define MTRESOURCEHEAP_DATA1(PTR, TYPE)                 reinterpret_cast<TYPE*>((PTR) + MTRESOURCEHEAP_CONST_SEGMENT(PTR)->data1Offset)
#define MTRESOURCEHEAP_DATA1_OFFSETS_CONST(PTR)         MTRESOURCEHEAP_DATA1(PTR, const NSUInteger)
#define MTRESOURCEHEAP_DATA1_OFFSETS(PTR)               MTRESOURCEHEAP_DATA1(PTR, NSUInteger)


/*
 * MTResourceHeap class
 */

MTResourceHeap::MTResourceHeap(
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
{
    /* Get pipeline layout object */
    auto pipelineLayoutMT = LLGL_CAST(MTPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutMT)
        throw std::invalid_argument("failed to create resource heap due to missing pipeline layout");

    /* Validate binding descriptors */
    const auto& bindings            = pipelineLayoutMT->GetHeapBindings();
    const auto  numBindings         = static_cast<std::uint32_t>(bindings.size());
    const auto  numResourceViews    = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    /* Allocate array to map binding index to descriptor index */
    bindingMap_.resize(numBindings);

    /* Build buffer segments */
    constexpr long vertexStages     = (StageFlags::VertexStage | StageFlags::TessEvaluationStage);
    constexpr long fragmentStages   = (StageFlags::FragmentStage);
    constexpr long kernelStages     = (StageFlags::ComputeStage | StageFlags::TessControlStage);

    BindingDescriptorIterator bindingIter{ bindings };

    /* Build vertex resource segments */
    segmentation_.numVertexBufferSegments       = AllocBufferSegments(bindingIter, vertexStages);
    segmentation_.numVertexTextureSegments      = AllocTextureSegments(bindingIter, vertexStages);
    segmentation_.numVertexSamplerSegments      = AllocSamplerStateSegments(bindingIter, vertexStages);

    /* Build fragment resource segments */
    segmentation_.numFragmentBufferSegments     = AllocBufferSegments(bindingIter, fragmentStages);
    segmentation_.numFragmentTextureSegments    = AllocTextureSegments(bindingIter, fragmentStages);
    segmentation_.numFragmentSamplerSegments    = AllocSamplerStateSegments(bindingIter, fragmentStages);

    /* Build kernel resource segments (and store buffer offset to kernel segments) */
    heapOffsetKernel_ = static_cast<std::uint32_t>(heap_.Size());

    segmentation_.numKernelBufferSegments       = AllocBufferSegments(bindingIter, kernelStages);
    segmentation_.numKernelTextureSegments      = AllocTextureSegments(bindingIter, kernelStages);
    segmentation_.numKernelSamplerSegments      = AllocSamplerStateSegments(bindingIter, kernelStages);

    /* Store resource usage bits in segmentation header */
    CacheResourceUsage();

    /* Finalize segments in buffer */
    const auto numSegmentSets = (numResourceViews / numBindings);
    heap_.FinalizeSegments(numSegmentSets);

    /* Allocate texture view array */
    textureViews_.resize(numTextureViewsPerSet_ * numSegmentSets);

    /* Write initial resource views */
    if (!initialResourceViews.empty())
        WriteResourceViews(0, initialResourceViews);
}

MTResourceHeap::~MTResourceHeap()
{
    for (auto& tex : textureViews_)
    {
        if (tex != nil)
            [tex release];
    }
}

std::uint32_t MTResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(heap_.NumSets());
}

std::uint32_t MTResourceHeap::WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    /* Quit if there's nothing to do */
    if (resourceViews.empty())
        return 0;

    const auto numSets          = GetNumDescriptorSets();
    const auto numBindings      = static_cast<std::uint32_t>(bindingMap_.size());
    const auto numDescriptors   = numSets * numBindings;

    /* Silently quit on out of bounds; debug layer must report these errors */
    if (firstDescriptor >= numDescriptors)
        return 0;
    if (firstDescriptor + resourceViews.size() > numDescriptors)
        return 0;

    /* Write each resource view into respective segment */
    std::uint32_t numWritten = 0;

    for (const auto& desc : resourceViews)
    {
        /* Skip over empty resource descriptors */
        if (desc.resource == nullptr)
            continue;

        /* Get binding information and heap start for descriptor set */
        const auto& binding = bindingMap_[firstDescriptor % numBindings];

        auto descriptorSet  = firstDescriptor / numBindings;
        auto heapStartPtr   = heap_.SegmentData(descriptorSet);

        /* Write descriptor into respective heap segment for each affected shader stage */
        for_range(stage, static_cast<int>(MTShaderStage_Count))
        {
            auto offset     = binding.stages[stage].segmentOffset;
            if (offset == BindingSegmentLocation::invalidOffset)
                continue;

            auto heapPtr    = heapStartPtr + offset;
            auto segment    = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);

            switch (segment->type)
            {
                case MTResourceType_Buffer:
                    WriteResourceViewBuffer(desc, heapPtr, binding.stages[stage]);
                    break;
                case MTResourceType_Texture:
                    WriteResourceViewTexture(desc, heapPtr, binding.stages[stage], descriptorSet);
                    break;
                case MTResourceType_SamplerState:
                    WriteResourceViewSamplerState(desc, heapPtr, binding.stages[stage]);
                    break;
            }
        }

        ++numWritten;
        ++firstDescriptor;
    }

    return numWritten;
}

void MTResourceHeap::BindGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder, std::uint32_t descriptorSet)
{
    if (descriptorSet >= heap_.NumSets())
        return;

    const char* heapPtr = heap_.SegmentData(descriptorSet);
    if (segmentation_.hasVertexResources)
        heapPtr = BindVertexResources(renderEncoder, heapPtr);
    if (segmentation_.hasFragmentResources)
        BindFragmentResources(renderEncoder, heapPtr);
}

void MTResourceHeap::BindComputeResources(id<MTLComputeCommandEncoder> computeEncoder, std::uint32_t descriptorSet)
{
    if (descriptorSet >= heap_.NumSets())
        return;

    if (segmentation_.hasKernelResources)
    {
        auto heapPtr = heap_.SegmentData(descriptorSet) + heapOffsetKernel_;
        BindKernelResources(computeEncoder, heapPtr);
    }
}

bool MTResourceHeap::HasGraphicsResources() const
{
    return ((segmentation_.hasVertexResources | segmentation_.hasFragmentResources) != 0);
}

bool MTResourceHeap::HasComputeResources() const
{
    return (segmentation_.hasKernelResources != 0);
}


/*
 * ======= Private: =======
 */

#define BIND_SEGMENT_ALLOCATOR(FUNC, STAGE, TYPE, ...)  \
    std::bind(                                          \
        &FUNC,                                          \
        this,                                           \
        StageFlagsToMTShaderStage(STAGE),               \
        TYPE,                                           \
        std::placeholders::_1, std::placeholders::_2,   \
        __VA_ARGS__                                     \
    )

static MTShaderStage StageFlagsToMTShaderStage(long stage)
{
    if ((stage & StageFlags::VertexStage) != 0)
        return MTShaderStage_Vertex;
    if ((stage & StageFlags::FragmentStage) != 0)
        return MTShaderStage_Fragment;
    if ((stage & StageFlags::ComputeStage) != 0)
        return MTShaderStage_Kernel;
    return MTShaderStage_Count;
}

std::vector<MTResourceHeap::MTResourceBinding> MTResourceHeap::FilterAndSortMTBindingSlots(
    BindingDescriptorIterator&  bindingIter,
    ResourceType                resourceType,
    long                        affectedStage)
{
    /* Collect all binding points of the specified resource types */
    std::vector<MTResourceBinding> resourceBindings;
    resourceBindings.reserve(bindingIter.GetCount());

    bindingIter.Reset(resourceType, /*bindFlags:*/ 0, affectedStage);
    for (std::size_t index = 0; const auto* bindingDesc = bindingIter.Next(&index);)
    {
        MTResourceBinding resourceBinding;
        {
            resourceBinding.slot    = bindingDesc->slot.index;
            resourceBinding.stages  = affectedStage;
            resourceBinding.index   = index;
        }
        resourceBindings.push_back(resourceBinding);
    }

    /* Sort resources by slot index */
    std::sort(
        resourceBindings.begin(),
        resourceBindings.end(),
        [](const MTResourceBinding& lhs, const MTResourceBinding& rhs)
        {
            return (lhs.slot < rhs.slot);
        }
    );

    return resourceBindings;
}

MTResourceHeap::SegmentationSizeType MTResourceHeap::AllocBufferSegments(BindingDescriptorIterator& bindingIter, long stage)
{
    /* Collect all buffers */
    auto bufferBindingSlots = MTResourceHeap::FilterAndSortMTBindingSlots(bindingIter, ResourceType::Buffer, stage);

    /* Build all resource segments for MTLBuffer ranges */
    return MTResourceHeap::ConsolidateSegments(
        bufferBindingSlots,
        BIND_SEGMENT_ALLOCATOR(
            MTResourceHeap::Alloc2PartSegment, stage, MTResourceType_Buffer,
            sizeof(id<MTLBuffer>*), sizeof(NSUInteger)
        )
    );
}

MTResourceHeap::SegmentationSizeType MTResourceHeap::AllocTextureSegments(BindingDescriptorIterator& bindingIter, long stage)
{
    /* Collect all textures */
    auto textureBindingSlots = MTResourceHeap::FilterAndSortMTBindingSlots(bindingIter, ResourceType::Texture, stage);

    /* Build all resource segments for MTLTexture ranges */
    return MTResourceHeap::ConsolidateSegments(
        textureBindingSlots,
        BIND_SEGMENT_ALLOCATOR(
            MTResourceHeap::Alloc1PartSegment, stage, MTResourceType_Texture,
            sizeof(id<MTLTexture>*)
        )
    );
}

MTResourceHeap::SegmentationSizeType MTResourceHeap::AllocSamplerStateSegments(BindingDescriptorIterator& bindingIter, long stage)
{
    /* Collect all samplers */
    auto samplerBindingSlots = MTResourceHeap::FilterAndSortMTBindingSlots(bindingIter, ResourceType::Sampler, stage);

    /* Build all resource segments for MTLSamplerState ranges */
    return MTResourceHeap::ConsolidateSegments(
        samplerBindingSlots,
        BIND_SEGMENT_ALLOCATOR(
            MTResourceHeap::Alloc1PartSegment, stage, MTResourceType_SamplerState,
            sizeof(id<MTLSamplerState>*)
        )
    );
}

void MTResourceHeap::Alloc1PartSegment(
    MTShaderStage               stage,
    MTResourceType              type,
    const MTResourceBinding*    first,
    NSUInteger                  count,
    std::size_t                 payload0Stride)
{
    /* Write binding map entries */
    WriteBindingMappings(stage, type, first, count);

    /* Allocate space for segment */
    const auto  payloadSize     = static_cast<std::uint32_t>(payload0Stride * count);
    auto        segmentAlloc    = heap_.AllocSegment<MTResourceHeapSegment>(payloadSize);

    /* Write segment header */
    auto header = segmentAlloc.Header();
    {
        header->size        = segmentAlloc.Size();
        header->type        = type;
        header->range       = NSMakeRange(first->slot, count);
        header->data1Offset = 0;
    }
}

void MTResourceHeap::Alloc2PartSegment(
    MTShaderStage               stage,
    MTResourceType              type,
    const MTResourceBinding*    first,
    NSUInteger                  count,
    std::size_t                 payload0Stride,
    std::size_t                 payload1Stride)
{
    /* Write binding map entries */
    WriteBindingMappings(stage, type, first, count);

    /* Allocate space for segment */
    const auto  payloadData1Offset  = static_cast<std::uint32_t>(payload0Stride * count);
    const auto  payloadSize         = static_cast<std::uint32_t>(payload1Stride * count + payloadData1Offset);
    auto        segmentAlloc        = heap_.AllocSegment<MTResourceHeapSegment>(payloadSize);

    /* Write segment header */
    auto header = segmentAlloc.Header();
    {
        header->size        = segmentAlloc.Size();
        header->type        = type;
        header->range       = NSMakeRange(first->slot, count);
        header->data1Offset = segmentAlloc.PayloadOffset() + payloadData1Offset;
    }
}

void MTResourceHeap::WriteBindingMappings(MTShaderStage stage, MTResourceType type, const MTResourceBinding* first, NSUInteger count)
{
    for_range(i, static_cast<std::uint32_t>(count))
    {
        LLGL_ASSERT(first[i].index < bindingMap_.size());
        auto& mapping = bindingMap_[first[i].index];
        mapping.stages[stage].segmentOffset     = static_cast<std::uint32_t>(heap_.Size());
        mapping.stages[stage].descriptorIndex   = i;
        mapping.stages[stage].textureViewIndex  = (type == MTResourceType_Texture ? numTextureViewsPerSet_++ : 0);
    }
}

void MTResourceHeap::CacheResourceUsage()
{
    if ( ( segmentation_.numVertexBufferSegments  |
           segmentation_.numVertexTextureSegments |
           segmentation_.numVertexSamplerSegments ) != 0 )
    {
        segmentation_.hasVertexResources = 1;
    }

    if ( ( segmentation_.numFragmentBufferSegments  |
           segmentation_.numFragmentTextureSegments |
           segmentation_.numFragmentSamplerSegments ) != 0 )
    {
        segmentation_.hasFragmentResources = 1;
    }

    if ( ( segmentation_.numKernelBufferSegments  |
           segmentation_.numKernelTextureSegments |
           segmentation_.numKernelSamplerSegments ) != 0 )
    {
        segmentation_.hasKernelResources = 1;
    }
}

const char* MTResourceHeap::BindVertexResources(id<MTLRenderCommandEncoder> cmdEncoder, const char* heapPtr)
{
    /* Bind all buffers */
    for_range(i, segmentation_.numVertexBufferSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setVertexBuffers:   MTRESOURCEHEAP_DATA0_MTLBUFFER_CONST(heapPtr)
            offsets:            MTRESOURCEHEAP_DATA1_OFFSETS_CONST(heapPtr)
            withRange:          segment->range
        ];
        heapPtr += segment->size;
    }

    /* Bind all textures */
    for_range(i, segmentation_.numVertexTextureSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setVertexTextures:  MTRESOURCEHEAP_DATA0_MTLTEXTURE_CONST(heapPtr)
            withRange:          segment->range
        ];
        heapPtr += segment->size;
    }

    /* Bind all samplers */
    for_range(i, segmentation_.numVertexSamplerSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setVertexSamplerStates: MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE_CONST(heapPtr)
            withRange:              segment->range
        ];
        heapPtr += segment->size;
    }

    return heapPtr;
}

const char* MTResourceHeap::BindFragmentResources(id<MTLRenderCommandEncoder> cmdEncoder, const char* heapPtr)
{
    /* Bind all buffers */
    for_range(i, segmentation_.numFragmentBufferSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setFragmentBuffers: MTRESOURCEHEAP_DATA0_MTLBUFFER_CONST(heapPtr)
            offsets:            MTRESOURCEHEAP_DATA1_OFFSETS_CONST(heapPtr)
            withRange:          segment->range
        ];
        heapPtr += segment->size;
    }

    /* Bind all textures */
    for_range(i, segmentation_.numFragmentTextureSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setFragmentTextures:    MTRESOURCEHEAP_DATA0_MTLTEXTURE_CONST(heapPtr)
            withRange:              segment->range
        ];
        heapPtr += segment->size;
    }

    /* Bind all samplers */
    for_range(i, segmentation_.numFragmentSamplerSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setFragmentSamplerStates:   MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE_CONST(heapPtr)
            withRange:                  segment->range
        ];
        heapPtr += segment->size;
    }

    return heapPtr;
}

const char* MTResourceHeap::BindKernelResources(id<MTLComputeCommandEncoder> cmdEncoder, const char* heapPtr)
{
    /* Bind all buffers */
    for_range(i, segmentation_.numKernelBufferSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setBuffers: MTRESOURCEHEAP_DATA0_MTLBUFFER_CONST(heapPtr)
            offsets:    MTRESOURCEHEAP_DATA1_OFFSETS_CONST(heapPtr)
            withRange:  segment->range
        ];
        heapPtr += segment->size;
    }

    /* Bind all textures */
    for_range(i, segmentation_.numKernelTextureSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setTextures:    MTRESOURCEHEAP_DATA0_MTLTEXTURE_CONST(heapPtr)
            withRange:      segment->range
        ];
        heapPtr += segment->size;
    }

    /* Bind all samplers */
    for_range(i, segmentation_.numKernelSamplerSegments)
    {
        auto segment = MTRESOURCEHEAP_CONST_SEGMENT(heapPtr);
        [cmdEncoder
            setSamplerStates:   MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE_CONST(heapPtr)
            withRange:          segment->range
        ];
        heapPtr += segment->size;
    }

    return heapPtr;
}

void MTResourceHeap::WriteResourceViewBuffer(const ResourceViewDescriptor& desc, char* heapPtr, const BindingSegmentLocation::Stage& binding)
{
    /* Get buffer resource and write MTLBuffer ID plus offset (Metal only needs offset) */
    auto bufferMT = LLGL_CAST(MTBuffer*, GetAsExpectedBuffer(desc.resource));
    MTRESOURCEHEAP_DATA0_MTLBUFFER(heapPtr)[binding.descriptorIndex] = bufferMT->GetNative();
    MTRESOURCEHEAP_DATA1_OFFSETS(heapPtr)[binding.descriptorIndex] = static_cast<NSUInteger>(desc.bufferView.offset);
}

void MTResourceHeap::WriteResourceViewTexture(const ResourceViewDescriptor& desc, char* heapPtr, const BindingSegmentLocation::Stage& binding, std::uint32_t descriptorSet)
{
    /* Get texture resource and Write MTLTexture ID */
    auto textureMT = LLGL_CAST(MTTexture*, GetAsExpectedTexture(desc.resource));
    MTRESOURCEHEAP_DATA0_MTLTEXTURE(heapPtr)[binding.descriptorIndex] = GetOrCreateTexture(descriptorSet, binding, *textureMT, desc.textureView);
}

void MTResourceHeap::WriteResourceViewSamplerState(const ResourceViewDescriptor& desc, char* heapPtr, const BindingSegmentLocation::Stage& binding)
{
    /* Get sampler resource and Write MTLSamplerState ID */
    auto samplerMT = LLGL_CAST(MTSampler*, GetAsExpectedSampler(desc.resource));
    MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE(heapPtr)[binding.descriptorIndex] = samplerMT->GetNative();
}

[[noreturn]]
static void ErrTextureViewSwizzleNotSupported()
{
    LLGL_TRAP("cannot create texture-view with swizzling for this version of the Metal API");
}

static void ValidateTexViewNoSwizzle(const TextureViewDescriptor& desc)
{
    if (!IsTextureSwizzleIdentity(desc.swizzle))
        ErrTextureViewSwizzleNotSupported();
}

static void ValidateTexViewNoTypeAndRange(const TextureViewDescriptor& desc, MTTexture& textureMT)
{
    id<MTLTexture> tex = textureMT.GetNative();
    if (MTTypes::ToMTLTextureType(desc.type) != [tex textureType])
        LLGL_TRAP("cannot create texture-view of different type for this version of the Metal API");
    if (desc.subresource.baseMipLevel != 0 || desc.subresource.numMipLevels != [tex mipmapLevelCount])
        LLGL_TRAP("cannot create texture-view of different MIP-level range for this version of the Metal API");
    if (desc.subresource.baseArrayLayer != 0 || desc.subresource.numArrayLayers != [tex arrayLength])
        LLGL_TRAP("cannot create texture-view of different array-layer range for this version of the Metal API");
}

void MTResourceHeap::ExchangeTextureView(
    std::uint32_t                           descriptorSet,
    const BindingSegmentLocation::Stage&    binding,
    id<MTLTexture>                          textureView)
{
    LLGL_ASSERT(binding.textureViewIndex < numTextureViewsPerSet_);
    auto& texViewEntry = textureViews_[descriptorSet * numTextureViewsPerSet_ + binding.textureViewIndex];
    if (texViewEntry != textureView)
    {
        if (texViewEntry != nil)
            [texViewEntry release];
        texViewEntry = textureView;
    }
}

id<MTLTexture> MTResourceHeap::GetOrCreateTexture(
    std::uint32_t                           descriptorSet,
    const BindingSegmentLocation::Stage&    binding,
    MTTexture&                              textureMT,
    const TextureViewDescriptor&            textureViewDesc)
{
    if (IsTextureViewEnabled(textureViewDesc))
    {
        /* Create texture view from source texture */
        const auto& subresource = textureViewDesc.subresource;
        id<MTLTexture> textureView = nil;

        /* Try different methods to create a texture view as some might return NIL even though the host system should support it */
        if (!IsTextureSwizzleIdentity(textureViewDesc.swizzle))
        {
            /*
            This method is available in macOS 10.15 and iOS 13.0,
            but it is only supported by GPU family MTLGPUFamilyMetal3 or later,
            which is only available in macOS 13.0 and iOS 16.0 or later.
            */
            if (@available(macOS 13.0, iOS 16.0, *))
            {
                MTLTextureSwizzleChannels swizzle;
                MTTypes::Convert(swizzle, textureViewDesc.swizzle);
                textureView = [textureMT.GetNative()
                    newTextureViewWithPixelFormat:  MTTypes::ToMTLPixelFormat(textureViewDesc.format)
                    textureType:                    MTTypes::ToMTLTextureType(textureViewDesc.type)
                    levels:                         NSMakeRange(subresource.baseMipLevel, subresource.numMipLevels)
                    slices:                         NSMakeRange(subresource.baseArrayLayer, subresource.numArrayLayers)
                    swizzle:                        swizzle
                ];
            }
            else
                ErrTextureViewSwizzleNotSupported();
        }
        else if (@available(macOS 10.11, iOS 9.0, *))
        {
            ValidateTexViewNoSwizzle(textureViewDesc);
            textureView = [textureMT.GetNative()
                newTextureViewWithPixelFormat:  MTTypes::ToMTLPixelFormat(textureViewDesc.format)
                textureType:                    MTTypes::ToMTLTextureType(textureViewDesc.type)
                levels:                         NSMakeRange(subresource.baseMipLevel, subresource.numMipLevels)
                slices:                         NSMakeRange(subresource.baseArrayLayer, subresource.numArrayLayers)
            ];
        }
        else
        {
            ValidateTexViewNoSwizzle(textureViewDesc);
            ValidateTexViewNoTypeAndRange(textureViewDesc, textureMT);
            textureView = [textureMT.GetNative()
                newTextureViewWithPixelFormat:  MTTypes::ToMTLPixelFormat(textureViewDesc.format)
            ];
        }

        /* Store texture view reference */
        LLGL_ASSERT(textureView != nil, "unable to create Metal texture view");
        ExchangeTextureView(descriptorSet, binding, textureView);
        return textureView;
    }
    else
    {
        /* Release previously stored texture view reference */
        ExchangeTextureView(descriptorSet, binding, nil);
        return textureMT.GetNative();
    }
}

MTResourceHeap::SegmentationSizeType MTResourceHeap::ConsolidateSegments(
    const ArrayView<MTResourceBinding>& bindingSlots,
    const AllocSegmentFunc&             allocSegmentFunc)
{
    return ConsolidateConsecutiveSequences<SegmentationSizeType>(
        bindingSlots.begin(),
        bindingSlots.end(),
        allocSegmentFunc,
        [](const MTResourceBinding& entry) -> NSUInteger
        {
            return entry.slot;
        }
    );
}

#undef MTRESOURCEHEAP_SEGMENT
#undef MTRESOURCEHEAP_CONST_SEGMENT
#undef MTRESOURCEHEAP_DATA0
#undef MTRESOURCEHEAP_DATA0_MTLBUFFER_CONST
#undef MTRESOURCEHEAP_DATA0_MTLTEXTURE_CONST
#undef MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE_CONST
#undef MTRESOURCEHEAP_DATA0_MTLBUFFER
#undef MTRESOURCEHEAP_DATA0_MTLTEXTURE
#undef MTRESOURCEHEAP_DATA0_MTLSAMPLERSTATE
#undef MTRESOURCEHEAP_DATA1
#undef MTRESOURCEHEAP_DATA1_OFFSETS_CONST
#undef MTRESOURCEHEAP_DATA1_OFFSETS
#undef BIND_SEGMENT_ALLOCATOR


} // /namespace LLGL



// ================================================================================
