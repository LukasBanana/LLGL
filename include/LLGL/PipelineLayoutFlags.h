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
#include <vector>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Layout binding type enumeration.
\see LayoutBinding::type
*/
enum class LayoutBindingType
{
    Sampler,        //!< Sampler state resource.
    Texture,        //!< Texture (or image) resource.
    ConstantBuffer, //!< Constant buffer (or uniform buffer) resource.
    StorageBuffer,  //!< Storage buffer resource.
};


/* ----- Structures ----- */

struct LayoutBinding
{
    //! Buffer type for this layout binding. By default LayoutBindingType::ConstantBuffer.
    LayoutBindingType   type        = LayoutBindingType::ConstantBuffer;

    /**
    \brief Specifies the first zero-based binding slot. By default 0.
    \note For Vulkan, each binding slot of all layout bindings must have a different value within a pipeline layout.
    */
    std::uint32_t       startSlot   = 0;

    //! Specifies the number of binding slots. By default 1.
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
