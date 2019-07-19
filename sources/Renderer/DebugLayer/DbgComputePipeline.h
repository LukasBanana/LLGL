/*
 * DbgComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_COMPUTE_PIPELINE_H
#define LLGL_DBG_COMPUTE_PIPELINE_H


#include <LLGL/ComputePipeline.h>
#include <LLGL/ComputePipelineFlags.h>
#include <string>


namespace LLGL
{


class DbgComputePipeline final : public ComputePipeline
{

    public:

        void SetName(const char* name) override;

    public:

        DbgComputePipeline(ComputePipeline& instance, const ComputePipelineDescriptor& desc);

    public:

        ComputePipeline&                instance;
        const ComputePipelineDescriptor desc;
        std::string                     label;

};


} // /namespace LLGL


#endif



// ================================================================================
