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
#include "../../StaticLimits.h"
#include "../../../Core/Helper.h"
#include "../../../Core/RawBufferIterator.h"
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


/* ----- Internal functions ----- */

static void Convert(GLStencil& dst, const StencilFaceDescriptor& src)
{
    dst.sfail        = GLTypes::Map(src.stencilFailOp);
    dst.dpfail       = GLTypes::Map(src.depthFailOp);
    dst.dppass       = GLTypes::Map(src.depthPassOp);
    dst.func         = GLTypes::Map(src.compareOp);
    dst.ref          = static_cast<GLint>(src.reference);
    dst.mask         = src.readMask;
    dst.writeMask    = src.writeMask;
}

static void Convert(GLBlend& dst, const BlendTargetDescriptor& src)
{
    dst.srcColor     = GLTypes::Map(src.srcColor);
    dst.dstColor     = GLTypes::Map(src.dstColor);
    dst.funcColor    = GLTypes::Map(src.colorArithmetic);
    dst.srcAlpha     = GLTypes::Map(src.srcAlpha);
    dst.dstAlpha     = GLTypes::Map(src.dstAlpha);
    dst.funcAlpha    = GLTypes::Map(src.alphaArithmetic);
    dst.colorMask[0] = GLBoolean(src.colorMask.r);
    dst.colorMask[1] = GLBoolean(src.colorMask.g);
    dst.colorMask[2] = GLBoolean(src.colorMask.b);
    dst.colorMask[3] = GLBoolean(src.colorMask.a);
}

static bool IsBlendColorNeeded(const BlendOp blendOp)
{
    return (blendOp == BlendOp::BlendFactor || blendOp == BlendOp::InvBlendFactor);
}

