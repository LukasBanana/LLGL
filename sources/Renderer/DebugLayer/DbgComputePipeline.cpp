/*
 * DbgComputePipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgComputePipeline.h"
#include "DbgCore.h"


namespace LLGL
{


DbgComputePipeline::DbgComputePipeline(ComputePipeline& instance, const ComputePipelineDescriptor& desc) :
    instance { instance },
    desc     { desc     }
{
}

void DbgComputePipeline::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}


} // /namespace LLGL



// ================================================================================
