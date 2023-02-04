/*
 * GLResourceHeap.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RESOURCE_HEAP_H
#define LLGL_GL_RESOURCE_HEAP_H


#include <LLGL/ResourceHeap.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/ResourceFlags.h>
#include <LLGL/Container/ArrayView.h>
#include "../../SegmentedBuffer.h"
#include "../OpenGL.h"
#include <functional>


namespace LLGL
{


enum GLResourceType : std::uint32_t;
class GLStateManager;
class BindingDescriptorIterator;
struct ResourceHeapDescriptor;

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
        void Bind(GLStateManager& stateMngr, std::uint32_t descriptorSet);

    private:

        struct GLResourceBinding;

        using SegmentationSizeType  = std::uint8_t;
        using AllocSegmentFunc      = std::function<void(const GLResourceBinding* first, SegmentationSizeType count)>;

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
            std::uint32_t segmentOffset   : 24; // Byte offset to the first segment within a segment set.
            std::uint32_t descriptorIndex :  8; // Index of the descriptor the binding maps to.
        };

        // GL resource binding slot with index to the input binding list
        struct GLResourceBinding
        {
            GLuint      slot;   // GL pipeline binding slot
            std::size_t index;  // Index to the input bindings list
        };

    private:

        void AllocTextureView(GLuint& texViewID, GLuint sourceTexID, const TextureViewDescriptor& textureViewDesc);
        bool FreeTextureView(GLuint& texViewID);
        void FreeAllSegmentSetTextureViews(const char* heapPtr);
        void FreeAllSegmentsTextureViews();

        void AllocSegmentsUBO(BindingDescriptorIterator& bindingIter);
        void AllocSegmentsSSBO(BindingDescriptorIterator& bindingIter);
        void AllocSegmentsTexture(BindingDescriptorIterator& bindingIter);
        void AllocSegmentsImage(BindingDescriptorIterator& bindingIter);
        void AllocSegmentsSampler(BindingDescriptorIterator& bindingIter);
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        void AllocSegmentsGL2XSampler(BindingDescriptorIterator& bindingIter);
        #endif

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

        void WriteBindingMappings(const GLResourceBinding* first, SegmentationSizeType count);
        void CopyBindingMapping(const GLResourceBinding& dst, const GLResourceBinding& src);

        void WriteResourceViewBuffer(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index, long anyBindFlags);
        void WriteResourceViewUBO(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewSSBO(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewTexture(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewImage(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        void WriteResourceViewSampler(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        #ifdef LLGL_GL_ENABLE_OPENGL2X
        void WriteResourceViewGL2XSampler(const ResourceViewDescriptor& desc, char* heapPtr, std::uint32_t index);
        #endif

    private:

        static std::vector<GLResourceBinding> FilterAndSortGLBindingSlots(
            BindingDescriptorIterator&  bindingIter,
            ResourceType                resourceType,
            long                        resourceBindFlags
        );

        static SegmentationSizeType ConsolidateSegments(
            const ArrayView<GLResourceBinding>& bindingSlots,
            const AllocSegmentFunc&             allocSegmentFunc
        );

    private:

        SmallVector<BindingSegmentLocation> bindingMap_;            // Maps a binding index to a descriptor location.
        BufferSegmentation                  segmentation_;
        SegmentedBuffer                     heap_;                  // Buffer with resource binding information and stride (in bytes) per descriptor set
        GLbitfield                          barriers_       = 0;    // Bitmask for glMemoryBarrier

};


} // /namespace LLGL


#endif



// ================================================================================
