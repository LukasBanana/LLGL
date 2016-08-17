/*
 * GLGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGraphicsPipeline.h"
#include "../GLExtensions.h"
#include "../GLTypes.h"


namespace LLGL
{


/* ----- Internal functions ----- */

static void Convert(GLViewport& to, const Viewport& from)
{
    to.x        = from.x;
    to.y        = from.y;
    to.width    = from.width;
    to.height   = from.height;
}

static void Convert(GLDepthRange& to, const Viewport& from)
{
    to.minDepth = static_cast<GLdouble>(from.minDepth);
    to.maxDepth = static_cast<GLdouble>(from.maxDepth);
}

static void Convert(GLScissor& to, const Scissor& from)
{
    to.x        = from.x;
    to.y        = from.y;
    to.width    = from.width;
    to.height   = from.height;
}

template <typename To, typename From>
void Convert(std::vector<To>& to, const std::vector<From>& from)
{
    to.resize(from.size());
    for (std::size_t i = 0, n = from.size(); i < n; ++i)
        Convert(to[i], from[i]);
}

static void Convert(GLStencil& to, const StencilStateDescriptor& from)
{
    to.func         = GLTypes::Map(from.compareOp);
    to.sfail        = GLTypes::Map(from.stencilFailOp);
    to.dpfail       = GLTypes::Map(from.depthFailOp);
    to.dppass       = GLTypes::Map(from.depthPassOp);
    to.ref          = static_cast<GLint>(from.reference);
    to.mask         = from.compareMask;
    to.writeMask    = from.writeMask;
}


/* ----- GLGraphicsPipeline class ----- */

GLGraphicsPipeline::GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    Convert(viewports_, desc.viewports);
    Convert(depthRanges_, desc.viewports);
    Convert(scissors_, desc.scissors);

    depthTestEnabled_   = desc.depth.testEnabled;
    depthWriteEnabled_  = desc.depth.writeEnabled;
    depthClampEnabled_  = desc.depth.clampEnabled;
    depthFunc_          = GLTypes::Map(desc.depth.compareOp);

    stencilTestEnabled_ = desc.stencil.testEnabled;
    Convert(stencilFront_, desc.stencil.front);
    Convert(stencilBack_, desc.stencil.back);
}

void GLGraphicsPipeline::Bind(GLStateManager& stateMngr)
{
    /* Setup viewports, depth-ranges, and scissors */
    stateMngr.SetViewports(viewports_);
    stateMngr.SetDepthRanges(depthRanges_);
    stateMngr.SetScissors(scissors_);

    /* Setup depth test */
    if (depthTestEnabled_)
    {
        stateMngr.Enable(GLState::DEPTH_TEST);
        stateMngr.Set(GLState::DEPTH_CLAMP, depthClampEnabled_);
        stateMngr.SetDepthFunc(depthFunc_);
    }
    else
        stateMngr.Disable(GLState::DEPTH_TEST);

    /* Setup stencil test */
    if (stencilTestEnabled_)
    {
        stateMngr.Enable(GLState::STENCIL_TEST);
        stateMngr.SetStencilFunc(GL_FRONT, stencilFront_);
        stateMngr.SetStencilFunc(GL_BACK, stencilBack_);
    }
    else
        stateMngr.Disable(GLState::STENCIL_TEST);
}


} // /namespace LLGL



// ================================================================================
