/*
 * GLResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLResourceHeap.h"
#include "GLPipelineLayout.h"
#include "GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Buffer/GLBuffer.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLTexture.h"
#include "../Texture/GLTextureViewPool.h"
#include "../../CheckedCast.h"
#include "../../ResourceBindingIterator.h"
#include "../GLTypes.h"
#include <LLGL/ResourceHeapFlags.h>
#include <string.h>


namespace LLGL
{


/*
 * Internal structures
 */

/*

The internal buffer of GLResourceHeap is tightly packed which stores all segments of binding points consecutively.
Here is an illustration of the buffer layout for one Texture resouce (at binding point 4) and two Sampler resources (at binding points 5 and 6) on a 32-bit build:

Offset      Attribute                               Value   Description                                         Segment
--------------------------------------------------------------------------------------------------------------------------------------------
0x00000000  GLResourceViewHeapSegment2::segmentSize    24   Size of this segment                                \
0x00000004  GLResourceViewHeapSegment2::offsetEnd0     20   Relative offset to texture[0] (at 0x00000014)        |
0x00000008  GLResourceViewHeapSegment2::first           4   First binding point                                  |-- Texture segment
0x0000000C  GLResourceViewHeapSegment2::count           1   Number of binding points                             |
0x00000010  target[0]                                   1   Texture target (GLTextureTarget::TEXTURE_2D = 1)     |
0x00000014  texture[0]                                  1   1st OpenGL texture ID (from 'glGenTextures')        /
0x00000018  GLResourceViewHeapSegment1::segmentSize    20   Size of this segment                                \
0x0000001C  GLResourceViewHeapSegment1::first           5   First binding point                                  |
0x00000020  GLResourceViewHeapSegment1::count           2   Number of binding points                             |-- Sampler segment
0x00000024  sampler[0]                                  1   1st OpenGL sampler ID (from 'glGenSamplers')         |
0x00000028  sampler[1]                                  2   2nd OpenGL sampler ID (from 'glGenSamplers')        /

*/

// Resource view heap (RVH) segment structure with one dynamic sub-buffer for <GLuint>
struct GLResourceViewHeapSegment1
{
    std::size_t segmentSize;    // TODO: maybe use std::uint16_t for optimization
    GLuint      first;          // TODO: maybe use std::uint8_t for optimization
    GLsizei     count;          // TODO: maybe use std::uint8_t for optimization
};

// Resource view heap (RVH) segment structure with two dynamic sub-buffers, one for <GLTextureTarget> and one for <GLuint>; used for 'GLStateManager::BindTextures'.
struct GLResourceViewHeapSegment2
{
    std::size_t segmentSize;
    std::size_t offsetEnd0; // Byte offset after the first sub-buffer, following the second sub-buffer
    GLuint      first;
    GLsizei     count;
};

// Resource view heap (RVH) segment structure with three dynamic sub-buffers, one for <GLuint>, one for <GLintptr>, and one for <GLsizeiptr>; used for 'GLStateManager::BindBuffersRange'.
struct GLResourceViewHeapSegment3
{
    std::size_t segmentSize;
    std::size_t offsetEnd0; // Byte offset after the first sub-buffer, following the second sub-buffer
    std::size_t offsetEnd1; // Byte offset after the second sub-buffer, following the third sub-buffer
    GLuint      first;
    GLsizei     count;
};

// Helper struct to gather resource binding information for all segment types
struct GLResourceBinding
{
    GLuint          slot;
    GLuint          object;
    GLTextureTarget target; // Only used for textures and image texture units
    GLenum          format; // Only used for image texture units
    GLintptr        offset; // Only used for buffer ranges
    GLsizeiptr      size;   // Only used for buffer ranges
};


/*
 * Internal functions
 */

// Returns true if the specified buffer view is enabled for OpenGL bindings
static bool IsGLBufferViewEnabled(const BufferViewDescriptor& bufferViewDesc)
{
    /* For OpenGL buffer binding, only the range is relevant, no format is considerd */
    return (bufferViewDesc.size != Constants::wholeSize);
}

