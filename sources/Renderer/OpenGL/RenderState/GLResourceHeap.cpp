/*
 * GLResourceHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLResourceHeap.h"
#include "GLPipelineLayout.h"
#include "GLStateManager.h"
#include "../Buffer/GLBuffer.h"
#include "../Texture/GLSampler.h"
#include "../Texture/GLTexture.h"
#include "../../CheckedCast.h"
#include "../../ResourceBindingIterator.h"


namespace LLGL
{


/*
 * Internal structures
 */

/*

The internal buffer of GLResourceHeap is tightly packed which stores all segments of binding points consecutively.
Here is an illustration of the buffer layout for one Texture resouce (at binding point 4) and two Sampler resources (at binding points 5 and 6) on a 32-bit build:

Offset      Attribute                                   Value   Description                                         Segment
--------------------------------------------------------------------------------------------------------------------------------------------
0x00000000  GLResourceViewHeapSegment2::segmentSize     24      Size of this segment                                \
0x00000004  GLResourceViewHeapSegment2::offsetEnd0      20      Relative offset to texture[0] (at 0x00000014)        |
0x00000008  GLResourceViewHeapSegment2::first           4       First binding point                                  |-- Texture segment
0x0000000C  GLResourceViewHeapSegment2::count           1       Number of binding points                             |
0x00000010  target[0]                                   1       Texture target (GLTextureTarget::TEXTURE_2D = 1)     |
0x00000014  texture[0]                                  1       1st OpenGL texture ID (from 'glGenTextures')        /
0x00000018  GLResourceViewHeapSegment1::segmentSize     20      Size of this segment                                \
0x0000001C  GLResourceViewHeapSegment1::first           5       First binding point                                  |
0x00000020  GLResourceViewHeapSegment1::count           2       Number of binding points                             |-- Sampler segment
0x00000024  sampler[0]                                  1       1st OpenGL sampler ID (from 'glGenTextures')         |
0x00000028  sampler[1]                                  2       2nd OpenGL sampler ID (from 'glGenSamplers')        /

*/

// Resource view heap (RVH) segment structure with one dynamic sub-buffer for <GLuint>
struct GLResourceViewHeapSegment1
{
    std::size_t segmentSize;    // TODO: maybe use std::uint16_t for optimization
    GLuint      first;          // TODO: maybe use std::uint8_t for optimization
    GLsizei     count;          // TODO: maybe use std::uint8_t for optimization
};

// Resource view heap (RVH) segment structure with two dynamic sub-buffers, one for <GLTextureTarget> and one for <GLuint>
struct GLResourceViewHeapSegment2
{
    std::size_t segmentSize;
    std::size_t offsetEnd0; // Byte offset after the first sub-buffer, following the second sub-buffer
    GLuint      first;
    GLsizei     count;
};

struct GLResourceBinding
{
    GLuint          slot;
    GLuint          object;
    GLTextureTarget target;
};


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
    const auto& bindings = pipelineLayoutGL->GetBindings();
    if (desc.resourceViews.size() != bindings.size())
        throw std::invalid_argument("failed to create resource heap due to mismatch between number of resources and bindings");

    /* Build buffer segments */
    ResourceBindingIterator resourceIterator { desc.resourceViews, bindings };

    BuildConstantBufferSegments(resourceIterator);
    BuildStorageBufferSegments(resourceIterator);
    BuildTextureSegments(resourceIterator);
    BuildSamplerSegments(resourceIterator);
}

static void BindBuffersBaseSegment(GLStateManager& stateMngr, std::int8_t*& byteAlignedBuffer, const GLBufferTarget bufferTarget)
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

static void BindTexturesSegment(GLStateManager& stateMngr, std::int8_t*& byteAlignedBuffer)
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

static void BindSamplersSegment(GLStateManager& stateMngr, std::int8_t*& byteAlignedBuffer)
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

