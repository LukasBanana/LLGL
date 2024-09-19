/*
 * GLResourceHeap.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLResourceHeap.h"
#include "GLPipelineLayout.h"
#include "GLStateManager.h"
#include "GLResourceType.h"
#include "../Ext/GLExtensions.h"
#include "../Buffer/GLBuffer.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLEmulatedSampler.h"
#include "../Texture/GLTexture.h"
#include "../Texture/GLTextureViewPool.h"
#include "../../CheckedCast.h"
#include "../../BindingDescriptorIterator.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../StaticAssertions.h"
#include "../../ResourceUtils.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Utils/ForRange.h>
#include <string.h>
#include <limits.h>


namespace LLGL
{


/*
 * Internal structures
 */

/*

The internal buffer of GLResourceHeap is tightly packed which stores all segments of binding points consecutively.
Here is an illustration of the buffer layout for one Texture resouce (at binding point 4) and two Sampler resources (at binding points 5 and 6) on a 32-bit build:

Offset      Attribute                              Value   Description                                         Segment
----------------------------------------------------------------------------------------------------------------------------------------
0x00000000  GLResourceHeapSegment::size               20   Size of this segment                                \
0x00000004  GLResourceHeapSegment::first               4   First binding point                                  |
0x00000006  GLResourceHeapSegment::count               1   Number of binding points                             |
0x00000008  GLResourceHeapSegment::data1Offset        16   Relative offset to target[0] (at 0x00000010)         |-- Texture segment
0x0000000A  GLResourceHeapSegment::data2Offset         0                    <unused>                            |
0x0000000C  texture[0]                                 1   1st OpenGL texture ID (from 'glGenTextures')         |
0x00000010  target[0]                                  1   Texture target (GLTextureTarget::TEXTURE_2D = 1)    /
0x00000014  GLResourceHeapSegment::size               20   Size of this segment                                \
0x00000018  GLResourceHeapSegment::first               5   First binding point                                  |
0x0000001A  GLResourceHeapSegment::count               2   Number of binding points                             |
0x0000001C  GLResourceHeapSegment::data1Offset         0                    <unused>                            |-- Sampler segment
0x0000001E  GLResourceHeapSegment::data2Offset         0                    <unused>                            |
0x00000020  sampler[0]                                 1   1st OpenGL sampler ID (from 'glGenSamplers')         |
0x00000024  sampler[1]                                 2   2nd OpenGL sampler ID (from 'glGenSamplers')        /

*/

// Resource segment flags. Bits can be shared as they are only used for certain segment types.
enum GLResourceFlags : std::uint32_t
{
    GLResourceFlags_HasBufferRange  = (1 << 0),
    GLResourceFlags_HasTextureViews = (1 << 0),
};

// Resource view heap (RVH) segment structure with up to three dynamic sub-buffers.
struct alignas(sizeof(std::uintptr_t)) GLResourceHeapSegment
{
    std::uint32_t   size        : 28; // Byte size of this segment
    std::uint32_t   flags       :  1; // GLResourceFlags
    GLResourceType  type        :  3;
    GLuint          first       : 16;
    GLsizei         count       : 16;
    std::uint32_t   data1Offset : 16; // Byte offset after the first sub-buffer, following the second sub-buffer
    std::uint32_t   data2Offset : 16; // Byte offset after the second sub-buffer, following the third sub-buffer
};

LLGL_ASSERT_POD_TYPE(GLResourceHeapSegment);


/*
 * Internal functions
 */

#define GLRESOURCEHEAP_SEGMENT(PTR)         reinterpret_cast<GLResourceHeapSegment*>(PTR)
#define GLRESOURCEHEAP_CONST_SEGMENT(PTR)   reinterpret_cast<const GLResourceHeapSegment*>(PTR)
#define GLRESOURCEHEAP_DATA0(PTR, TYPE)     reinterpret_cast<TYPE*>((PTR) + sizeof(GLResourceHeapSegment))
#define GLRESOURCEHEAP_DATA1(PTR, TYPE)     reinterpret_cast<TYPE*>((PTR) + GLRESOURCEHEAP_CONST_SEGMENT(PTR)->data1Offset)
#define GLRESOURCEHEAP_DATA2(PTR, TYPE)     reinterpret_cast<TYPE*>((PTR) + GLRESOURCEHEAP_CONST_SEGMENT(PTR)->data2Offset)

