/*
 * ResourceViewHeapFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_RESOURCE_HEAP_FLAGS_H
#define LLGL_RESOURCE_HEAP_FLAGS_H


#include "Export.h"
#include <vector>


namespace LLGL
{


class Buffer;
class Texture;
class Sampler;
class PipelineLayout;

/* ----- Enumerations ----- */

/**
\brief Resource view type enumeration.
\see LayoutBinding::type
\see ResourceViewDescriptor::type
*/
enum class ResourceViewType
{
    Sampler,        //!< Sampler state resource.
    Texture,        //!< Texture (or image) resource.
    ConstantBuffer, //!< Constant buffer (or uniform buffer) resource.
    StorageBuffer,  //!< Storage buffer resource.
};


/* ----- Structures ----- */

//! Resource view descriptor structure.
struct ResourceViewDescriptor
{
    inline ResourceViewDescriptor() :
        buffer { nullptr }
    {
    }

    //! Resource view type for this layout binding. By default ResourceViewType::ConstantBuffer.
    ResourceViewType    type    = ResourceViewType::ConstantBuffer;

    union
    {
        Buffer*         buffer;
        Texture*        texture;
        Sampler*        sampler;
    };
};

//! Resource view heap descriptor structure.
struct ResourceViewHeapDescriptor
{
    //! Reference to the pipeline layout.
    PipelineLayout*                     pipelineLayout = nullptr;

    //! List of all resource view descriptors.
    std::vector<ResourceViewDescriptor> resourceViews;
};


} // /namespace LLGL


#endif



// ================================================================================
