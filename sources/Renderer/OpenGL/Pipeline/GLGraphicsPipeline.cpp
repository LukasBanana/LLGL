/*
 * GLGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGraphicsPipeline.h"
#include "../GLStateManager.h"
#include "../GLExtensions.h"
#include "../GLTypes.h"


namespace LLGL
{


/* ----- Internal functions ----- */

static GLViewport ConvertViewport(const Viewport& viewport)
{
    return
    {
        static_cast<GLfloat>(viewport.x),
        static_cast<GLfloat>(viewport.y),
        static_cast<GLfloat>(viewport.width),
        static_cast<GLfloat>(viewport.height)
    };
}

static GLDepthRange ConvertDepthRange(const Viewport& viewport)
{
    return
    {
        static_cast<GLdouble>(viewport.minDepth),
        static_cast<GLdouble>(viewport.maxDepth)
    };
}

static GLScissor ConvertScissor(const Scissor& scissor)
{
    return
    {
        static_cast<GLint>(scissor.x),
        static_cast<GLint>(scissor.y),
        static_cast<GLsizei>(scissor.width),
        static_cast<GLsizei>(scissor.height)
    };
}

template <typename To, typename From, typename Func>
void Convert(std::vector<To>& to, const std::vector<From>& from, Func func)
{
    to.reserve(from.size());
    for (const auto& entry : from)
        to.push_back(func(entry));
}

static void Convert(GLStencilState& to, const StencilStateDescriptor& from)
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
    Convert(viewports_, desc.viewports, ConvertViewport);
    Convert(depthRanges_, desc.viewports, ConvertDepthRange);
    Convert(scissors_, desc.scissors, ConvertScissor);

    depthTestEnabled_   = desc.depth.testEnabled;
    depthWriteEnabled_  = desc.depth.writeEnabled;
    depthRangeEnabled_  = desc.depth.rangeEnabled;
    depthCompareOp_     = GLTypes::Map(desc.depth.compareOp);

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
    stateMngr.Set(GLState::DEPTH_TEST, depthTestEnabled_);
    stateMngr.Set(GLState::DEPTH_CLAMP, depthRangeEnabled_);

    /* Setup stencil test */
    if (stencilTestEnabled_)
    {
        BindStencilFace(GL_FRONT, stencilFront_);
        BindStencilFace(GL_BACK, stencilBack_);
    }
}


/*
 * ======= Private: =======
 */

void GLGraphicsPipeline::BindStencilFace(GLenum face, const GLStencilState& state)
{
    glStencilFuncSeparate(face, state.func, state.ref, state.mask);
    glStencilMaskSeparate(face, state.writeMask);
    glStencilOpSeparate(face, state.sfail, state.dpfail, state.dppass);
}


} // /namespace LLGL



// ================================================================================