// Returns true if the specified buffer view is enabled for OpenGL bindings
static bool IsGLBufferViewEnabled(const BufferViewDescriptor& bufferViewDesc)
{
    /* For OpenGL buffer binding, only the range is relevant, no format is considerd */
    return (bufferViewDesc.size != LLGL_WHOLE_SIZE);
}


/*
 * GLResourceHeap class
 */

GLResourceHeap::GLResourceHeap(
    const ResourceHeapDescriptor&               desc,
    const ArrayView<ResourceViewDescriptor>&    initialResourceViews)
{
    /* Get pipeline layout object */
    auto pipelineLayoutGL = LLGL_CAST(const GLPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutGL)
        LLGL_TRAP("failed to create resource heap due to missing pipeline layout");

    /* Get and validate number of bindings and resource views */
    const auto&         bindings            = pipelineLayoutGL->GetHeapBindings();
    const std::uint32_t numBindings         = static_cast<std::uint32_t>(bindings.size());
    const std::uint32_t numResourceViews    = GetNumResourceViewsOrThrow(numBindings, desc, initialResourceViews);

    /* Allocate array to map binding index to descriptor index */
    bindingMap_.resize(numBindings);

    /* Allocate templates for all resource view segments */
    BindingDescriptorIterator bindingIter{ bindings };

    AllocSegmentsUBO(bindingIter);
    AllocSegmentsSSBO(bindingIter);
    AllocSegmentsTexture(bindingIter);
    AllocSegmentsImage(bindingIter);
    AllocSegmentsSampler(bindingIter);

    /* Finalize segments in buffer */
    const std::uint32_t numSegmentSets = (numResourceViews / numBindings);
    heap_.FinalizeSegments(numSegmentSets);

    /* Write initial resource views */
    if (!initialResourceViews.empty())
        WriteResourceViews(0, initialResourceViews);
}

GLResourceHeap::~GLResourceHeap()
{
    /* Release all texture views for this resource heap */
    FreeAllSegmentsTextureViews();
}

std::uint32_t GLResourceHeap::WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
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
        const BindingSegmentLocation&   binding         = bindingMap_[firstDescriptor % numBindings];
        const std::uint32_t             descriptorSet   = firstDescriptor / numBindings;
        char*                           heapStartPtr    = heap_.SegmentData(descriptorSet);
        char*                           heapPtr         = heapStartPtr + binding.segmentOffset;
        const GLResourceHeapSegment*    segment         = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);

        /* Write descriptor into respective heap segment */
        switch (segment->type)
        {
            case GLResourceType_Invalid:
                /* Ignore */
                break;
            case GLResourceType_UBO:
                WriteResourceViewUBO(desc, heapPtr, binding.descriptorIndex);
                break;
            case GLResourceType_SSBO:
                WriteResourceViewSSBO(desc, heapPtr, binding.descriptorIndex);
                break;
            case GLResourceType_Texture:
                WriteResourceViewTexture(desc, heapPtr, binding.descriptorIndex);
                break;
            case GLResourceType_Image:
                WriteResourceViewImage(desc, heapPtr, binding.descriptorIndex);
                break;
            case GLResourceType_Sampler:
                WriteResourceViewSampler(desc, heapPtr, binding.descriptorIndex);
                break;
            case GLResourceType_EmulatedSampler:
                WriteResourceViewEmulatedSampler(desc, heapPtr, binding.descriptorIndex);
                break;
        }

        ++numWritten;
        ++firstDescriptor;
    }

    return numWritten;
}

static std::size_t BindBuffersSegment(GLStateManager& stateMngr, const char* heapPtr, const GLBufferTarget bufferTarget)
{
    auto* segment = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);
    {
        const bool hasBufferRangeData = ((segment->flags & GLResourceFlags_HasBufferRange) != 0);
        if (hasBufferRangeData)
        {
            stateMngr.BindBuffersRange(
                bufferTarget,
                segment->first,
                segment->count,
                reinterpret_cast<const GLuint*>(heapPtr + sizeof(GLResourceHeapSegment)),
                reinterpret_cast<const GLintptr*>(heapPtr + segment->data1Offset),
                reinterpret_cast<const GLsizeiptr*>(heapPtr + segment->data2Offset)
            );
        }
        else
        {
            stateMngr.BindBuffersBase(
                bufferTarget,
                segment->first,
                segment->count,
                reinterpret_cast<const GLuint*>(heapPtr + sizeof(GLResourceHeapSegment))
            );
        }
    }
    return segment->size;
}