#ifdef GL_ARB_shader_image_load_store

// Returns the bitfield of <glMemoryBarrier> for the specified resources
static GLbitfield GetMemoryBarrierBitfield(const std::vector<ResourceViewDescriptor>& resourceViews)
{
    GLbitfield barriers = 0;

    for (const auto& desc : resourceViews)
    {
        if (auto resource = desc.resource)
        {
            /* Enable <GL_SHADER_STORAGE_BARRIER_BIT> bitmask for UAV buffers */
            if (resource->GetResourceType() == ResourceType::Buffer)
            {
                auto buffer = LLGL_CAST(Buffer*, resource);
                if ((buffer->GetBindFlags() & BindFlags::Storage) != 0)
                    barriers |= GL_SHADER_STORAGE_BARRIER_BIT;
            }
        }
    }

    return barriers;
}

#endif // /GL_ARB_shader_image_load_store

// Returns the resource of the specified descriptor as <GLTexture> if it describes a texture-view.
static GLTexture* GetAsTextureView(const ResourceViewDescriptor& rvDesc)
{
    if (IsTextureViewEnabled(rvDesc.textureView))
    {
        if (auto* resource = rvDesc.resource)
        {
            if (resource->GetResourceType() == ResourceType::Texture)
                return LLGL_CAST(GLTexture*, resource);
        }
    }
    return nullptr;
}


/*
 * GLResourceHeap class
 */

GLResourceHeap::GLResourceHeap(const ResourceHeapDescriptor& desc)
{
    /* Get pipeline layout object */
    auto pipelineLayoutGL = LLGL_CAST(GLPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutGL)
        throw std::invalid_argument("failed to create resource heap due to missing pipeline layout");

    /* Validate binding descriptors */
    const auto& bindings            = pipelineLayoutGL->GetBindings();
    const auto  numBindings         = bindings.size();
    const auto  numResourceViews    = desc.resourceViews.size();

    if (numBindings == 0)
        throw std::invalid_argument("cannot create resource heap without bindings in pipeline layout");
    if (numResourceViews % numBindings != 0)
        throw std::invalid_argument("failed to create resource heap because due to mismatch between number of resources and bindings");

    /* Determine memory barriers */
    #ifdef GL_ARB_shader_image_load_store
    barriers_ = GetMemoryBarrierBitfield(desc.resourceViews);
    #else
    barriers_ = 0;
    #endif // /GL_ARB_shader_image_load_store

    /* Create all texture views */
    for (std::size_t i = 0; i < numResourceViews; i += numBindings)
    {
        ResourceBindingIterator resourceIterator{ desc.resourceViews, bindings, i };
        BuildTextureViews(resourceIterator, BindFlags::Sampled);
        BuildTextureViews(resourceIterator, BindFlags::Storage);
    }

    /* Build all resource view segments */
    for (std::size_t i = 0; i < numResourceViews; i += numBindings)
    {
        /* Reset segment header, only one is required */
        ResourceBindingIterator resourceIterator{ desc.resourceViews, bindings, i };
        ::memset(&segmentation_, 0, sizeof(segmentation_));

        /* Build resource view segments for current descriptor set */
        BuildUniformBufferSegments(resourceIterator);
        BuildStorageBufferSegments(resourceIterator);
        BuildTextureSegments(resourceIterator);
        BuildImageTextureSegments(resourceIterator);
        BuildSamplerSegments(resourceIterator);
    }

    /* Store buffer stride */
    stride_ = GetSegmentationHeapSize() / (numResourceViews / numBindings);
}

GLResourceHeap::~GLResourceHeap()
{
    /* Release all texture views for this resource heap */
    const GLuint* textureViewIDs = reinterpret_cast<const GLuint*>(buffer_.data());
    for (std::size_t i = 0; i < numTextureViews_; ++i)
        GLTextureViewPool::Get().ReleaseTextureView(textureViewIDs[i]);
}

