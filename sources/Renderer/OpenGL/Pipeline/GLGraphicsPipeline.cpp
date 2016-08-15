/*
 * GLGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGraphicsPipeline.h"
#include "../GLStateManager.h"


namespace LLGL
{


GLGraphicsPipeline::GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc) :
    viewports_          ( desc.viewports          ),
    scissors_           ( desc.scissors           ),
    depthTestEnabled_   ( desc.depth.testEnabled  ),
    depthWriteEnabled_  ( desc.depth.writeEnabled ),
    depthRangeEnabled_  ( desc.depth.rangeEnabled )
{
}

void GLGraphicsPipeline::Bind(GLStateManager& stateMngr)
{
    stateMngr.Set(GLState::DEPTH_TEST, depthTestEnabled_);
    stateMngr.Set(GLState::DEPTH_CLAMP, depthRangeEnabled_);
    //todo...
}


} // /namespace LLGL



// ================================================================================