static std::size_t BindTexturesSegment(GLStateManager& stateMngr, const char* heapPtr)
{
    auto* segment = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);
    {
        stateMngr.BindTextures(
            segment->first,
            segment->count,
            reinterpret_cast<const GLTextureTarget*>(heapPtr + segment->data1Offset),
            reinterpret_cast<const GLuint*>(heapPtr + sizeof(GLResourceHeapSegment))
        );
    }
    return segment->size;
}

static std::size_t BindImageTexturesSegment(GLStateManager& stateMngr, const char* heapPtr)
{
    auto* segment = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);
    {
        stateMngr.BindImageTextures(
            segment->first,
            segment->count,
            reinterpret_cast<const GLenum*>(heapPtr + segment->data1Offset),
            reinterpret_cast<const GLuint*>(heapPtr + sizeof(GLResourceHeapSegment))
        );
    }
    return segment->size;
}

static std::size_t BindSamplersSegment(GLStateManager& stateMngr, const char* heapPtr)
{
    auto* segment = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);
    {
        stateMngr.BindSamplers(
            segment->first,
            segment->count,
            reinterpret_cast<const GLuint*>(heapPtr + sizeof(GLResourceHeapSegment))
        );
    }
    return segment->size;
}

static std::size_t BindTexturesWithEmulatedSamplersSegment(GLStateManager& stateMngr, const char* heapPtr)
{
    auto* segment = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);
    {
        const auto* texturesGL   = reinterpret_cast<GLTexture* const *>(heapPtr + sizeof(GLResourceHeapSegment));
        const auto* samplersGL2X = reinterpret_cast<const GLEmulatedSampler* const *>(heapPtr + segment->data1Offset);
        for_range(i, static_cast<GLuint>(segment->count))
        {
            const GLuint layer = segment->first + i;
            stateMngr.BindCombinedEmulatedSampler(layer, *samplersGL2X[i], *texturesGL[i]);
        }
    }
    return segment->size;
}

std::uint32_t GLResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(heap_.NumSets());
}

void GLResourceHeap::Bind(GLStateManager& stateMngr, std::uint32_t descriptorSet)
{
    if (descriptorSet >= heap_.NumSets())
        return;

    char* heapPtr = heap_.SegmentData(descriptorSet);

    /* Bind all constant buffers */
    for_range(i, segmentation_.numUniformBufferSegments)
        heapPtr += BindBuffersSegment(stateMngr, heapPtr, GLBufferTarget::UniformBuffer);

    /* Bind all shader storage buffers */
    for_range(i, segmentation_.numStorageBufferSegments)
        heapPtr += BindBuffersSegment(stateMngr, heapPtr, GLBufferTarget::ShaderStorageBuffer);

    if (!HasNativeSamplers())
    {
        /* Bind all textures */
        for_range(i, segmentation_.numTextureSegments)
            heapPtr += BindTexturesWithEmulatedSamplersSegment(stateMngr, heapPtr);
    }
    else
    {
        /* Bind all textures */
        for_range(i, segmentation_.numTextureSegments)
            heapPtr += BindTexturesSegment(stateMngr, heapPtr);

        /* Bind all image texture units */
        for_range(i, segmentation_.numImageTextureSegments)
            heapPtr += BindImageTexturesSegment(stateMngr, heapPtr);

        /* Bind all samplers */
        for_range(i, segmentation_.numSamplerSegments)
            heapPtr += BindSamplersSegment(stateMngr, heapPtr);
    }
}


/*
 * ======= Private: =======
 */

#define BIND_SEGMENT_ALLOCATOR(FUNC, TYPE, ...)         \
    std::bind(                                          \
        &FUNC,                                          \
        this,                                           \
        TYPE,                                           \
        std::placeholders::_1, std::placeholders::_2,   \
        __VA_ARGS__                                     \
    )

void GLResourceHeap::AllocTextureView(GLuint& texViewID, GLuint sourceTexID, const TextureViewDescriptor& textureViewDesc)
{
    if (texViewID != 0)
        GLTextureViewPool::Get().ReleaseTextureView(texViewID);
    texViewID = GLTextureViewPool::Get().CreateTextureView(sourceTexID, textureViewDesc);
}

