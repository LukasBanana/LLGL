/*
 * ResourceViewHeapFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_VIEW_HEAP_FLAGS_H
#define LLGL_RESOURCE_VIEW_HEAP_FLAGS_H


#include "Export.h"
#include <vector>


namespace LLGL
{


class Resource;
class PipelineLayout;


/* ----- Structures ----- */

//! Resource view descriptor structure.
struct ResourceViewDescriptor
{
    //! Default constructor to initialize the resource view with a null pointer.
    ResourceViewDescriptor() = default;

    //! Constructor to initialize the descriptor with a Buffer resource view.
    inline ResourceViewDescriptor(Resource* resource) :
        resource { resource }
    {
    }

    //! Pointer to the hardware resoudce.
    Resource* resource = nullptr;
};

/**
\brief Resource heap descriptor structure.
\remarks For the render systems of modern graphics APIs (i.e. Vulkan and Direct3D 12), a resource heap is the only way to bind hardware resources to a shader pipeline.
The resource heap is a container for one or more resources such as textures, samplers, constant buffers etc.
*/
struct ResourceHeapDescriptor
{
    //! Reference to the pipeline layout.
    PipelineLayout*                     pipelineLayout = nullptr;

    //! List of all resource view descriptors.
    std::vector<ResourceViewDescriptor> resourceViews;
};


} // /namespace LLGL


#endif



// ================================================================================
