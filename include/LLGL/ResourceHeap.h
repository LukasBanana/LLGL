/*
 * ResourceHeap.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RESOURCE_HEAP_H
#define LLGL_RESOURCE_HEAP_H


#include <LLGL/RenderSystemChild.h>
#include <cstdint>


namespace LLGL
{


/**
\brief Resource heap interface.
\remarks An instance of this interface provides all descriptor sets (as called in Vulkan)
or descriptor heaps (as called in Direct3D 12) for graphics and compute pipelines.
For other backends that do not support resource heaps natively, the functionality is emulated by LLGL.
\see RenderSystem::CreateResourceHeap
\see CommandBuffer::SetResourceHeap
*/
class LLGL_EXPORT ResourceHeap : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::ResourceHeap );

    public:

        /**
        \brief Returns whether this is a bindless resource heap.
        \remarks A bindless resource heap is created with a PipelineLayout whoes \c heapBindings list only contains a single element of undefined resource type.
        \code
        // Default value of LLGL::BindingDescriptor has a type equal to LLGL::ResourceType::Undefined
        // This tells LLGL to create a PSO layout with a bindless resource heap.
        LLGL::PipelineLayoutDescriptor psoLayoutDesc;
        psoLayoutDesc.heapBindings = { LLGL::BindingDescriptor{} };
        LLGL::PipelineLayout* psoLayout = renderer->CreatePipelineLayout(psoLayoutDesc);

        // Create a bindless resource heap
        LLGL::ResourceHeapDescriptor resHeapDesc;
        resHeapDesc.pipelineLayout = psoLayout;
        resHeapDesc.numResourceViews = 100;
        LLGL::ResourceHeap resHeap = renderer->CreateResourceHeap(resHeapDesc);
        \endcode
        */
        virtual bool IsBindless() const = 0;

        /**
        \brief Returns the number of descriptor sets in this heap.
        \remarks This is determined by the number of resources in the heap divided by the number of heap bindings in the pipeline layout.
        The total number of resources in the heap can therefore be determined by the following code:
        \code
        myResourceHeap->GetNumDescriptorSets() * myPipelineLayout->GetNumHeapBindings();
        \endcode
        \see PipelineLayout::GetNumHeapBindings
        \see CommandBuffer::SetResourceHeap
        */
        virtual std::uint32_t GetNumDescriptorSets() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