bool GLResourceHeap::FreeTextureView(GLuint& texViewID)
{
    if (texViewID != 0)
    {
        GLTextureViewPool::Get().ReleaseTextureView(texViewID);
        texViewID = 0;
        return true;
    }
    return false;
}

static std::uint32_t FreeSegmentTextureViews(const char* heapPtr)
{
    auto* segment = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);
    if ((segment->flags & GLResourceFlags_HasTextureViews) != 0)
    {
        for_range(i, segment->count)
        {
            GLuint texViewID = GLRESOURCEHEAP_DATA2(heapPtr, const GLuint)[i];
            if (texViewID != 0)
                GLTextureViewPool::Get().ReleaseTextureView(texViewID);
        }
    }
    return segment->size;
}

void GLResourceHeap::FreeAllSegmentSetTextureViews(const char* heapPtr)
{
    /* Jump over buffer segments */
    for_range(i, segmentation_.numUniformBufferSegments)
        heapPtr += GLRESOURCEHEAP_CONST_SEGMENT(heapPtr)->size;
    for_range(i, segmentation_.numStorageBufferSegments)
        heapPtr += GLRESOURCEHEAP_CONST_SEGMENT(heapPtr)->size;

    /* Free texture views in texture segments */
    for_range(i, segmentation_.numTextureSegments)
        heapPtr += FreeSegmentTextureViews(heapPtr);
    for_range(i, segmentation_.numImageTextureSegments)
        heapPtr += FreeSegmentTextureViews(heapPtr);
}

void GLResourceHeap::FreeAllSegmentsTextureViews()
{
    for (char* heapPtr = heap_.Data(); heapPtr != heap_.PayloadData(); heapPtr += heap_.Stride())
        FreeAllSegmentSetTextureViews(heapPtr);
}

void GLResourceHeap::AllocSegmentsUBO(BindingDescriptorIterator& bindingIter)
{
    /* Collect all uniform buffers */
    auto bindingSlots = FilterAndSortGLBindingSlots(bindingIter, ResourceType::Buffer, BindFlags::ConstantBuffer);

    /* Build all resource segments for type <GLResourceHeap3PartSegment> */
    segmentation_.numUniformBufferSegments = GLResourceHeap::ConsolidateSegments(
        bindingSlots,
        BIND_SEGMENT_ALLOCATOR(GLResourceHeap::Alloc3PartSegment, GLResourceType_UBO, sizeof(GLuint), sizeof(GLintptr), sizeof(GLsizeiptr))
    );
}

void GLResourceHeap::AllocSegmentsSSBO(BindingDescriptorIterator& bindingIter)
{
    /* Collect all shader storage buffers */
    auto bindingSlots = FilterAndSortGLBindingSlots(bindingIter, ResourceType::Buffer, (BindFlags::Sampled | BindFlags::Storage));

    /* Build all resource segments for type <GLResourceHeap3PartSegment> */
    segmentation_.numStorageBufferSegments = GLResourceHeap::ConsolidateSegments(
        bindingSlots,
        BIND_SEGMENT_ALLOCATOR(GLResourceHeap::Alloc3PartSegment, GLResourceType_SSBO, sizeof(GLuint), sizeof(GLintptr), sizeof(GLsizeiptr))
    );
}

void GLResourceHeap::AllocSegmentsTexture(BindingDescriptorIterator& bindingIter)
{
    /* If native samplers are not supported, all texture bindings are handled via the emulated sampler bindings; see AllocSegmentsEmulatedSampler() */
    if (!HasNativeSamplers())
        return;

    /* Collect all textures with sampled binding */
    auto bindingSlots = FilterAndSortGLBindingSlots(bindingIter, ResourceType::Texture, BindFlags::Sampled);

    /* Build all resource segments for type <GLResourceHeapSegment> */
    segmentation_.numTextureSegments = GLResourceHeap::ConsolidateSegments(
        bindingSlots,
        BIND_SEGMENT_ALLOCATOR(GLResourceHeap::Alloc3PartSegment, GLResourceType_Texture, sizeof(GLuint), sizeof(GLTextureTarget), sizeof(GLuint))
    );
}

