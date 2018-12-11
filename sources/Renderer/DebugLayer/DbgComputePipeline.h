/*
 * DbgComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_COMPUTE_PIPELINE_H
#define LLGL_DBG_COMPUTE_PIPELINE_H


#include <LLGL/ComputePipeline.h>


namespace LLGL
{


class DbgComputePipeline : public ComputePipeline
{

    public:

        DbgComputePipeline(ComputePipeline& instance, const ComputePipelineDescriptor& desc) :
            instance { instance },
            desc     { desc     }
        {
        }

        ComputePipeline&                instance;
        const ComputePipelineDescriptor desc;

};


} // /namespace LLGL


#endif



// ================================================================================
