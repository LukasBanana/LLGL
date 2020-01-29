/*
 * GLResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RESOURCE_HEAP_H
#define LLGL_GL_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceFlags.h>
#include "../OpenGL.h"
#include <vector>
#include <functional>


namespace LLGL
{


class GLStateManager;
class ResourceBindingIterator;
struct ResourceHeapDescriptor;
struct GLResourceBinding;

/*
This class emulates the behavior of a descriptor set like in Vulkan,
by binding all shader resources within one bind call in the command buffer.
*/
class GLResourceHeap final : public ResourceHeap
{

    public:

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        GLResourceHeap(const ResourceHeapDescriptor& desc);
        ~GLResourceHeap();

        // Binds this resource heap with the specified GL state manager.
        void Bind(GLStateManager& stateMngr, std::uint32_t firstSet);

    private:

        using GLResourceBindingIter = std::vector<GLResourceBinding>::const_iterator;
        using BuildSegmentFunc = std::function<void(GLResourceBindingIter begin, GLsizei count)>;

        void BuildTextureViews(ResourceBindingIterator& resourceIterator, long bindFlags);

        void BuildBufferSegments(ResourceBindingIterator& resourceIterator, long bindFlags, std::uint8_t& numSegments);
        void BuildBufferRangeSegments(ResourceBindingIterator& resourceIterator, long bindFlags, std::uint8_t& numSegments);
        void BuildUniformBufferSegments(ResourceBindingIterator& resourceIterator);
        void BuildStorageBufferSegments(ResourceBindingIterator& resourceIterator);
        void BuildTextureSegments(ResourceBindingIterator& resourceIterator);
        void BuildImageTextureSegments(ResourceBindingIterator& resourceIterator);
        void BuildSamplerSegments(ResourceBindingIterator& resourceIterator);

        void BuildAllSegments(
            const std::vector<GLResourceBinding>&   resourceBindings,
            const BuildSegmentFunc&                 buildSegmentFunc,
            std::uint8_t&                           numSegments
        );

        void BuildSegment1(GLResourceBindingIter it, GLsizei count);
        void BuildSegment2Target(GLResourceBindingIter it, GLsizei count);
        void BuildSegment2Format(GLResourceBindingIter it, GLsizei count);
        void BuildSegment3(GLResourceBindingIter it, GLsizei count);

        void WriteSegmentationHeapEnd(const void* data, std::size_t size);

        GLuint GetTextureViewID(std::size_t idx) const;

        std::size_t GetSegmentationHeapSize() const;

        const std::int8_t* GetSegmentationHeapStart(std::uint32_t firstSet) const;

    private:

        // Describes the segments within the raw buffer (per descriptor set).
        struct BufferSegmentation
        {
            std::uint8_t numUniformBufferSegments       = 0;
            std::uint8_t numUniformBufferRangeSegments  = 0;
            std::uint8_t numStorageBufferSegments       = 0;
            std::uint8_t numStorageBufferRangeSegments  = 0;
            std::uint8_t numTextureSegments             = 0;
            std::uint8_t numImageTextureSegments        = 0;
            std::uint8_t numSamplerSegments             = 0;
        };

    private:

        BufferSegmentation          segmentation_;

        std::size_t                 numTextureViews_    = 0;    // Number of GL texture objects generated with glTextureView
        std::size_t                 stride_             = 0;    // Buffer stride (in bytes) per descriptor set
        std::vector<std::int8_t>    buffer_;                    // Raw buffer with resource binding information

        GLbitfield                  barriers_           = 0;    // Bitmask for glMemoryBarrier

};


} // /namespace LLGL


#endif



// ================================================================================