void GLResourceHeap::AllocSegmentsImage(BindingDescriptorIterator& bindingIter)
{
    /* Collect all textures with storage binding */
    auto bindingSlots = FilterAndSortGLBindingSlots(bindingIter, ResourceType::Texture, BindFlags::Storage);

    /* Build all resource segments for type <GLResourceHeapSegment> */
    segmentation_.numImageTextureSegments = GLResourceHeap::ConsolidateSegments(
        bindingSlots,
        BIND_SEGMENT_ALLOCATOR(GLResourceHeap::Alloc2PartSegment, GLResourceType_Image, sizeof(GLuint), sizeof(GLenum))
    );
}

void GLResourceHeap::AllocSegmentsSampler(BindingDescriptorIterator& bindingIter)
{
    if (HasNativeSamplers())
        AllocSegmentsNativeSampler(bindingIter);
    else
        AllocSegmentsEmulatedSampler(bindingIter);
}

void GLResourceHeap::AllocSegmentsNativeSampler(BindingDescriptorIterator& bindingIter)
{
    /* Collect all samplers */
    auto bindingSlots = FilterAndSortGLBindingSlots(bindingIter, ResourceType::Sampler, 0);

    /* Allocate all resource segments for type <GLResourceHeap1PartSegment> */
    segmentation_.numSamplerSegments = GLResourceHeap::ConsolidateSegments(
        bindingSlots,
        BIND_SEGMENT_ALLOCATOR(GLResourceHeap::Alloc1PartSegment, GLResourceType_Sampler, sizeof(GLuint))
    );
}

void GLResourceHeap::AllocSegmentsEmulatedSampler(BindingDescriptorIterator& bindingIter)
{
    /* Collect all textures with sampled binding */
    auto textureBindingSlots = FilterAndSortGLBindingSlots(bindingIter, ResourceType::Texture, BindFlags::Sampled);

    /* Allocate all resource segments for type <GLResourceHeapSegment> */
    segmentation_.numTextureSegments = GLResourceHeap::ConsolidateSegments(
        textureBindingSlots,
        BIND_SEGMENT_ALLOCATOR(GLResourceHeap::Alloc2PartSegment, GLResourceType_EmulatedSampler, sizeof(GLTexture*), sizeof(const GLEmulatedSampler*))
    );

    /* Collect all samplers states */
    auto samplerBindingSlots = FilterAndSortGLBindingSlots(bindingIter, ResourceType::Sampler, 0);

    /* Ensure there is exactly one sampler for each texture */
    if (samplerBindingSlots.size() != textureBindingSlots.size())
    {
        LLGL_TRAP(
            "cannot create GL resource heap with mismatching number of emulated samplers (%zu) and textures (%zu)",
            samplerBindingSlots.size(), textureBindingSlots.size()
        );
    }

    /* Ensure all samplers are distributed onto the same binding slots as the textures */
    for (GLResourceBinding& samplerBinding : samplerBindingSlots)
    {
        /* Find corresponding texture binding */
        const GLResourceBinding* textureBinding = FindInSortedArray<GLResourceBinding>(
            textureBindingSlots.data(),
            textureBindingSlots.size(),
            [&samplerBinding](const GLResourceBinding& entry) -> int
            {
                return (static_cast<int>(samplerBinding.slot) - static_cast<int>(entry.slot));
            }
        );
        if (textureBinding != nullptr)
        {
            /* Copy binding segment location from texture to combine with sampler */
            CopyBindingMapping(samplerBinding, *textureBinding);
        }
        else
        {
            LLGL_TRAP(
                "cannot create GL resource heap with missing texture for emulated sampler at slot %u",
                samplerBinding.slot
            );
        }
    }
}

void GLResourceHeap::Alloc1PartSegment(
    GLResourceType              type,
    const GLResourceBinding*    first,
    SegmentationSizeType        count,
    std::size_t                 payload0Stride)
{
    /* Write binding map entries */
    WriteBindingMappings(first, count);

    /* Allocate space for segment */
    const std::uint32_t payloadSize     = static_cast<std::uint32_t>(payload0Stride * count);
    auto                segmentAlloc    = heap_.AllocSegment<GLResourceHeapSegment>(payloadSize);

    /* Write segment header */
    GLResourceHeapSegment* header = segmentAlloc.Header();
    {
        header->size        = segmentAlloc.Size();
        header->type        = type;
        header->first       = first->slot;
        header->count       = static_cast<GLsizei>(count);
        header->data1Offset = 0;
        header->data2Offset = 0;
    }
}

