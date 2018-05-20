/*
 * PipelineLayoutFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PIPELINE_LAYOUT_FLAGS_H
#define LLGL_PIPELINE_LAYOUT_FLAGS_H


#include "Export.h"
#include "BufferFlags.h"
#include "ShaderFlags.h"
#include "ResourceViewHeapFlags.h"
#include <vector>


namespace LLGL
{


/* ----- Structures ----- */

/**
\brief Layout structure for a single binding point of the pipeline layout descriptor.
\see PipelineLayoutDescriptor
*/
struct BindingDescriptor
{
    BindingDescriptor() = default;
    BindingDescriptor(const BindingDescriptor&) = default;

    //! Constructors with all attributes and a default value for a uniform array.
    inline BindingDescriptor(ResourceType type, long stageFlags, std::uint32_t slot, std::uint32_t arraySize = 1) :
        type       { type       },
        stageFlags { stageFlags },
        slot       { slot       },
        arraySize  { arraySize  }
    {
    }

    //! Resource view type for this layout binding. By default ResourceType::ConstantBuffer.
    ResourceType    type        = ResourceType::ConstantBuffer;

    /**
    \brief Specifies which shader stages are affected by this layout binding. By default 0.
    \remarks This can be a bitwise OR combination of the StageFlags bitmasks.
    \see StageFlags
    */
    long            stageFlags  = 0;

    /**
    \brief Specifies the zero-based binding slot. By default 0.
    \note For Vulkan, each binding slot of all layout bindings must have a different value within a pipeline layout.
    */
    std::uint32_t   slot        = 0;

    /**
    \brief Specifies the number of binding slots for an array resource. By default 1.
    \note For Vulkan, this number specifies the size of an array of resources (e.g. an array of uniform buffers).
    */
    std::uint32_t   arraySize   = 1;
};

/**
\brief Pipeline layout descritpor structure.
\remarks Contains all layout bindings that will be used by graphics and compute pipelines.
*/
struct PipelineLayoutDescriptor
{
    std::vector<BindingDescriptor> bindings; //!< List of layout resource bindings.
};


} // /namespace LLGL


#endif



// ================================================================================