static void BindBuffersBaseSegment(GLStateManager& stateMngr, const std::int8_t*& byteAlignedBuffer, const GLBufferTarget bufferTarget)
{
    const auto segment = reinterpret_cast<const GLResourceViewHeapSegment1*>(byteAlignedBuffer);
    {
        stateMngr.BindBuffersBase(
            bufferTarget,
            segment->first,
            segment->count,
            reinterpret_cast<const GLuint*>(byteAlignedBuffer + sizeof(GLResourceViewHeapSegment1))
        );
    }
    byteAlignedBuffer += segment->segmentSize;
}

static void BindBuffersRangeSegment(GLStateManager& stateMngr, const std::int8_t*& byteAlignedBuffer, const GLBufferTarget bufferTarget)
{
    const auto segment = reinterpret_cast<const GLResourceViewHeapSegment3*>(byteAlignedBuffer);
    {
        stateMngr.BindBuffersRange(
            bufferTarget,
            segment->first,
            segment->count,
            reinterpret_cast<const GLuint*>(byteAlignedBuffer + sizeof(GLResourceViewHeapSegment3)),
            reinterpret_cast<const GLintptr*>(byteAlignedBuffer + segment->offsetEnd0),
            reinterpret_cast<const GLsizeiptr*>(byteAlignedBuffer + segment->offsetEnd1)
        );
    }
    byteAlignedBuffer += segment->segmentSize;
}

static void BindTexturesSegment(GLStateManager& stateMngr, const std::int8_t*& byteAlignedBuffer)
{
    const auto segment = reinterpret_cast<const GLResourceViewHeapSegment2*>(byteAlignedBuffer);
    {
        stateMngr.BindTextures(
            segment->first,
            segment->count,
            reinterpret_cast<const GLTextureTarget*>(byteAlignedBuffer + sizeof(GLResourceViewHeapSegment2)),
            reinterpret_cast<const GLuint*>(byteAlignedBuffer + segment->offsetEnd0)
        );
    }
    byteAlignedBuffer += segment->segmentSize;
}

static void BindImageTexturesSegment(GLStateManager& stateMngr, const std::int8_t*& byteAlignedBuffer)
{
    const auto segment = reinterpret_cast<const GLResourceViewHeapSegment2*>(byteAlignedBuffer);
    {
        stateMngr.BindImageTextures(
            segment->first,
            segment->count,
            reinterpret_cast<const GLenum*>(byteAlignedBuffer + sizeof(GLResourceViewHeapSegment2)),
            reinterpret_cast<const GLuint*>(byteAlignedBuffer + segment->offsetEnd0)
        );
    }
    byteAlignedBuffer += segment->segmentSize;
}

static void BindSamplersSegment(GLStateManager& stateMngr, const std::int8_t*& byteAlignedBuffer)
{
    const auto segment = reinterpret_cast<const GLResourceViewHeapSegment1*>(byteAlignedBuffer);
    {
        stateMngr.BindSamplers(
            segment->first,
            segment->count,
            reinterpret_cast<const GLuint*>(byteAlignedBuffer + sizeof(GLResourceViewHeapSegment1))
        );
    }
    byteAlignedBuffer += segment->segmentSize;
}

std::uint32_t GLResourceHeap::GetNumDescriptorSets() const
{
    return static_cast<std::uint32_t>(stride_ > 0 ? GetSegmentationHeapSize() / stride_ : 0);
}