void GLResourceHeap::Alloc2PartSegment(
    GLResourceType              type,
    const GLResourceBinding*    first,
    SegmentationSizeType        count,
    std::size_t                 payload0Stride,
    std::size_t                 payload1Stride)
{
    /* Write binding map entries */
    WriteBindingMappings(first, count);

    /* Allocate space for segment */
    const std::uint32_t payloadData1Offset  = static_cast<std::uint32_t>(payload0Stride * count);
    const std::uint32_t payloadSize         = static_cast<std::uint32_t>(payload1Stride * count + payloadData1Offset);
    auto                segmentAlloc        = heap_.AllocSegment<GLResourceHeapSegment>(payloadSize);

    /* Write segment header */
    GLResourceHeapSegment* header = segmentAlloc.Header();
    {
        header->size        = segmentAlloc.Size();
        header->type        = type;
        header->first       = first->slot;
        header->count       = static_cast<GLsizei>(count);
        header->data1Offset = segmentAlloc.PayloadOffset() + payloadData1Offset;
        header->data2Offset = 0;
    }
}

void GLResourceHeap::Alloc3PartSegment(
    GLResourceType              type,
    const GLResourceBinding*    first,
    SegmentationSizeType        count,
    std::size_t                 payload0Stride,
    std::size_t                 payload1Stride,
    std::size_t                 payload2Stride)
{
    /* Write binding map entries */
    WriteBindingMappings(first, count);

    /* Allocate space for segment */
    const std::uint32_t payloadData1Offset  = static_cast<std::uint32_t>(payload0Stride * count);
    const std::uint32_t payloadData2Offset  = static_cast<std::uint32_t>(payload1Stride * count + payloadData1Offset);
    const std::uint32_t payloadSize         = static_cast<std::uint32_t>(payload2Stride * count + payloadData2Offset);
    auto                segmentAlloc        = heap_.AllocSegment<GLResourceHeapSegment>(payloadSize);

    /* Write segment header */
    GLResourceHeapSegment* header = segmentAlloc.Header();
    {
        header->size        = segmentAlloc.Size();
        header->type        = type;
        header->first       = first->slot;
        header->count       = static_cast<GLsizei>(count);
        header->data1Offset = segmentAlloc.PayloadOffset() + payloadData1Offset;
        header->data2Offset = segmentAlloc.PayloadOffset() + payloadData2Offset;
    }
}

void GLResourceHeap::WriteBindingMappings(const GLResourceBinding* first, SegmentationSizeType count)
{
    for_range(i, count)
    {
        LLGL_ASSERT(first[i].index < bindingMap_.size());
        BindingSegmentLocation& mapping = bindingMap_[first[i].index];
        mapping.segmentOffset   = static_cast<std::uint32_t>(heap_.Size());
        mapping.descriptorIndex = i;
    }
}

void GLResourceHeap::CopyBindingMapping(const GLResourceBinding& dst, const GLResourceBinding& src)
{
    LLGL_ASSERT(dst.index < bindingMap_.size());
    LLGL_ASSERT(src.index < bindingMap_.size());
    bindingMap_[dst.index] = bindingMap_[src.index];
}

void GLResourceHeap::WriteResourceViewBuffer(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index, long anyBindFlags)
{
    /* Get buffer resource and its size parameter */
    auto* bufferGL = LLGL_CAST(GLBuffer*, GetAsExpectedBuffer(desc.resource, anyBindFlags));

    GLint bufferSize = 0;
    bufferGL->GetBufferParams(&bufferSize, nullptr, nullptr);

    /* Write buffer ID to segment (GLuint) */
    GLRESOURCEHEAP_DATA0(heapPtr, GLuint)[index] = bufferGL->GetID();

    /* Write buffer offset and length to segment (GLintptr, GLsizeiptr) */
    if (IsGLBufferViewEnabled(desc.bufferView))
    {
        /* If one buffer view uses a buffe range, the whole segment must be bound with ranged buffers */
        GLRESOURCEHEAP_SEGMENT(heapPtr)->flags |= GLResourceFlags_HasBufferRange;

        GLRESOURCEHEAP_DATA1(heapPtr, GLintptr  )[index] = static_cast<GLintptr>(desc.bufferView.offset);
        GLRESOURCEHEAP_DATA2(heapPtr, GLsizeiptr)[index] = static_cast<GLsizeiptr>(desc.bufferView.size);
    }
    else
    {
        GLRESOURCEHEAP_DATA1(heapPtr, GLintptr  )[index] = 0;
        GLRESOURCEHEAP_DATA2(heapPtr, GLsizeiptr)[index] = static_cast<GLsizeiptr>(bufferSize);
    }
}

