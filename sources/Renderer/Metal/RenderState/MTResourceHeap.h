/*
 * MTResourceHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_RESOURCE_HEAP_H
#define LLGL_MT_RESOURCE_HEAP_H


#import <Metal/Metal.h>

#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include "../Shader/MTShaderStage.h"
#include "../../BindingIterator.h"
#include "../../SegmentedBuffer.h"
#include <vector>
#include <functional>


namespace LLGL
{


enum MTResourceType : std::uint32_t;
class MTTexture;
struct MTResourceBinding;
struct ResourceHeapDescriptor;
struct TextureViewDescriptor;

/*
This class emulates the behavior of a descriptor set like in Vulkan,
by binding all shader resources within one bind call in the command buffer.
*/
class MTResourceHeap final : public ResourceHeap
{

    public:

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        MTResourceHeap(
            const ResourceHeapDescriptor&               desc,
            const ArrayView<ResourceViewDescriptor>&    initialResourceViews = {}
        );
        ~MTResourceHeap();

        // Writes the specified resource views to this resource heap and generates texture views as required.
        std::uint32_t WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews);

        void BindGraphicsResources(id<MTLRenderCommandEncoder> renderEncoder, std::uint32_t descriptorSet);
        void BindComputeResources(id<MTLComputeCommandEncoder> computeEncoder, std::uint32_t descriptorSet);

        bool HasGraphicsResources() const;
        bool HasComputeResources() const;

    private:

        struct MTResourceBinding;

        using SegmentationSizeType  = std::uint8_t;
        using AllocSegmentFunc      = std::function<void(const MTResourceBinding* first, NSUInteger count)>;

        // Header structure to describe all segments within the raw buffer.
        struct BufferSegmentation
        {
            SegmentationSizeType hasVertexResources         : 1;
            SegmentationSizeType hasFragmentResources       : 1;
            SegmentationSizeType hasKernelResources         : 1;

            SegmentationSizeType numVertexBufferSegments;
            SegmentationSizeType numVertexTextureSegments;
            SegmentationSizeType numVertexSamplerSegments;

            SegmentationSizeType numFragmentBufferSegments;
            SegmentationSizeType numFragmentTextureSegments;
            SegmentationSizeType numFragmentSamplerSegments;

            SegmentationSizeType numKernelBufferSegments;
            SegmentationSizeType numKernelTextureSegments;
            SegmentationSizeType numKernelSamplerSegments;
        };

        // Binding-to-descriptor map location.
        struct BindingSegmentLocation
        {
            static constexpr std::uint32_t invalidOffset = 0x0000FFFF;

            struct Stage
            {
                inline Stage() :
                    segmentOffset    { BindingSegmentLocation::invalidOffset },
                    descriptorIndex  { 0                                     },
                    textureViewIndex { 0                                     }
                {
                }

                std::uint32_t segmentOffset     : 16; // Byte offset to the first segment within a segment set.
                std::uint32_t descriptorIndex   :  8; // Index of the descriptor the binding maps to.
                std::uint32_t textureViewIndex  :  8; // Index of the texture view if the segment type equals MTResourceType_Texture.
            }
            stages[MTShaderStage_Count];
        };

        // Metal resource binding slot with index to the input binding list
        struct MTResourceBinding
        {
            NSUInteger  slot;
            long        stages; // bitwise OR combination of StageFlags entries
            std::size_t index;  // Index to the input bindings list
        };

    private:

        SegmentationSizeType AllocBufferSegments(BindingDescriptorIterator& bindingIter, long stage);
        SegmentationSizeType AllocTextureSegments(BindingDescriptorIterator& bindingIter, long stage);
        SegmentationSizeType AllocSamplerStateSegments(BindingDescriptorIterator& bindingIter, long stage);

        void Alloc1PartSegment(
            MTShaderStage               stage,
            MTResourceType              type,
            const MTResourceBinding*    first,
            NSUInteger                  count,
            std::size_t                 payload0Stride
        );

        void Alloc2PartSegment(
            MTShaderStage               stage,
            MTResourceType              type,
            const MTResourceBinding*    first,
            NSUInteger                  count,
            std::size_t                 payload0Stride,
            std::size_t                 payload1Stride
        );

        void WriteBindingMappings(MTShaderStage stage, MTResourceType type, const MTResourceBinding* first, NSUInteger count);
        void CacheResourceUsage();

        const char* BindVertexResources(id<MTLRenderCommandEncoder> cmdEncoder, const char* heapPtr);
        const char* BindFragmentResources(id<MTLRenderCommandEncoder> cmdEncoder, const char* heapPtr);
        const char* BindKernelResources(id<MTLComputeCommandEncoder> cmdEncoder, const char* heapPtr);

        void WriteResourceViewBuffer(const ResourceViewDescriptor& desc, char* heapPtr, const BindingSegmentLocation::Stage& binding);
        void WriteResourceViewTexture(const ResourceViewDescriptor& desc, char* heapPtr, const BindingSegmentLocation::Stage& binding, std::uint32_t descriptorSet);
        void WriteResourceViewSamplerState(const ResourceViewDescriptor& desc, char* heapPtr, const BindingSegmentLocation::Stage& binding);

        void ExchangeTextureView(
            std::uint32_t                           descriptorSet,
            const BindingSegmentLocation::Stage&    binding,
            id<MTLTexture>                          textureView
        );

        id<MTLTexture> GetOrCreateTexture(
            std::uint32_t                           descriptorSet,
            const BindingSegmentLocation::Stage&    binding,
            MTTexture&                              textureMT,
            const TextureViewDescriptor&            textureViewDesc
        );

    private:

        static std::vector<MTResourceBinding> FilterAndSortMTBindingSlots(
            BindingDescriptorIterator&  bindingIter,
            ResourceType                resourceType,
            long                        affectedStage
        );

        static SegmentationSizeType ConsolidateSegments(
            const ArrayView<MTResourceBinding>& bindingSlots,
            const AllocSegmentFunc&             allocSegmentFunc
        );

    private:

        SmallVector<BindingSegmentLocation> bindingMap_;                    // Maps a binding index to a descriptor location.
        BufferSegmentation                  segmentation_           = {};

        SegmentedBuffer                     heap_;                          // Buffer with resource binding information and stride (in bytes) per descriptor set
        std::uint32_t                       heapOffsetKernel_       = 0;    // Heap offset for kernel resources.

        std::vector<id<MTLTexture>>         textureViews_;
        std::uint32_t                       numTextureViewsPerSet_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
