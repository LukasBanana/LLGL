/*
 * DbgGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgGraphicsPipeline.h"
#include "DbgCore.h"


namespace LLGL
{


DbgGraphicsPipeline::DbgGraphicsPipeline(GraphicsPipeline& instance, const GraphicsPipelineDescriptor& desc) :
    instance { instance },
    desc     { desc     }
{
}

void DbgGraphicsPipeline::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}


} // /namespace LLGL



// ================================================================================
