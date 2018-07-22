/*
 * GLGraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGraphicsPipeline.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLCore.h"
#include "../../CheckedCast.h"
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


/* ----- Internal functions ----- */

static void Convert(GLStencil& to, const StencilFaceDescriptor& from)
{
    to.sfail        = GLTypes::Map(from.stencilFailOp);
    to.dpfail       = GLTypes::Map(from.depthFailOp);
    to.dppass       = GLTypes::Map(from.depthPassOp);
    to.func         = GLTypes::Map(from.compareOp);
    to.ref          = static_cast<GLint>(from.reference);
    to.mask         = from.readMask;
    to.writeMask    = from.writeMask;
}

static void Convert(GLBlend& to, const BlendTargetDescriptor& from)
{
    to.srcColor     = GLTypes::Map(from.srcColor);
    to.dstColor     = GLTypes::Map(from.dstColor);
    to.funcColor    = GLTypes::Map(from.colorArithmetic);
    to.srcAlpha     = GLTypes::Map(from.srcAlpha);
    to.dstAlpha     = GLTypes::Map(from.dstAlpha);
    to.funcAlpha    = GLTypes::Map(from.alphaArithmetic);
    to.colorMask.r  = GLBoolean(from.colorMask.r);
    to.colorMask.g  = GLBoolean(from.colorMask.g);
    to.colorMask.b  = GLBoolean(from.colorMask.b);
    to.colorMask.a  = GLBoolean(from.colorMask.a);
}

template <typename DstType, typename SrcType>
void Convert(std::vector<DstType>& to, const std::vector<SrcType>& from)
{
    to.resize(from.size());
    for (std::size_t i = 0, n = from.size(); i < n; ++i)
        Convert(to[i], from[i]);
}

static bool IsBlendColorNeeded(const BlendOp blendOp)
{
    return (blendOp == BlendOp::BlendFactor || blendOp == BlendOp::InvBlendFactor);
}

// Returns true if the specified blend description requires that "glBlendColor" is called when the blend state is bound
static bool IsBlendColorNeeded(const BlendDescriptor& blendDesc)
{
    if (!blendDesc.blendEnabled)
        return false;

    for (const auto& target : blendDesc.targets)
    {
        if ( IsBlendColorNeeded(target.srcColor) ||
             IsBlendColorNeeded(target.srcAlpha) ||
             IsBlendColorNeeded(target.dstColor) ||
             IsBlendColorNeeded(target.dstAlpha) )
        {
            return true;
        }
    }

    return false;
}

static GLState PolygonModeToPolygonOffset(const PolygonMode mode)
{
    switch (mode)
    {
        case PolygonMode::Fill:         return GLState::POLYGON_OFFSET_FILL;
        case PolygonMode::Wireframe:    return GLState::POLYGON_OFFSET_LINE;
        case PolygonMode::Points:       return GLState::POLYGON_OFFSET_POINT;
    }
    throw std::invalid_argument("failed to map 'PolygonMode' to polygon offset mode (GL_POLYGON_OFFSET_FILL, GL_POLYGON_OFFSET_LINE, or GL_POLYGON_OFFSET_POINT)");
}

static bool IsPolygonOffsetEnabled(const DepthBiasDescriptor& desc)
{
    /* Ignore clamp factor for this check, since it's useless without the other two parameters */
    return (desc.slopeFactor != 0.0f || desc.constantFactor != 0.0f);
}


/* ----- GLGraphicsPipeline class ----- */