void GLResourceHeap::WriteResourceViewUBO(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index)
{
    WriteResourceViewBuffer(desc, heapPtr, index, BindFlags::ConstantBuffer);
}

void GLResourceHeap::WriteResourceViewSSBO(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index)
{
    WriteResourceViewBuffer(desc, heapPtr, index, (BindFlags::Sampled | BindFlags::Storage));
}

// Returns true if the segment at the specified heap position contains any texture views.
static bool HasTextureSegmentAnyTextureViews(const char* heapPtr)
{
    const GLResourceHeapSegment* segment = GLRESOURCEHEAP_CONST_SEGMENT(heapPtr);
    for_range(i, segment->count)
    {
        if (GLRESOURCEHEAP_DATA2(heapPtr, const GLuint)[i] != 0)
            return true;
    }
    return false;
}

// Updates the segment flags for the specified heap position if any texture views have been added or removed from the segment.
static void UpdateTextureSegmentFlags(char* heapPtr, bool isAnyTextureViewAdded, bool isAnyTextureViewRemoved)
{
    GLResourceHeapSegment* segment = GLRESOURCEHEAP_SEGMENT(heapPtr);
    if (isAnyTextureViewAdded)
    {
        /* Mark segment to have texture views */
        segment->flags |= GLResourceFlags_HasTextureViews;
    }
    else if (isAnyTextureViewRemoved)
    {
        /* Count texture views in segment and remove marker if there are no one left */
        if ((segment->flags & GLResourceFlags_HasTextureViews) != 0 && !HasTextureSegmentAnyTextureViews(heapPtr))
            segment->flags &= (~GLResourceFlags_HasTextureViews);
    }
}

void GLResourceHeap::WriteResourceViewTexture(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index)
{
    bool isAnyTextureViewAdded = false;
    bool isAnyTextureViewRemoved = false;

    /* Get texture resource and its size parameter */
    auto* textureGL = LLGL_CAST(GLTexture*, GetAsExpectedTexture(desc.resource, BindFlags::Sampled));

    if (IsTextureViewEnabled(desc.textureView))
    {
        /* Allocate new texture view */
        AllocTextureView(GLRESOURCEHEAP_DATA2(heapPtr, GLuint)[index], textureGL->GetID(), desc.textureView);
        isAnyTextureViewAdded = true;

        /* Write texture ID to segment (GLuint, GLTextureTarget) */
        GLRESOURCEHEAP_DATA0(heapPtr, GLuint         )[index] = GLRESOURCEHEAP_DATA2(heapPtr, GLuint)[index];
        GLRESOURCEHEAP_DATA1(heapPtr, GLTextureTarget)[index] = GLStateManager::GetTextureTarget(desc.textureView.type);
    }
    else
    {
        /* Release old texture if it was a texture view */
        if (FreeTextureView(GLRESOURCEHEAP_DATA2(heapPtr, GLuint)[index]))
            isAnyTextureViewRemoved = true;

        /* Write texture ID to segment (GLuint, GLTextureTarget) */
        GLRESOURCEHEAP_DATA0(heapPtr, GLuint         )[index] = textureGL->GetID();
        GLRESOURCEHEAP_DATA1(heapPtr, GLTextureTarget)[index] = GLStateManager::GetTextureTarget(textureGL->GetType());
    }

    /* Update flags for segment if texture views have been added or removed */
    UpdateTextureSegmentFlags(heapPtr, isAnyTextureViewAdded, isAnyTextureViewRemoved);
}

