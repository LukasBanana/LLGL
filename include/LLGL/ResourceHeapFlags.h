/*
 * ResourceViewHeapFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_VIEW_HEAP_FLAGS_H
#define LLGL_RESOURCE_VIEW_HEAP_FLAGS_H


#include "Export.h"
#include "Texture.h"
#include "TextureFlags.h"
#include "Buffer.h"
#include "BufferFlags.h"
#include <vector>


namespace LLGL
{


class PipelineLayout;


/* ----- Structures ----- */

/**
\brief Resource view descriptor structure.
\see ResourceHeapDescriptor::resourceViews
*/
struct ResourceViewDescriptor
{
    //! Default constructor to initialize the resource view with a null pointer.
    ResourceViewDescriptor() = default;

    //! Constructor to initialize the descriptor with a resource. The resource view will access the entire resource.
    inline ResourceViewDescriptor(Resource* resource) :
        resource { resource }
    {
        /* Invalidate subresource views */
        textureView.format                      = Format::Undefined;
        textureView.subresource.baseArrayLayer  = 0;
        textureView.subresource.numArrayLayers  = 0;
        textureView.subresource.baseMipLevel    = 0;
        textureView.subresource.numMipLevels    = 0;
    }

    //! Constructor to initialize a descriptor with a texture subresource view.
    inline ResourceViewDescriptor(Texture* texture, const TextureViewDescriptor& subresourceDesc) :
        resource    { texture         },
        textureView { subresourceDesc }
    {
    }

    #if 0//TODO
    //! Constructor to initialize a descriptor with a buffer subresource view.
    inline ResourceViewDescriptor(Buffer* buffer, const BufferViewDescriptor& subresourceDesc) :
        resource   { buffer          },
        bufferView { subresourceDesc }
    {
    }
    #endif

    //! Pointer to the hardware resoudce.
    Resource*               resource    = nullptr;

    /**
    \brief Optional texture view descriptor.
    \remarks Can be used to declare a subresource view of a texture resource.
    \remarks This attribute is ignored if either \c numMipLevels or \c numArrayLayers of the TextureSubresource structure is zero.
    \todo Not implemented yet.
    */
    TextureViewDescriptor   textureView;

    #if 0//TODO
    //! Optional buffer view descriptor.
    BufferViewDescriptor    bufferView;
    #endif
};

/**
\brief Resource heap descriptor structure.
\remarks For the render systems of modern graphics APIs (i.e. Vulkan and Direct3D 12), a resource heap is the only way to bind hardware resources to a shader pipeline.
The resource heap is a container for one or more resources such as textures, samplers, constant buffers etc.
\see RenderSystem::CreateResourceHeap
*/
struct ResourceHeapDescriptor
{
    //! Reference to the pipeline layout. This must not be null, when a resource heap is created.
    PipelineLayout*                     pipelineLayout = nullptr;

    /**
    \brief List of all resource view descriptors.
    \remarks These resources must be specified in the same order as they were specified when the pipeline layout was created.
    \see PipelineLayoutDescriptor::bindings
    */
    std::vector<ResourceViewDescriptor> resourceViews;
};


} // /namespace LLGL


#endif



// ================================================================================
