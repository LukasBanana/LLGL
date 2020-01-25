/*
 * ResourceHeap.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_HEAP_H
#define LLGL_RESOURCE_HEAP_H


#include "RenderSystemChild.h"
#include <cstdint>


namespace LLGL
{


/**
\brief Resource heap interface.
\remarks An instance of this interface provides a descriptor set (as called in Vulkan)
or descriptor heap (as called in Direct3D 12) for graphics and compute pipelines.
\see RenderSystem::CreateResourceHeap
\see CommandBuffer::SetResourceHeap
*/
class LLGL_EXPORT ResourceHeap : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::ResourceHeap );

    public:

        /**
        \brief Returns the number of descriptor sets in this heap.
        \remarks This is determined by the number of resources in the heap divided by the number of bindings in the pipeline layout.
        The total number of resources in the heap can therefore be determined by the following code:
        \code
        myResourceHeap->GetNumDescriptorSets() * myPipelineLayout->GetNumBindings();
        \endcode
        \see PipelineLayout::GetNumBindings
        \see CommandBuffer::SetResourceHeap
        */
        virtual std::uint32_t GetNumDescriptorSets() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