void GLResourceHeap::WriteResourceViewImage(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index)
{
    bool isAnyTextureViewAdded = false;
    bool isAnyTextureViewRemoved = false;

    /* Get texture resource and its size parameter */
    auto* textureGL = LLGL_CAST(GLTexture*, GetAsExpectedTexture(desc.resource, BindFlags::Sampled));

    if (IsTextureViewEnabled(desc.textureView))
    {
        /* Allocate new texture view */
        AllocTextureView(GLRESOURCEHEAP_DATA2(heapPtr, GLuint)[index], textureGL->GetID(), desc.textureView);
        isAnyTextureViewAdded = true;

        /* Write texture ID to segment (GLuint, GLenum) */
        GLRESOURCEHEAP_DATA0(heapPtr, GLuint)[index] = GLRESOURCEHEAP_DATA2(heapPtr, GLuint)[index];
        GLRESOURCEHEAP_DATA1(heapPtr, GLenum)[index] = GLTypes::Map(desc.textureView.format);
    }
    else
    {
        /* Release old texture if it was a texture view */
        if (FreeTextureView(GLRESOURCEHEAP_DATA2(heapPtr, GLuint)[index]))
            isAnyTextureViewRemoved = true;

        /* Write texture ID to segment (GLuint, GLenum) */
        GLRESOURCEHEAP_DATA0(heapPtr, GLuint)[index] = textureGL->GetID();
        GLRESOURCEHEAP_DATA1(heapPtr, GLenum)[index] = textureGL->GetGLInternalFormat();
    }

    /* Update flags for segment if texture views have been added or removed */
    UpdateTextureSegmentFlags(heapPtr, isAnyTextureViewAdded, isAnyTextureViewRemoved);
}

void GLResourceHeap::WriteResourceViewSampler(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index)
{
    /* Get sampler resource and write sampler ID to segment (GLuint) */
    auto* samplerGL = LLGL_CAST(GLSampler*, GetAsExpectedSampler(desc.resource));
    GLRESOURCEHEAP_DATA0(heapPtr, GLuint)[index] = samplerGL->GetID();
}

void GLResourceHeap::WriteResourceViewEmulatedSampler(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index)
{
    /* Combine texture and sampler into same segment entry */
    if (desc.resource != nullptr && desc.resource->GetResourceType() == ResourceType::Sampler)
    {
        /* Get sampler resource and write sampler reference to segment (const GLEmulatedSampler*) */
        auto* emulatedSamplerGL = LLGL_CAST(const GLEmulatedSampler*, GetAsExpectedSampler(desc.resource));
        GLRESOURCEHEAP_DATA1(heapPtr, const GLEmulatedSampler*)[index] = emulatedSamplerGL;
    }
    else
    {
        /* Get texture resource and write texture reference to segment (GLTexture*) */
        auto* textureGL = LLGL_CAST(GLTexture*, GetAsExpectedTexture(desc.resource));
        GLRESOURCEHEAP_DATA0(heapPtr, GLTexture*)[index] = textureGL;
    }
}

std::vector<GLResourceHeap::GLResourceBinding> GLResourceHeap::FilterAndSortGLBindingSlots(
    BindingDescriptorIterator&  bindingIter,
    ResourceType                resourceType,
    long                        resourceBindFlags)
{
    /* Collect all binding points of the specified resource type */
    bindingIter.Reset(resourceType, resourceBindFlags);

    std::vector<GLResourceBinding> resourceBindings;
    resourceBindings.reserve(bindingIter.GetCount());

    for (std::size_t index = 0; const BindingDescriptor* bindingDesc = bindingIter.Next(&index);)
        resourceBindings.push_back({ static_cast<GLuint>(bindingDesc->slot.index), index });

    /* Sort resources by slot index */
    std::sort(
        resourceBindings.begin(),
        resourceBindings.end(),
        [](const GLResourceBinding& lhs, const GLResourceBinding& rhs) -> bool
        {
            return (lhs.slot < rhs.slot);
        }
    );

    return resourceBindings;
}

GLResourceHeap::SegmentationSizeType GLResourceHeap::ConsolidateSegments(
    const ArrayView<GLResourceBinding>& bindingSlots,
    const AllocSegmentFunc&             allocSegmentFunc)
{
    return ConsolidateConsecutiveSequences<SegmentationSizeType>(
        bindingSlots.begin(),
        bindingSlots.end(),
        allocSegmentFunc,
        [](const GLResourceBinding& entry) -> GLuint
        {
            return entry.slot;
        }
    );
}

#undef BIND_SEGMENT_ALLOCATOR
#undef GLRESOURCEHEAP_SEGMENT
#undef GLRESOURCEHEAP_CONST_SEGMENT
#undef GLRESOURCEHEAP_DATA0
#undef GLRESOURCEHEAP_DATA1
#undef GLRESOURCEHEAP_DATA2


} // /namespace LLGL



// ================================================================================
