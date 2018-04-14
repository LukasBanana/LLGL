/*
 * PipelineLayoutFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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


/* ----- Enumerations ----- */

//! Pipeline type enumeration.
enum class PipelineType
{
    Graphics,   //!< Specifies the graphics shader pipeline.
    Compute,    //!< Specifies the compute shader pipeline.
};


/* ----- Structures ----- */

/**
\brief Layout structure for a single binding point of the pipeline layout descriptor.
\see PipelineLayoutDescriptor
*/
struct LayoutBinding
{
    //! Resource view type for this layout binding. By default ResourceViewType::ConstantBuffer.
    ResourceViewType    type        = ResourceViewType::ConstantBuffer;

    /**
    \brief Specifies the first zero-based binding slot. By default 0.
    \note For Vulkan, each binding slot of all layout bindings must have a different value within a pipeline layout.
    */
    std::uint32_t       startSlot   = 0;

    /**
    \brief Specifies the number of binding slots. By default 1.
    \note For Vulkan, this number specifies the size of an array of resources (e.g. an array of uniform buffers).
    */
    std::uint32_t       numSlots    = 1;

    //! Specifies which shader stages are affected by this layout binding. By default all shader stages are affected.
    long                stageFlags  = ShaderStageFlags::AllStages;
};

/**
\brief Pipeline layout descritpor structure.
\remarks Contains all layout bindings that will be used by graphics and compute pipelines.
*/
struct PipelineLayoutDescriptor
{
    std::vector<LayoutBinding> bindings; //!< List of layout resource bindings.
};


} // /namespace LLGL


#endif



// ================================================================================
