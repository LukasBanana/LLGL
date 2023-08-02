/*
 * ResourceViewHeapFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RESOURCE_VIEW_HEAP_FLAGS_H
#define LLGL_RESOURCE_VIEW_HEAP_FLAGS_H


#include <LLGL/Export.h>
#include <LLGL/Texture.h>
#include <LLGL/TextureFlags.h>
#include <LLGL/Buffer.h>
#include <LLGL/BufferFlags.h>
#include <vector>


namespace LLGL
{


class PipelineLayout;


/* ----- Flags ----- */

/**
\brief Flags for memory barriers in resource heaps.
\see ResourceHeapDescriptor::barrierFlags
*/
struct BarrierFlags
{
    enum
    {
        /**
        \brief Memory barrier for Buffer resources that were created with the BindFlags::Storage bind flags.
        \remarks Shader access to the buffer will reflect all data written to by previous shaders.
        \see BindFlags::Storage
        */
        StorageBuffer   = (1 << 0),

        /**
        \brief Memory barrier for Texture resources that were created with the BindFlags::Storage bind flags.
        \remarks Shader access to the texture will reflect all data written to by previous shaders.
        \see BindFlags::Storage
        */
        StorageTexture  = (1 << 1),

        /**
        \brief Memory barrier for any storage resource. This is just a bitwise OR combination of \c StorageBuffer and \c StorageTexture.
        \remarks Renderer backends such as Direct3D 12 and Vulkan have bookkeeping for storage resources
        and don't have to distinguish between Buffer and Texture resource views for their barriers at time of creating the ResourceHeap.
        Hence, using BarrierFlags::Storage by default when any resource views in the ResourceHeap have to be synchronized is recommended.
        Only the OpenGL backend has to know at creation time what type of resources need a global barrier via \c glMemoryBarrier.
        \see BindFlags::Storage
        */
        Storage         = (StorageBuffer | StorageTexture),
    };
};


/* ----- Structures ----- */

/**
\brief Resource view descriptor structure.
\see RenderSystem::CreateResourceHeap
\see RenderSystem::WriteResourceHeap
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
        textureView.format = Format::Undefined;
    }

    //! Constructor to initialize a descriptor with a texture subresource view.
    inline ResourceViewDescriptor(Texture* texture, const TextureViewDescriptor& subresourceDesc) :
        resource    { texture         },
        textureView { subresourceDesc }
    {
    }

    //! Constructor to initialize a descriptor with a buffer subresource view.
    inline ResourceViewDescriptor(Buffer* buffer, const BufferViewDescriptor& subresourceDesc) :
        resource   { buffer          },
        bufferView { subresourceDesc }
    {
        /* Invalidate subresource views */
        textureView.format = Format::Undefined;
    }

    /**
    \brief Pointer to the hardware resource.
    \see This \e can be null when passed to a ResourceHeap to skip over resources that are intended to be unchanged.
    This way a single \c WriteResourceHeap invocation can be used to write a partial range of resource views.
    */
    Resource*               resource        = nullptr;

    /**
    \brief Optional texture view descriptor.
    \remarks Can be used to declare a subresource view of a texture resource.
    \remarks This attribute is ignored if \e one of the following sub members has the respective value listed below:
    - \c textureView.format is Format::Undefined
    - \c textureView.subresource.numMipLevels is 0
    - \c textureView.subresource.numArrayLayers is 0
    */
    TextureViewDescriptor   textureView;

    /**
    \brief Optional buffer view descriptor.
    \remarks Can be used to declare a subresource view of a buffer resource.
    \remarks This attribute is ignored if \e all of the following sub members have the respective value listed below:
    - \c bufferView.format is Format::Undefined;
    - \c bufferView.offset is 0.
    - \c bufferView.size is \c Constants::wholeSize.
    */
    BufferViewDescriptor    bufferView;

    /**
    \brief Initial counter value for an \c AppendStructuredBuffer and \c ConsumeStructuredBuffer in HLSL.
    \remarks This is only used for HLSL (D3D) to initialize the hidden counter of appendable and consumable unordered access views (UAV).
    This will be used in the D3D backends for buffer resources that have been created with either the MiscFlags::Append or MiscFlags::Counter flags.
    \note Only supported with: Direct3D 11, Direct3D 12.
    \see MiscFlags::Append
    \see MiscFlags::Counter
    */
    std::uint32_t           initialCount    = 0;
};

/**
\brief Resource heap descriptor structure.
\remarks For the render systems of modern graphics APIs (i.e. Vulkan and Direct3D 12), a resource heap is the only way to bind hardware resources to a shader pipeline.
The resource heap is a container for one or more resources such as textures, samplers, constant buffers etc.
\see RenderSystem::CreateResourceHeap
*/
struct ResourceHeapDescriptor
{
    ResourceHeapDescriptor() = default;

    //! Initializes the resource heap descriptor with the specified pipeline layout and optional secondary parameters.
    inline ResourceHeapDescriptor(PipelineLayout* pipelineLayout, std::uint32_t numResourceViews = 0, long barrierFlags = 0) :
        pipelineLayout   { pipelineLayout   },
        numResourceViews { numResourceViews },
        barrierFlags     { barrierFlags     }
    {
    }

    //! Reference to the pipeline layout. This must not be null, when a resource heap is created.
    PipelineLayout* pipelineLayout      = nullptr;

    /**
    \brief Specifies the number of resource views.
    \remarks If the number of resource views is non-zero, it \b must a multiple of the heap-bindings in the pipeline layout.
    \remarks If the number of resource views is zero, the number will be determined by the initial resource views
    and they must \e not be empty and they \b must be a multiple of the heap-bindings in the pipeline layout.
    \see PipelineLayoutDescriptor::heapBindings
    \see RenderSystem::CreateResourceHeap
    */
    std::uint32_t   numResourceViews    = 0;

    /**
    \brief Specifies optional resource barrier flags. By default 0.
    \remarks If the barrier flags are non-zero, they will be applied before any resource are bound to the graphics/compute pipeline.
    This should be used when a resource is bound to the pipeline that was previously written to.
    \see BarrierFlags
    */
    long            barrierFlags        = 0;
};


} // /namespace LLGL


#endif



// ================================================================================
