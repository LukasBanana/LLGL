/*
 * PipelineState.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PIPELINE_STATE_H
#define LLGL_PIPELINE_STATE_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/Report.h>


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

    public:

        /**
        \brief Returns a pointer to the report or null if there is none.
        \remarks If there is a report, it might contain warnings and/or errors from the PSO and shader compilation process.
        \see Report
        */
        virtual const Report* GetReport() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