void GLResourceHeap::Bind(GLStateManager& stateMngr)
{
    auto byteAlignedBuffer = buffer_.data();

    /* Bind all constant buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numConstantBufferSegments; ++i)
        BindBuffersBaseSegment(stateMngr, byteAlignedBuffer, GLBufferTarget::UNIFORM_BUFFER);

    /* Bind all shader storage buffers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numStorageBufferSegments; ++i)
        BindBuffersBaseSegment(stateMngr, byteAlignedBuffer, GLBufferTarget::SHADER_STORAGE_BUFFER);

    /* Bind all textures */
    for (std::uint8_t i = 0; i < segmentationHeader_.numTextureSegments; ++i)
        BindTexturesSegment(stateMngr, byteAlignedBuffer);

    /* Bind all samplers */
    for (std::uint8_t i = 0; i < segmentationHeader_.numSamplerSegments; ++i)
        BindSamplersSegment(stateMngr, byteAlignedBuffer);
}


/*
 * ======= Private: =======
 */

using GLResourceBindingFunc = std::function<GLResourceBinding(Resource* resource, std::uint32_t slot)>;

static std::vector<GLResourceBinding> CollectGLResourceBindings(
    ResourceBindingIterator&        resourceIterator,
    const ResourceType              resourceType,
    const GLResourceBindingFunc&    resourceFunc)
{
    /* Collect all binding points of the specified resource type */
    BindingDescriptor bindingDesc;
    resourceIterator.Reset(resourceType);

    std::vector<GLResourceBinding> resourceBindings;
    resourceBindings.reserve(resourceIterator.GetCount());

    while (auto resource = resourceIterator.Next(bindingDesc))
        resourceBindings.push_back(resourceFunc(resource, bindingDesc.slot));

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

void GLResourceHeap::BuildBufferSegments(ResourceBindingIterator& resourceIterator, const ResourceType resourceType, std::uint8_t& numSegments)
{
    /* Collect all buffers */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::ConstantBuffer,
        [](Resource* resource, std::uint32_t slot) -> GLResourceBinding
        {
            auto bufferGL = LLGL_CAST(GLBuffer*, resource);
            return { slot, bufferGL->GetID(), GLTextureTarget::TEXTURE_1D };
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment1> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment1, this, std::placeholders::_1, std::placeholders::_2),
        numSegments
    );
}

void GLResourceHeap::BuildConstantBufferSegments(ResourceBindingIterator& resourceIterator)
{
    BuildBufferSegments(resourceIterator, ResourceType::ConstantBuffer, segmentationHeader_.numConstantBufferSegments);
}

void GLResourceHeap::BuildStorageBufferSegments(ResourceBindingIterator& resourceIterator)
{
    BuildBufferSegments(resourceIterator, ResourceType::StorageBuffer, segmentationHeader_.numStorageBufferSegments);
}

void GLResourceHeap::BuildTextureSegments(ResourceBindingIterator& resourceIterator)
{
    /* Collect all textures */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::Texture,
        [](Resource* resource, std::uint32_t slot) -> GLResourceBinding
        {
            auto textureGL = LLGL_CAST(GLTexture*, resource);
            return { slot, textureGL->GetID(), GLStateManager::GetTextureTarget(textureGL->GetType()) };
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment2> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment2, this, std::placeholders::_1, std::placeholders::_2),
        segmentationHeader_.numTextureSegments
    );
}

void GLResourceHeap::BuildSamplerSegments(ResourceBindingIterator& resourceIterator)
{
    /* Collect all samplers */
    auto resourceBindings = CollectGLResourceBindings(
        resourceIterator,
        ResourceType::Sampler,
        [](Resource* resource, std::uint32_t slot) -> GLResourceBinding
        {
            auto samplerGL = LLGL_CAST(GLSampler*, resource);
            return { slot, samplerGL->GetID(), GLTextureTarget::TEXTURE_1D };
        }
    );

    /* Build all resource segments for type <GLResourceViewHeapSegment1> */
    BuildAllSegments(
        resourceBindings,
        std::bind(&GLResourceHeap::BuildSegment1, this, std::placeholders::_1, std::placeholders::_2),
        segmentationHeader_.numSamplerSegments
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
    const auto segmentSize = sizeof(GLResourceViewHeapSegment1) + sizeof(GLuint) * count;
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

void GLResourceHeap::BuildSegment2(GLResourceBindingIter it, GLsizei count)
{
    std::size_t startOffset = buffer_.size();

    /* Allocate space for segment */
    const auto segmentOffsetEnd0    = sizeof(GLResourceViewHeapSegment2) + sizeof(GLTextureTarget) * count;
    const auto segmentSize          = segmentOffsetEnd0 + sizeof(GLuint) * count;
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


} // /namespace LLGL



// ================================================================================