void GLResourceHeap::Bind(GLStateManager& stateMngr, std::uint32_t firstSet)
{
    auto byteAlignedBuffer = GetSegmentationHeapStart(firstSet);

    #ifdef GL_ARB_shader_image_load_store

    /* Submit memory barriers for UAVs */
    if (barriers_ != 0)
        glMemoryBarrier(barriers_);

    #endif // /GL_ARB_shader_image_load_store

    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentation_.numUniformBufferSegments; ++i)
        BindBuffersBaseSegment(stateMngr, byteAlignedBuffer, GLBufferTarget::UNIFORM_BUFFER);
    for (std::uint8_t i = 0; i < segmentation_.numUniformBufferRangeSegments; ++i)
        BindBuffersRangeSegment(stateMngr, byteAlignedBuffer, GLBufferTarget::UNIFORM_BUFFER);

    /* Bind all shader storage buffers */
    for (std::uint8_t i = 0; i < segmentation_.numStorageBufferSegments; ++i)
        BindBuffersBaseSegment(stateMngr, byteAlignedBuffer, GLBufferTarget::SHADER_STORAGE_BUFFER);
    for (std::uint8_t i = 0; i < segmentation_.numStorageBufferRangeSegments; ++i)
        BindBuffersRangeSegment(stateMngr, byteAlignedBuffer, GLBufferTarget::SHADER_STORAGE_BUFFER);

    /* Bind all textures */
    for (std::uint8_t i = 0; i < segmentation_.numTextureSegments; ++i)
        BindTexturesSegment(stateMngr, byteAlignedBuffer);

    /* Bind all image texture units */
    for (std::uint8_t i = 0; i < segmentation_.numImageTextureSegments; ++i)
        BindImageTexturesSegment(stateMngr, byteAlignedBuffer);

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentation_.numSamplerSegments; ++i)
        BindSamplersSegment(stateMngr, byteAlignedBuffer);
}


/*
 * ======= Private: =======
 */

using GLResourceBindingFunc = std::function<
    void(
        GLResourceBinding&              binding,
        Resource*                       resource,
        const ResourceViewDescriptor&   rvDesc,
        std::uint32_t                   slot
    )
>;

static std::vector<GLResourceBinding> CollectGLResourceBindings(
    ResourceBindingIterator&        resourceIterator,
    ResourceType                    resourceType,
    long                            resourceBindFlags,
    const GLResourceBindingFunc&    bindingFunc)
{
    /* Collect all binding points of the specified resource type */
    const BindingDescriptor* bindingDesc = nullptr;
    const ResourceViewDescriptor* rvDesc = nullptr;
    resourceIterator.Reset(resourceType, resourceBindFlags);

    std::vector<GLResourceBinding> resourceBindings;
    resourceBindings.reserve(resourceIterator.GetCount());

    while (auto resource = resourceIterator.Next(&bindingDesc, &rvDesc))
    {
        GLResourceBinding binding = {};
        bindingFunc(binding, resource, *rvDesc, bindingDesc->slot);
        resourceBindings.push_back(binding);
    }

    /* Sort resources by slot index */
    std::sort(
        resourceBindings.begin(), resourceBindings.end(),
        [](const GLResourceBinding& lhs, const GLResourceBinding& rhs)
        {
            return (lhs.slot < rhs.slot);
        }
    );

    return resourceBindings;
}

// Returns true if the specified resource bindings require a buffer range instead of the entire buffer
static bool RequiresRangeForGLResourceBindings(
    ResourceBindingIterator&        resourceIterator,
    ResourceType                    resourceType,
    long                            resourceBindFlags)
{
    /* Collect all binding points of the specified resource type */
    const ResourceViewDescriptor* rvDesc = nullptr;
    resourceIterator.Reset(resourceType, resourceBindFlags);

    std::vector<GLResourceBinding> resourceBindings;
    resourceBindings.reserve(resourceIterator.GetCount());

    while (resourceIterator.Next(nullptr, &rvDesc) != nullptr)
    {
        if (IsGLBufferViewEnabled(rvDesc->bufferView))
            return true;
    }

    return false;
}

void GLResourceHeap::BuildTextureViews(ResourceBindingIterator& resourceIterator, long bindFlags)
{
    const ResourceViewDescriptor* rvDesc = nullptr;
    resourceIterator.Reset(ResourceType::Texture, bindFlags);
    while (auto resource = resourceIterator.Next(nullptr, &rvDesc))
    {
        if (auto textureGL = GetAsTextureView(*rvDesc))
        {
            GLuint texID = GLTextureViewPool::Get().CreateTextureView(textureGL->GetID(), rvDesc->textureView);
            WriteSegmentationHeapEnd(&texID, sizeof(texID));
        }
    }
}

