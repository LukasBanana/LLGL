/*
 * ComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_COMPUTE_PIPELINE_H
#define LLGL_COMPUTE_PIPELINE_H


#include "Export.h"
#include "ComputePipelineFlags.h"


namespace LLGL
{


//! Compute pipeline interface.
class LLGL_EXPORT ComputePipeline
{

    public:

        virtual ~ComputePipeline()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
