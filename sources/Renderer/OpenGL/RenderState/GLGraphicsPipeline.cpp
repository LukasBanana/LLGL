/*
 * GLGraphicsPipeline.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGraphicsPipeline.h"
#include "GLRenderPass.h"
#include "GLStatePool.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLCore.h"
#include "../../CheckedCast.h"
#include "../../StaticLimits.h"
#include "../../../Core/Helper.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/GraphicsPipelineFlags.h>


namespace LLGL
{


// Returns true if the specified pipeline layout contains any names in the descriptor.
static bool AnyNamesInPipelineLayout(const GLPipelineLayout& pipelineLayout)
{
    for (const auto& binding : pipelineLayout.GetBindings())
    {
        if (!binding.name.empty())
            return true;
    }
    return false;
}

GLGraphicsPipeline::GLGraphicsPipeline(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits)
{
    /* Convert shader state */
    shaderProgram_ = LLGL_CAST(const GLShaderProgram*, desc.shaderProgram);
    if (!shaderProgram_)
        throw std::invalid_argument("failed to create graphics pipeline due to missing shader program");

    /* Create shader binding layout by binding descriptor */
    if (desc.pipelineLayout)
    {
        /* Ignore pipeline layout if there are no names specified, because no valid binding layout can be created then */
        auto pipelineLayoutGL = LLGL_CAST(const GLPipelineLayout*, desc.pipelineLayout);
        if (AnyNamesInPipelineLayout(*pipelineLayoutGL))
        {
            shaderBindingLayout_ = GLStatePool::Instance().CreateShaderBindingLayout(*pipelineLayoutGL);
            if (!shaderBindingLayout_->HasBindings())
                GLStatePool::Instance().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
        }
    }

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

    /* Create depth-stencil state */
    depthStencilState_ = GLStatePool::Instance().CreateDepthStencilState(desc.depth, desc.stencil);

    /* Create rasterizer state */
    rasterizerState_ = GLStatePool::Instance().CreateRasterizerState(desc.rasterizer);

    /* Create blend state */
    if (auto renderPass = desc.renderPass)
    {
        auto renderPassGL = LLGL_CAST(const GLRenderPass*, renderPass);
        blendState_ = GLStatePool::Instance().CreateBlendState(desc.blend, renderPassGL->GetNumColorAttachments());
    }
    else
        blendState_ = GLStatePool::Instance().CreateBlendState(desc.blend, 1);

    /* Build static state buffer for viewports and scissors */
    if (!desc.viewports.empty() || !desc.scissors.empty())
        BuildStaticStateBuffer(desc);
}

GLGraphicsPipeline::~GLGraphicsPipeline()
{
    GLStatePool::Instance().ReleaseDepthStencilState(std::move(depthStencilState_));
    GLStatePool::Instance().ReleaseRasterizerState(std::move(rasterizerState_));
    GLStatePool::Instance().ReleaseBlendState(std::move(blendState_));
    GLStatePool::Instance().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
}

void GLGraphicsPipeline::Bind(GLStateManager& stateMngr)
{
    /* Bind shader program and discard rasterizer if there is no fragment shader */
    stateMngr.BindShaderProgram(shaderProgram_->GetID());

    /* Update resource slots in shader program (if necessary) */
    if (shaderBindingLayout_)
        shaderProgram_->BindResourceSlots(*shaderBindingLayout_);

    /* Set input-assembler state */
    if (patchVertices_ > 0)
        stateMngr.SetPatchVertices(patchVertices_);

    /* Set depth states */
    stateMngr.SetDepthStencilState(depthStencilState_.get());

    /* Set rasterizer state */
    stateMngr.SetRasterizerState(rasterizerState_.get());

    /* Set blend state */
    stateMngr.BindBlendState(blendState_.get());

    /* Set static viewports and scissors */
    if (staticStateBuffer_)
    {
        ByteBufferIterator byteBufferIter { staticStateBuffer_.get() };
        if (numStaticViewports_ > 0)
            SetStaticViewports(stateMngr, byteBufferIter);
        if (numStaticScissors_ > 0)
            SetStaticScissors(stateMngr, byteBufferIter);
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

    ByteBufferIterator byteBufferIter { staticStateBuffer_.get() };

    /* Build static viewports in raw buffer */
    if (!desc.viewports.empty())
        BuildStaticViewports(desc.viewports.size(), desc.viewports.data(), byteBufferIter);

    /* Build static scissors in raw buffer */
    if (!desc.scissors.empty())
        BuildStaticScissors(desc.scissors.size(), desc.scissors.data(), byteBufferIter);
}

void GLGraphicsPipeline::BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter)
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
        auto dst = byteBufferIter.Next<GLViewport>();
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
        auto dst = byteBufferIter.Next<GLDepthRange>();
        {
            dst->minDepth = static_cast<GLdouble>(viewports[i].minDepth);
            dst->maxDepth = static_cast<GLdouble>(viewports[i].maxDepth);
        }
    }
}

void GLGraphicsPipeline::BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter)
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
        auto dst = byteBufferIter.Next<GLScissor>();
        {
            dst->x      = static_cast<GLint>(scissors[i].x);
            dst->y      = static_cast<GLint>(scissors[i].y);
            dst->width  = static_cast<GLsizei>(scissors[i].width);
            dst->height = static_cast<GLsizei>(scissors[i].height);
        }
    }
}

void GLGraphicsPipeline::SetStaticViewports(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter)
{
    /* Copy viewports to intermediate array (must be adjusted when framebuffer is bound) */
    GLViewport intermediateViewports[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
    ::memcpy(
        intermediateViewports,
        byteBufferIter.Next<GLViewport>(numStaticViewports_),
        numStaticViewports_ * sizeof(GLViewport)
    );
    stateMngr.SetViewportArray(0, numStaticViewports_, intermediateViewports);

    /* Set depth ranges */
    stateMngr.SetDepthRangeArray(0, numStaticViewports_, byteBufferIter.Next<GLDepthRange>(numStaticViewports_));
}

void GLGraphicsPipeline::SetStaticScissors(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter)
{
    /* Copy scissors to intermediate array (must be adjusted when framebuffer is bound) */
    GLScissor intermediateScissors[LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS];
    ::memcpy(
        intermediateScissors,
        byteBufferIter.Next<GLScissor>(numStaticScissors_),
        numStaticScissors_ * sizeof(GLScissor)
    );
    stateMngr.SetScissorArray(0, numStaticScissors_, intermediateScissors);
}


} // /namespace LLGL



// ================================================================================
