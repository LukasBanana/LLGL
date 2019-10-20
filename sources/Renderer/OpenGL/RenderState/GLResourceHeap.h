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
struct GLResourceBinding;

/*
This class emulates the behavior of a descriptor set like in Vulkan,
by binding all shader resources within one bind call in the command buffer.
*/
class GLResourceHeap final : public ResourceHeap
{

    public:

        GLResourceHeap(const ResourceHeapDescriptor& desc);

        // Binds this resource heap with the specified GL state manager.
        void Bind(GLStateManager& stateMngr);

    private:

        using GLResourceBindingIter = std::vector<GLResourceBinding>::const_iterator;
        using BuildSegmentFunc = std::function<void(GLResourceBindingIter begin, GLsizei count)>;

        void BuildBufferSegments(ResourceBindingIterator& resourceIterator, long bindFlags, std::uint8_t& numSegments);
        void BuildConstantBufferSegments(ResourceBindingIterator& resourceIterator);
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

    private:

        // Header structure to describe all segments within the raw buffer.
        struct SegmentationHeader
        {
            std::uint8_t numConstantBufferSegments  = 0;
            std::uint8_t numStorageBufferSegments   = 0;
            std::uint8_t numTextureSegments         = 0;
            std::uint8_t numImageTextureSegments    = 0;
            std::uint8_t numSamplerSegments         = 0;
        };

    private:

        SegmentationHeader          segmentationHeader_;
        std::vector<std::int8_t>    buffer_;
        GLbitfield                  barriers_           = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
