/*
 * GLResourceHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_RESOURCE_HEAP_H
#define LLGL_GL_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>
#include "../../BindingIterator.h"
#include "../../SegmentedBuffer.h"
#include "../OpenGL.h"
#include <functional>


namespace LLGL
{


enum GLResourceType : std::uint32_t;
class GLStateManager;
class GLShaderBufferInterfaceMap;
struct ResourceHeapDescriptor;
struct GLHeapResourceBinding;

/*
This class emulates the behavior of a descriptor set like in Vulkan,
by binding all shader resources within one bind call in the command buffer.
*/
class GLResourceHeap final : public ResourceHeap
{

    public:

        std::uint32_t GetNumDescriptorSets() const override;

    public:

        GLResourceHeap(
            const ResourceHeapDescriptor&               desc,
            const ArrayView<ResourceViewDescriptor>&    initialResourceViews = {}
        );
        ~GLResourceHeap();

        // Writes the specified resource views to this resource heap and generates texture views as required.
        std::uint32_t WriteResourceViews(std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews);

        // Binds this resource heap with the specified GL state manager.
        void Bind(GLStateManager& stateMngr, std::uint32_t descriptorSet, const GLShaderBufferInterfaceMap* bufferInterfaceMap = nullptr);

    private:

        struct GLResourceBinding;

        using SegmentationSizeType  = std::uint8_t;
        using AllocSegmentFunc      = std::function<void(const GLResourceBinding* first, SegmentationSizeType count)>;
        using GLHeapBindingIterator = BindingIterator<GLHeapResourceBinding>;

        // Describes the segments within the raw buffer (per descriptor set).
        struct BufferSegmentation
        {
            SegmentationSizeType numUniformBufferSegments   = 0;
            SegmentationSizeType numStorageBufferSegments   = 0;
            SegmentationSizeType numTextureSegments         = 0;
            SegmentationSizeType numImageTextureSegments    = 0;
            SegmentationSizeType numSamplerSegments         = 0;
        };

        // Binding-to-descriptor map location.
        struct BindingSegmentLocation
        {
            std::uint32_t isCombinedSampler      :  1;
            std::uint32_t segmentOrBindingOffset : 23; // Byte offset to the first segment within a segment set.
            std::uint32_t indexOrCount           :  8; // Index of the descriptor the binding maps to.
        };

        // GL resource binding slot with index to the input binding list.
        struct GLResourceBinding
        {
            GLuint      slot;       // GL pipeline binding slot
            std::size_t mapIndex;   // Index to the input binding map
        };

    private:

        void AllocTextureView(GLuint& texViewID, GLuint sourceTexID, const TextureViewDescriptor& textureViewDesc);
        bool FreeTextureView(GLuint& texViewID);
        void FreeAllSegmentSetTextureViews(const char* heapPtr);
        void FreeAllSegmentsTextureViews();

        void AllocSegmentsUBO(GLHeapBindingIterator& bindingIter);
        void AllocSegmentsBuffer(GLHeapBindingIterator& bindingIter);
        void AllocSegmentsTexture(GLHeapBindingIterator& bindingIter, const ArrayView<GLuint>& combinedSamplerSlots);
        void AllocSegmentsImage(GLHeapBindingIterator& bindingIter);
        void AllocSegmentsSampler(GLHeapBindingIterator& bindingIter, const ArrayView<GLuint>& combinedSamplerSlots);
        void AllocSegmentsNativeSampler(GLHeapBindingIterator& bindingIter, const ArrayView<GLuint>& combinedSamplerSlots);
        void AllocSegmentsEmulatedSampler(GLHeapBindingIterator& bindingIter, const ArrayView<GLuint>& combinedSamplerSlots);

        void Alloc1PartSegment(
            GLResourceType              type,
            const GLResourceBinding*    first,
            SegmentationSizeType        count,
            std::size_t                 payload0Stride
        );

        void Alloc2PartSegment(
            GLResourceType              type,
            const GLResourceBinding*    first,
            SegmentationSizeType        count,
            std::size_t                 payload0Stride,
            std::size_t                 payload1Stride
        );

        void Alloc3PartSegment(
            GLResourceType              type,
            const GLResourceBinding*    first,
            SegmentationSizeType        count,
            std::size_t                 payload0Stride,
            std::size_t                 payload1Stride,
            std::size_t                 payload2Stride
        );

        void Alloc4PartSegment(
            GLResourceType              type,
            const GLResourceBinding*    first,
            SegmentationSizeType        count,
            std::size_t                 payload0Stride,
            std::size_t                 payload1Stride,
            std::size_t                 payload2Stride,
            std::size_t                 payload3Stride
        );

        void WriteBindingMappings(const GLResourceBinding* bindings, SegmentationSizeType count);
        void CopyBindingMapping(const GLResourceBinding& dst, const GLResourceBinding& src);

        void WriteResourceView(const ResourceViewDescriptor& desc, const BindingSegmentLocation& binding, std::uint32_t descriptorSet);

        void WriteResourceViewUBO(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewBuffer(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewTexture(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewImage(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewSampler(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewEmulatedSampler(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);

        std::vector<GLResourceBinding> FilterAndSortGLBindingSlots(
            GLHeapBindingIterator&      bindingIter,
            ResourceType                resourceType,
            long                        resourceBindFlags,
            const ArrayView<GLuint>&    combinedSamplerSlots = {}
        );

    private:

        static SegmentationSizeType ConsolidateSegments(
            const ArrayView<GLResourceBinding>& bindingSlots,
            const AllocSegmentFunc&             allocSegmentFunc
        );

    private:

        SmallVector<BindingSegmentLocation> bindingMap_;            // Maps binding indices to descriptor locations; bindingMap_[numInputBindings_] starts implicit descriptors.
        std::uint32_t                       numInputBindings_ = 0;  // Number of bindings written explicitly to a heap segment.
        BufferSegmentation                  segmentation_;
        SegmentedBuffer                     heap_;                  // Buffer with resource binding information and stride (in bytes) per descriptor set

};


} // /namespace LLGL


#endif



// ================================================================================
