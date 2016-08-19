/*
 * GLGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGraphicsPipeline.h"
#include "../GLExtensions.h"
#include "../GLTypes.h"
#include "../GLCore.h"


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

static void Convert(GLBlend& to, const BlendTargetDescriptor& from)
{
    to.srcColor     = GLTypes::Map(from.srcColor);
    to.destColor    = GLTypes::Map(from.destColor);
    to.srcAlpha     = GLTypes::Map(from.srcAlpha);
    to.destAlpha    = GLTypes::Map(from.destAlpha);
    to.colorMask.r  = GLBoolean(from.colorMask.r);
    to.colorMask.g  = GLBoolean(from.colorMask.g);
    to.colorMask.b  = GLBoolean(from.colorMask.b);
    to.colorMask.a  = GLBoolean(from.colorMask.a);
}

template <typename To, typename From>
void Convert(std::vector<To>& to, const std::vector<From>& from)
{
    to.resize(from.size());
    for (std::size_t i = 0, n = from.size(); i < n; ++i)
        Convert(to[i], from[i]);
}


/* ----- GLGraphicsPipeline class ----- */

GLGraphicsPipeline::GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    /* Convert depth state */
    depthTestEnabled_   = desc.depth.testEnabled;
    depthMask_          = (desc.depth.writeEnabled ? GL_TRUE : GL_FALSE);
    depthFunc_          = GLTypes::Map(desc.depth.compareOp);

    /* Convert stencil state */
    stencilTestEnabled_ = desc.stencil.testEnabled;
    Convert(stencilFront_, desc.stencil.front);
    Convert(stencilBack_, desc.stencil.back);

    /* Convert rasterizer state */
    polygonMode_        = GLTypes::Map(desc.rasterizer.polygonMode);
    cullFace_           = GLTypes::Map(desc.rasterizer.cullMode);
    frontFace_          = (desc.rasterizer.frontCCW ? GL_CCW : GL_CW);
    scissorTestEnabled_ = desc.rasterizer.scissorTestEnabled;
    depthClampEnabled_  = desc.rasterizer.depthClampEnabled;
    multiSampleEnabled_ = desc.rasterizer.multiSampleEnabled;
    lineSmoothEnabled_  = desc.rasterizer.antiAliasedLineEnabled;

    /* Convert blend state */
    blendEnabled_       = desc.blend.blendEnabled;
    Convert(blendStates_, desc.blend.targets);
}

void GLGraphicsPipeline::Bind(GLStateManager& stateMngr)
{
    /* Setup depth state */
    if (depthTestEnabled_)
    {
        stateMngr.Enable(GLState::DEPTH_TEST);
        stateMngr.SetDepthFunc(depthFunc_);
    }
    else
        stateMngr.Disable(GLState::DEPTH_TEST);

    stateMngr.SetDepthMask(depthMask_);

    /* Setup stencil state */
    if (stencilTestEnabled_)
    {
        stateMngr.Enable(GLState::STENCIL_TEST);
        stateMngr.SetStencilState(GL_FRONT, stencilFront_);
        stateMngr.SetStencilState(GL_BACK, stencilBack_);
    }
    else
        stateMngr.Disable(GLState::STENCIL_TEST);

    /* Setup rasterizer state */
    stateMngr.SetPolygonMode(polygonMode_);
    stateMngr.SetFrontFace(frontFace_);

    if (cullFace_ != 0)
    {
        stateMngr.Enable(GLState::CULL_FACE);
        stateMngr.SetCullFace(cullFace_);
    }
    else
        stateMngr.Disable(GLState::CULL_FACE);

    stateMngr.Set(GLState::SCISSOR_TEST, scissorTestEnabled_);
    stateMngr.Set(GLState::DEPTH_CLAMP, depthClampEnabled_);
    stateMngr.Set(GLState::MULTISAMPLE, multiSampleEnabled_);
    stateMngr.Set(GLState::LINE_SMOOTH, lineSmoothEnabled_);

    /* Setup blend state */
    if (blendEnabled_)
    {
        stateMngr.Enable(GLState::BLEND);
        stateMngr.SetBlendStates(blendStates_);
    }
    else
        stateMngr.Disable(GLState::BLEND);
}


} // /namespace LLGL



// ================================================================================