GLGraphicsPipeline::GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits)
{
    /* Convert shader state */
    shaderProgram_ = LLGL_CAST(const GLShaderProgram*, desc.shaderProgram);
    if (!shaderProgram_)
        throw std::invalid_argument("failed to create graphics pipeline due to missing shader program");

    /* Convert input-assembler state */
    drawMode_ = GLTypes::Map(desc.primitiveTopology);

    if (IsPrimitiveTopologyPatches(desc.primitiveTopology))
    {
        /* Store patch vertices and check limit */
        const auto patchSize = GetPrimitiveTopologyPatchSize(desc.primitiveTopology);
        if (patchSize > limits.maxPatchVertices)
        {
            throw std::runtime_error(
                "renderer does not support " + std::to_string(patchVertices_) +
                " control points for patches (limit is " + std::to_string(limits.maxPatchVertices) + ")"
            );
        }
        else
            patchVertices_ = static_cast<GLint>(patchSize);
    }
    else
        patchVertices_ = 0;

    /* Convert depth state */
    depthTestEnabled_       = desc.depth.testEnabled;
    depthMask_              = (desc.depth.writeEnabled ? GL_TRUE : GL_FALSE);
    depthFunc_              = GLTypes::Map(desc.depth.compareOp);

    /* Convert stencil state */
    stencilTestEnabled_     = desc.stencil.testEnabled;
    Convert(stencilFront_, desc.stencil.front);
    Convert(stencilBack_, desc.stencil.back);

    /* Convert rasterizer state */
    polygonMode_            = GLTypes::Map(desc.rasterizer.polygonMode);
    cullFace_               = GLTypes::Map(desc.rasterizer.cullMode);
    frontFace_              = (desc.rasterizer.frontCCW ? GL_CCW : GL_CW);
    scissorTestEnabled_     = desc.rasterizer.scissorTestEnabled;
    depthClampEnabled_      = desc.rasterizer.depthClampEnabled;
    multiSampleEnabled_     = desc.rasterizer.multiSampling.enabled;
    sampleMask_             = desc.rasterizer.multiSampling.sampleMask;
    lineSmoothEnabled_      = desc.rasterizer.antiAliasedLineEnabled;
    lineWidth_              = desc.rasterizer.lineWidth;
    polygonOffsetEnabled_   = IsPolygonOffsetEnabled(desc.rasterizer.depthBias);
    polygonOffsetMode_      = PolygonModeToPolygonOffset(desc.rasterizer.polygonMode);
    polygonOffsetFactor_    = desc.rasterizer.depthBias.slopeFactor;
    polygonOffsetUnits_     = desc.rasterizer.depthBias.constantFactor;
    polygonOffsetClamp_     = desc.rasterizer.depthBias.clamp;

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    conservativeRaster_ = desc.rasterizer.conservativeRasterization;
    #endif

    /* Convert blend state */
    blendEnabled_           = desc.blend.blendEnabled;
    blendColor_             = desc.blend.blendFactor;
    blendColorNeeded_       = IsBlendColorNeeded(desc.blend);
    Convert(blendStates_, desc.blend.targets);
    sampleAlphaToCoverage_  = desc.blend.alphaToCoverageEnabled;

    /* Convert color logic operation state */
    if (desc.blend.logicOp != LogicOp::Disabled)
    {
        logicOpEnabled_ = true;
        logicOp_        = GLTypes::Map(desc.blend.logicOp);
    }
}

void GLGraphicsPipeline::Bind(GLStateManager& stateMngr)
{
    /* Bind shader program and discard rasterizer if there is no fragment shader */
    stateMngr.BindShaderProgram(shaderProgram_->GetID());
    stateMngr.Set(GLState::RASTERIZER_DISCARD, !shaderProgram_->HasFragmentShader());

    /* Setup input-assembler state */
    if (patchVertices_ > 0)
        stateMngr.SetPatchVertices(patchVertices_);

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

    if (polygonOffsetEnabled_)
    {
        stateMngr.Enable(polygonOffsetMode_);
        stateMngr.SetPolygonOffset(polygonOffsetFactor_, polygonOffsetUnits_, polygonOffsetClamp_);
    }
    else
        stateMngr.Disable(polygonOffsetMode_);

    stateMngr.Set(GLState::SCISSOR_TEST, scissorTestEnabled_);
    stateMngr.Set(GLState::DEPTH_CLAMP, depthClampEnabled_);
    stateMngr.Set(GLState::MULTISAMPLE, multiSampleEnabled_);
    stateMngr.Set(GLState::LINE_SMOOTH, lineSmoothEnabled_);
    stateMngr.SetLineWidth(lineWidth_);

    #ifdef LLGL_GL_ENABLE_VENDOR_EXT
    stateMngr.Set(GLStateExt::CONSERVATIVE_RASTERIZATION, conservativeRaster_);
    #endif

    /* Setup blend state */
    stateMngr.Set(GLState::BLEND, blendEnabled_);
    stateMngr.SetBlendStates(blendStates_, blendEnabled_);

    if (blendColorNeeded_)
        stateMngr.SetBlendColor(blendColor_);

    if (multiSampleEnabled_)
    {
        #if 0//TODO
        stateMngr.SetSampleMask(sampleMask_);
        #endif
        stateMngr.Set(GLState::SAMPLE_ALPHA_TO_COVERAGE, sampleAlphaToCoverage_);
    }

    /* Setup color logic operation */
    if (logicOpEnabled_)
    {
        stateMngr.Enable(GLState::COLOR_LOGIC_OP);
        stateMngr.SetLogicOp(logicOp_);
    }
    else
        stateMngr.Disable(GLState::COLOR_LOGIC_OP);
}


} // /namespace LLGL



// ================================================================================
