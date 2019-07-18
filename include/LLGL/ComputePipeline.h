/*
 * ComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMPUTE_PIPELINE_H
#define LLGL_COMPUTE_PIPELINE_H


#include "RenderSystemChild.h"
#include "ComputePipelineFlags.h"


namespace LLGL
{


//! Compute pipeline interface.
class LLGL_EXPORT ComputePipeline : public RenderSystemChild
{
    LLGL_DECLARE_INTERFACE( InterfaceID::ComputePipeline );
};


} // /namespace LLGL


#endif



// ================================================================================