void GLResourceHeap::BuildBufferSegments(ResourceBindingIterator& resourceIterator, long bindFlags, std::uint8_t& numSegments)
{
    /* Collect all buffers */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::Buffer,
        bindFlags,
        [](GLResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& /*rvDesc*/, std::uint32_t slot)
        {
            auto bufferGL = LLGL_CAST(GLBuffer*, resource);
            binding.slot    = slot;
            binding.object  = bufferGL->GetID();
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment1> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment1, this, std::placeholders::_1, std::placeholders::_2),
        numSegments
    );
}

void GLResourceHeap::BuildBufferRangeSegments(ResourceBindingIterator& resourceIterator, long bindFlags, std::uint8_t& numSegments)
{
    /* Collect all buffers */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::Buffer,
        bindFlags,
        [](GLResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot)
        {
            auto bufferGL = LLGL_CAST(GLBuffer*, resource);
            binding.slot    = slot;
            binding.object  = bufferGL->GetID();

            if (IsGLBufferViewEnabled(rvDesc.bufferView))
            {
                /* Fill specified range for binding */
                binding.offset  = static_cast<GLintptr>(rvDesc.bufferView.offset);
                binding.size    = static_cast<GLsizeiptr>(rvDesc.bufferView.size);
            }
            else
            {
                /* Get buffer size and fill entire range for binding */
                GLint bufferSize = 0;
                bufferGL->GetBufferParams(&bufferSize, nullptr, nullptr);

                binding.offset  = 0;
                binding.size    = bufferSize;
            }
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment3> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment3, this, std::placeholders::_1, std::placeholders::_2),
        numSegments
    );
}

void GLResourceHeap::BuildUniformBufferSegments(ResourceBindingIterator& resourceIterator)
{
    const long bindFlags = BindFlags::ConstantBuffer;
    if (RequiresRangeForGLResourceBindings(resourceIterator, ResourceType::Buffer, bindFlags))
        BuildBufferRangeSegments(resourceIterator, bindFlags, segmentation_.numUniformBufferRangeSegments);
    else
        BuildBufferSegments(resourceIterator, bindFlags, segmentation_.numUniformBufferSegments);
}

void GLResourceHeap::BuildStorageBufferSegments(ResourceBindingIterator& resourceIterator)
{
    const long bindFlags = (BindFlags::Sampled | BindFlags::Storage);
    if (RequiresRangeForGLResourceBindings(resourceIterator, ResourceType::Buffer, bindFlags))
        BuildBufferRangeSegments(resourceIterator, bindFlags, segmentation_.numStorageBufferRangeSegments);
    else
        BuildBufferSegments(resourceIterator, bindFlags, segmentation_.numStorageBufferSegments);
}

void GLResourceHeap::BuildTextureSegments(ResourceBindingIterator& resourceIterator)
{
    /* Collect all textures with sampled binding */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::Texture,
        BindFlags::Sampled,
        [this](GLResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot)
        {
            binding.slot = slot;
            if (IsTextureViewEnabled(rvDesc.textureView))
            {
                /* Generate resource binding for custom texture-view subresource */
                binding.object = GetTextureViewID(this->numTextureViews_++);
                binding.target = GLStateManager::GetTextureTarget(rvDesc.textureView.type);
            }
            else
            {
                /* Generate resource binding for texture resource */
                auto textureGL = LLGL_CAST(GLTexture*, resource);
                binding.object = textureGL->GetID();
                binding.target = GLStateManager::GetTextureTarget(textureGL->GetType());
            }
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment2> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment2Target, this, std::placeholders::_1, std::placeholders::_2),
        segmentation_.numTextureSegments
    );
}