// Returns true if the specified blend description requires that "glBlendColor" is called when the blend state is bound
static bool IsBlendColorNeeded(const BlendDescriptor& blendDesc)
{
    for (const auto& target : blendDesc.targets)
    {
        if (target.blendEnabled)
        {
            if ( IsBlendColorNeeded(target.srcColor) ||
                 IsBlendColorNeeded(target.srcAlpha) ||
                 IsBlendColorNeeded(target.dstColor) ||
                 IsBlendColorNeeded(target.dstAlpha) )
            {
                return true;
            }
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

    /* Determine if any blend target is enabled */
    if (desc.blend.logicOp == LogicOp::Disabled)
    {
        if (desc.blend.independentBlendEnabled)
        {
            /* Check if any blend target is enabled */
            for (std::uint32_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
            {
                if (desc.blend.targets[i].blendEnabled)
                {
                    anyBlendTargetEnabled_  = true;
                    numBlendStates_         = std::max(numBlendStates_, static_cast<std::uint8_t>(i + 1u));
                }
            }

            /* Convert all blend targets */
            for (int i = 0; i < 8; ++i)
                Convert(blendStates_[i], desc.blend.targets[i]);
        }
        else
        {
            /* Take information only from first target */
            anyBlendTargetEnabled_  = desc.blend.targets[0].blendEnabled;
            numBlendStates_         = 1;

            /* Convert only first target */
            Convert(blendStates_[0], desc.blend.targets[0]);
        }
    }
    else
    {
        /* Convert logic pixel operation */
        logicOpEnabled_ = true;
        logicOp_        = GLTypes::Map(desc.blend.logicOp);
    }

    /* Convert blend state */
    blendColor_             = desc.blend.blendFactor;
    blendColorNeeded_       = IsBlendColorNeeded(desc.blend);
    sampleAlphaToCoverage_  = desc.blend.alphaToCoverageEnabled;

    /* Build static state buffer for viewports and scissors */
    if (!desc.viewports.empty() || !desc.scissors.empty())
        BuildStaticStateBuffer(desc);
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
    stateMngr.Set(GLState::BLEND, anyBlendTargetEnabled_);
    stateMngr.SetBlendStates(numBlendStates_, blendStates_, anyBlendTargetEnabled_);

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

    /* Set static viewports and scissors */
    if (staticStateBuffer_)
    {
        RawBufferIterator rawBufferIter { staticStateBuffer_.get() };
        if (numStaticViewports_ > 0)
            SetStaticViewports(stateMngr, rawBufferIter);
        if (numStaticScissors_ > 0)
            SetStaticScissors(stateMngr, rawBufferIter);
    }
}


/*
 * ======= Private: =======
 */

void GLGraphicsPipeline::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
{
    /* Allocate packed raw buffer */
    const std::size_t bufferSize =
    (
        desc.viewports.size() * (sizeof(GLViewport) + sizeof(GLDepthRange)) +
        desc.scissors.size()  * (sizeof(GLScissor))
    );
    staticStateBuffer_ = MakeUniqueArray<char>(bufferSize);

    RawBufferIterator rawBufferIter { staticStateBuffer_.get() };

    /* Build static viewports in raw buffer */
    if (!desc.viewports.empty())
        BuildStaticViewports(desc.viewports.size(), desc.viewports.data(), rawBufferIter);

    /* Build static scissors in raw buffer */
    if (!desc.scissors.empty())
        BuildStaticScissors(desc.scissors.size(), desc.scissors.data(), rawBufferIter);
}

void GLGraphicsPipeline::BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, RawBufferIterator& rawBufferIter)
{
    /* Store number of viewports and validate limit */
    numStaticViewports_ = static_cast<GLsizei>(numViewports);

    if (numStaticViewports_ > LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS)
    {
        throw std::invalid_argument(
            "too many viewports in graphics pipeline state (" + std::to_string(numStaticViewports_) +
            " specified, but limit is " + std::to_string(LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS) + ")"
        );
    }

    /* Build <GLViewport> entries */
    for (std::size_t i = 0; i < numViewports; ++i)
    {
        auto dst = rawBufferIter.Next<GLViewport>();
        {
            dst->x      = static_cast<GLfloat>(viewports[i].x);
            dst->y      = static_cast<GLfloat>(viewports[i].y);
            dst->width  = static_cast<GLfloat>(viewports[i].width);
            dst->height = static_cast<GLfloat>(viewports[i].height);
        }
    }

    /* Build <GLDepthRange> entries */
    for (std::size_t i = 0; i < numViewports; ++i)
    {
        auto dst = rawBufferIter.Next<GLDepthRange>();
        {
            dst->minDepth = static_cast<GLdouble>(viewports[i].minDepth);
            dst->maxDepth = static_cast<GLdouble>(viewports[i].maxDepth);
        }
    }
}

void GLGraphicsPipeline::BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, RawBufferIterator& rawBufferIter)
{
    /* Store number of scissors and validate limit */
    numStaticScissors_ = static_cast<GLsizei>(numScissors);

    if (numStaticScissors_ > LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS)
    {
        throw std::invalid_argument(
            "too many scissors in graphics pipeline state (" + std::to_string(numStaticScissors_) +
            " specified, but limit is " + std::to_string(LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS) + ")"
        );
    }

    /* Build <GLScissor> entries */
    for (std::size_t i = 0; i < numScissors; ++i)
    {
        auto dst = rawBufferIter.Next<GLScissor>();
        {
            dst->x      = static_cast<GLint>(scissors[i].x);
            dst->y      = static_cast<GLint>(scissors[i].y);
            dst->width  = static_cast<GLsizei>(scissors[i].width);
            dst->height = static_cast<GLsizei>(scissors[i].height);
        }
    }
}

void GLGraphicsPipeline::SetStaticViewports(GLStateManager& stateMngr, RawBufferIterator& rawBufferIter)
{
    /* Copy viewports to intermediate array (must be adjusted when framebuffer is bound) */
    GLViewport intermediateViewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
    ::memcpy(
        intermediateViewports,
        rawBufferIter.Next<GLViewport>(numStaticViewports_),
        numStaticViewports_ * sizeof(GLViewport)
    );
    stateMngr.SetViewportArray(0, numStaticViewports_, intermediateViewports);

    /* Set depth ranges */
    stateMngr.SetDepthRangeArray(0, numStaticViewports_, rawBufferIter.Next<GLDepthRange>(numStaticViewports_));
}

void GLGraphicsPipeline::SetStaticScissors(GLStateManager& stateMngr, RawBufferIterator& rawBufferIter)
{
    /* Copy scissors to intermediate array (must be adjusted when framebuffer is bound) */
    GLScissor intermediateScissors[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
    ::memcpy(
        intermediateScissors,
        rawBufferIter.Next<GLScissor>(numStaticScissors_),
        numStaticScissors_ * sizeof(GLScissor)
    );
    stateMngr.SetScissorArray(0, numStaticScissors_, intermediateScissors);
}


} // /namespace LLGL



// ================================================================================
