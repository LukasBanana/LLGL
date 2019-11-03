/*
 * MTResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_RESOURCE_HEAP_H
#define LLGL_MT_RESOURCE_HEAP_H


#include <Metal/Metal.h>

#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceFlags.h>
#include <vector>
#include <functional>


namespace LLGL
{


class ResourceBindingIterator;
struct MTResourceBinding;

/*
This class emulates the behavior of a descriptor set like in Vulkan,
by binding all shader resources within one bind call in the command buffer.
*/
class MTResourceHeap : public ResourceHeap
{

    public:

        MTResourceHeap(const ResourceHeapDescriptor& desc);

        void BindGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder);
        void BindComputeResources(id<MTLComputeCommandEncoder> computeEncoder);

        bool HasGraphicsResources() const;
        bool HasComputeResources() const;

    private:

        using MTResourceBindingIter = std::vector<MTResourceBinding>::const_iterator;
        using BuildSegmentFunc = std::function<void(MTResourceBindingIter begin, NSUInteger count)>;
    
        void BuildBufferSegments(ResourceBindingIterator& resourceIterator, long stage, std::uint8_t& numSegments);
        void BuildTextureSegments(ResourceBindingIterator& resourceIterator, long stage, std::uint8_t& numSegments);
        void BuildSamplerSegments(ResourceBindingIterator& resourceIterator, long stage, std::uint8_t& numSegments);

        void BuildAllSegments(
            const std::vector<MTResourceBinding>&   resourceBindings,
            const BuildSegmentFunc&                 buildSegmentFunc,
            std::uint8_t&                           numSegments
        );

        void BuildSegment1(MTResourceBindingIter it, NSUInteger count);
        void BuildSegment2(MTResourceBindingIter it, NSUInteger count);

        void BindVertexResources(id<MTLRenderCommandEncoder> cmdEncoder, std::int8_t*& byteAlignedBuffer);
        void BindFragmentResources(id<MTLRenderCommandEncoder> cmdEncoder, std::int8_t*& byteAlignedBuffer);
        void BindKernelResources(id<MTLComputeCommandEncoder> cmdEncoder, std::int8_t*& byteAlignedBuffer);

        // Header structure to describe all segments within the raw buffer.
        struct SegmentationHeader
        {
            std::uint8_t hasVertexResources         : 1;
            std::uint8_t hasFragmentResources       : 1;
            std::uint8_t hasKernelResources         : 1;

            std::uint8_t numVertexBufferSegments;
            std::uint8_t numVertexTextureSegments;
            std::uint8_t numVertexSamplerSegments;
            
            std::uint8_t numFragmentBufferSegments;
            std::uint8_t numFragmentTextureSegments;
            std::uint8_t numFragmentSamplerSegments;
            
            std::uint8_t numKernelBufferSegments;
            std::uint8_t numKernelTextureSegments;
            std::uint8_t numKernelSamplerSegments;
        };

        SegmentationHeader          segmentationHeader_;
        std::uint16_t               bufferOffsetKernel_ = 0;
        std::vector<std::int8_t>    buffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