void GLResourceHeap::BuildImageTextureSegments(ResourceBindingIterator& resourceIterator)
{
    /* Collect all textures with storage binding */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::Texture,
        BindFlags::Storage,
        [this](GLResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& rvDesc, std::uint32_t slot)
        {
            binding.slot = slot;
            if (IsTextureViewEnabled(rvDesc.textureView))
            {
                /* Generate resource binding for custom texture-view subresource */
                binding.object = GetTextureViewID(this->numTextureViews_++);
                binding.format = GLTypes::Map(rvDesc.textureView.format);
            }
            else
            {
                /* Generate resource binding for texture resource */
                auto textureGL = LLGL_CAST(GLTexture*, resource);
                binding.object = textureGL->GetID();
                binding.format = textureGL->GetGLInternalFormat();
            }
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment2> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment2Format, this, std::placeholders::_1, std::placeholders::_2),
        segmentation_.numImageTextureSegments
    );
}

void GLResourceHeap::BuildSamplerSegments(ResourceBindingIterator& resourceIterator)
{
    /* Collect all samplers */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::Sampler,
        0,
        [](GLResourceBinding& binding, Resource* resource, const ResourceViewDescriptor& /*rvDesc*/, std::uint32_t slot)
        {
            auto samplerGL = LLGL_CAST(GLSampler*, resource);
            binding.slot    = slot;
            binding.object  = samplerGL->GetID();
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment1> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment1, this, std::placeholders::_1, std::placeholders::_2),
        segmentation_.numSamplerSegments
    );
}

