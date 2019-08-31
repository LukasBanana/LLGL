/*
 * PipelineLayout.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PIPELINE_LAYOUT_H
#define LLGL_PIPELINE_LAYOUT_H


#include "RenderSystemChild.h"


namespace LLGL
{


/**
\brief Pipeline layout interface.
\remarks An instance of this interface provides all descriptor sets (as called in Vulkan)
or descriptor heaps (as called in Direct3D 12) for graphics and compute pipelines.
\see RenderSystem::CreatePipelineLayout
\see GraphicsPipelineDescriptor::pipelineLayout
\see ResourceHeapDescriptor::pipelineLayout
*/
class LLGL_EXPORT PipelineLayout : public RenderSystemChild
{
    LLGL_DECLARE_INTERFACE( InterfaceID::PipelineLayout );
};


} // /namespace LLGL


#endif



// ================================================================================
