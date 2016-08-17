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
    depthRangeEnabled_  = desc.depth.rangeEnabled;
    depthFunc_          = GLTypes::Map(desc.depth.compareOp);

    stencilTestEnabled_ = desc.stencil.testEnabled;
    Convert(stencilFront_, desc.stencil.front);
    Convert(stencilBack_, desc.stencil.back);
}

void GLGraphicsPipeline::Bind(GLStateManager& stateMngr)
{
    /* Setup viewports */
    if (viewports_.size() == 1)
    {
        const auto& v = viewports_.front();
        glViewport(
            static_cast<GLint>(v.x),
            static_cast<GLint>(v.y),
            static_cast<GLsizei>(v.width),
            static_cast<GLsizei>(v.height)
        );
    }
    else if (viewports_.size() > 1 && glViewportArrayv)
    {
        glViewportArrayv(
            0,
            static_cast<GLsizei>(viewports_.size()),
            reinterpret_cast<const GLfloat*>(viewports_.data())
        );
    }

    /* Setup depth ranges */
    if (depthRanges_.size() == 1)
    {
        const auto& dr = depthRanges_.front();
        glDepthRange(dr.minDepth, dr.maxDepth);
    }
    else if (depthRanges_.size() > 1 && glDepthRangeArrayv)
    {
        glDepthRangeArrayv(
            0,
            static_cast<GLsizei>(depthRanges_.size()),
            reinterpret_cast<const GLdouble*>(depthRanges_.data())
        );
    }

    /* Setup scissors */
    if (scissors_.size() == 1)
    {
        const auto& s = scissors_.front();
        glScissor(s.x, s.y, s.width, s.height);
    }
    else if (scissors_.size() > 1 && glScissorArrayv)
    {
        glScissorArrayv(
            0,
            static_cast<GLsizei>(scissors_.size()),
            reinterpret_cast<const GLint*>(scissors_.data())
        );
    }

    /* Setup depth test */
    if (depthTestEnabled_)
    {
        stateMngr.Enable(GLState::DEPTH_TEST);
        stateMngr.Set(GLState::DEPTH_CLAMP, depthRangeEnabled_);
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