void GLResourceHeap::BuildAllSegments(
    const std::vector<GLResourceBinding>&   resourceBindings,
    const BuildSegmentFunc&                 buildSegmentFunc,
    std::uint8_t&                           numSegments)
{
    if (!resourceBindings.empty())
    {
        /* Initialize iterators for sub-ranges of input bindings */
        auto    itStart = resourceBindings.begin();
        auto    itPrev  = itStart;
        auto    it      = itStart;
        GLsizei count   = 0;

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

void GLResourceHeap::BuildSegment1(GLResourceBindingIter it, GLsizei count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const std::size_t segmentSize = sizeof(GLResourceViewHeapSegment1) + sizeof(GLuint) * count;
    buffer_.resize(startOffset + segmentSize);

    /* Write segment header */
    auto segment = reinterpret_cast<GLResourceViewHeapSegment1*>(&buffer_[startOffset]);
    {
        segment->segmentSize    = segmentSize;
        segment->first          = it->slot;
        segment->count          = count;
    }

    /* Write segment body */
    auto segmentIDs = reinterpret_cast<GLuint*>(&buffer_[startOffset + sizeof(GLResourceViewHeapSegment1)]);
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentIDs[i] = it->object;
}

void GLResourceHeap::BuildSegment2Target(GLResourceBindingIter it, GLsizei count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const std::size_t segmentOffsetEnd0 = sizeof(GLResourceViewHeapSegment2) + sizeof(GLTextureTarget) * count;
    const std::size_t segmentSize       = segmentOffsetEnd0 + sizeof(GLuint) * count;
    buffer_.resize(startOffset + segmentSize);

    /* Write segment header */
    auto segment = reinterpret_cast<GLResourceViewHeapSegment2*>(&buffer_[startOffset]);
    {
        segment->segmentSize    = segmentSize;
        segment->offsetEnd0     = segmentOffsetEnd0;
        segment->first          = it->slot;
        segment->count          = count;
    }

    /* Write first part of segment body (of type <GLTextureTarget>) */
    auto segmentTargets = reinterpret_cast<GLTextureTarget*>(&buffer_[startOffset + sizeof(GLResourceViewHeapSegment2)]);
    auto begin = it;
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentTargets[i] = it->target;

    /* Write second part of segment body (of type <GLuint>) */
    auto segmentIDs = reinterpret_cast<GLuint*>(&buffer_[startOffset + segmentOffsetEnd0]);
    it = begin;
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentIDs[i] = it->object;
}

void GLResourceHeap::BuildSegment2Format(GLResourceBindingIter it, GLsizei count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const std::size_t segmentOffsetEnd0 = sizeof(GLResourceViewHeapSegment2) + sizeof(GLenum) * count;
    const std::size_t segmentSize       = segmentOffsetEnd0 + sizeof(GLuint) * count;
    buffer_.resize(startOffset + segmentSize);

    /* Write segment header */
    auto segment = reinterpret_cast<GLResourceViewHeapSegment2*>(&buffer_[startOffset]);
    {
        segment->segmentSize    = segmentSize;
        segment->offsetEnd0     = segmentOffsetEnd0;
        segment->first          = it->slot;
        segment->count          = count;
    }

    /* Write first part of segment body (of type <GLenum>) */
    auto segmentTargets = reinterpret_cast<GLenum*>(&buffer_[startOffset + sizeof(GLResourceViewHeapSegment2)]);
    auto begin = it;
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentTargets[i] = it->format;

    /* Write second part of segment body (of type <GLuint>) */
    auto segmentIDs = reinterpret_cast<GLuint*>(&buffer_[startOffset + segmentOffsetEnd0]);
    it = begin;
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentIDs[i] = it->object;
}

void GLResourceHeap::BuildSegment3(GLResourceBindingIter it, GLsizei count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const std::size_t segmentOffsetEnd0 = sizeof(GLResourceViewHeapSegment3) + sizeof(GLuint) * count;
    const std::size_t segmentOffsetEnd1 = segmentOffsetEnd0 + sizeof(GLintptr) * count;
    const std::size_t segmentSize       = segmentOffsetEnd1 + sizeof(GLsizeiptr) * count;
    buffer_.resize(startOffset + segmentSize);

    /* Write segment header */
    auto segment = reinterpret_cast<GLResourceViewHeapSegment3*>(&buffer_[startOffset]);
    {
        segment->segmentSize    = segmentSize;
        segment->offsetEnd0     = segmentOffsetEnd0;
        segment->offsetEnd1     = segmentOffsetEnd1;
        segment->first          = it->slot;
        segment->count          = count;
    }

    /* Write first part of segment body (of type <GLuint>) */
    auto segmentIDs = reinterpret_cast<GLuint*>(&buffer_[startOffset + sizeof(GLResourceViewHeapSegment3)]);
    auto begin = it;
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentIDs[i] = it->object;

    /* Write second part of segment body (of type <GLintptr>) */
    auto segmentOffsets = reinterpret_cast<GLintptr*>(&buffer_[startOffset + segmentOffsetEnd0]);
    it = begin;
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentOffsets[i] = it->offset;

    /* Write second part of segment body (of type <GLsizeiptr>) */
    auto segmentSizes = reinterpret_cast<GLsizeiptr*>(&buffer_[startOffset + segmentOffsetEnd1]);
    it = begin;
    for (GLsizei i = 0; i < count; ++i, ++it)
        segmentSizes[i] = it->size;
}

void GLResourceHeap::WriteSegmentationHeapEnd(const void* data, std::size_t size)
{
    std::size_t startOffset = buffer_.size();
    buffer_.resize(startOffset + size);
    ::memcpy(&buffer_[startOffset], data, size);
}

GLuint GLResourceHeap::GetTextureViewID(std::size_t idx) const
{
    /* All texture-view IDs are stored in the front of the internal buffer */
    return *(reinterpret_cast<const GLuint*>(buffer_.data()) + idx);
}

std::size_t GLResourceHeap::GetSegmentationHeapSize() const
{
    /* Resource heap starts after texture-view IDs (GLuint) */
    return (buffer_.size() - (numTextureViews_ * sizeof(GLuint)));
}

const std::int8_t* GLResourceHeap::GetSegmentationHeapStart(std::uint32_t firstSet) const
{
    /* Resource heap starts after texture-view IDs (GLuint) */
    return (buffer_.data() + (numTextureViews_ * sizeof(GLuint)) + stride_ * firstSet);
}


} // /namespace LLGL



// ================================================================================
