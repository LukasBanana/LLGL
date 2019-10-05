/*
 * PipelineState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_PIPELINE_STATE_H
#define LLGL_PIPELINE_STATE_H


#include "RenderSystemChild.h"


namespace LLGL
{


/**
\brief Graphics and compute pipeline state interface.
\see RenderSystem::CreatePipelineState
\see CommandBuffer::SetPipelineState
*/
class LLGL_EXPORT PipelineState : public RenderSystemChild
{
    LLGL_DECLARE_INTERFACE( InterfaceID::PipelineState );
};


} // /namespace LLGL


#endif



// ================================================================================
