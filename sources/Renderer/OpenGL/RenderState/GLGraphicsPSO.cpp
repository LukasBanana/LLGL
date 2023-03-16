/*
 * GLGraphicsPSO.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGraphicsPSO.h"
#include "GLRenderPass.h"
#include "GLStatePool.h"
#include "../Ext/GLExtensions.h"
#include "../Shader/GLShaderProgram.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/ByteBufferIterator.h"
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/StaticLimits.h>
#include <LLGL/Misc/ForRange.h>
#include <vector>


namespace LLGL
{


static void AddToShaderArray(Shader* shader, std::vector<Shader*>& shaders)
{
    if (shader != nullptr)
        shaders.push_back(shader);
}

static std::vector<Shader*> GetShaderArrayFromDesc(const GraphicsPipelineDescriptor& desc)
{
    std::vector<Shader*> shaders;
    shaders.reserve(5);

    AddToShaderArray(desc.vertexShader,         shaders);
    AddToShaderArray(desc.tessControlShader,    shaders);
    AddToShaderArray(desc.tessEvaluationShader, shaders);
    AddToShaderArray(desc.geometryShader,       shaders);
    AddToShaderArray(desc.fragmentShader,       shaders);

    return shaders;
}

GLGraphicsPSO::GLGraphicsPSO(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits) :
    GLPipelineState { /*isGraphicsPSO:*/ true, desc.pipelineLayout, GetShaderArrayFromDesc(desc) }
{
    /* Convert input-assembler state */
    drawMode_       = GLTypes::ToDrawMode(desc.primitiveTopology);
    primitiveMode_  = GLTypes::ToPrimitiveMode(desc.primitiveTopology);

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
    depthStencilState_ = GLStatePool::Get().CreateDepthStencilState(desc.depth, desc.stencil);

    /* Create rasterizer state */
    rasterizerState_ = GLStatePool::Get().CreateRasterizerState(desc.rasterizer);

    /* Create blend state */
    if (auto renderPass = desc.renderPass)
    {
        auto renderPassGL = LLGL_CAST(const GLRenderPass*, renderPass);
        blendState_ = GLStatePool::Get().CreateBlendState(desc.blend, renderPassGL->GetNumColorAttachments());
    }
    else
        blendState_ = GLStatePool::Get().CreateBlendState(desc.blend, 1);

    /* Build static state buffer for viewports and scissors */
    if (!desc.viewports.empty() || !desc.scissors.empty())
        BuildStaticStateBuffer(desc);
}

GLGraphicsPSO::~GLGraphicsPSO()
{
    GLStatePool::Get().ReleaseDepthStencilState(std::move(depthStencilState_));
    GLStatePool::Get().ReleaseRasterizerState(std::move(rasterizerState_));
    GLStatePool::Get().ReleaseBlendState(std::move(blendState_));
}

void GLGraphicsPSO::Bind(GLStateManager& stateMngr)
{
    /* Bind shader program and binding layout from base class */
    GLPipelineState::Bind(stateMngr);

    /* Set input-assembler state */
    if (patchVertices_ > 0)
        stateMngr.SetPatchVertices(patchVertices_);

    /* Bind depth-stencil, rasterizer, and blend states */
    stateMngr.BindDepthStencilState(depthStencilState_.get());
    stateMngr.BindRasterizerState(rasterizerState_.get());
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

void GLGraphicsPSO::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
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

void GLGraphicsPSO::BuildStaticViewports(std::size_t numViewports, const Viewport* viewports, ByteBufferIterator& byteBufferIter)
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
    for_range(i, numViewports)
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
    for_range(i, numViewports)
    {
        auto dst = byteBufferIter.Next<GLDepthRange>();
        {
            dst->minDepth = static_cast<GLclamp_t>(viewports[i].minDepth);
            dst->maxDepth = static_cast<GLclamp_t>(viewports[i].maxDepth);
        }
    }
}

void GLGraphicsPSO::BuildStaticScissors(std::size_t numScissors, const Scissor* scissors, ByteBufferIterator& byteBufferIter)
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
    for_range(i, numScissors)
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

void GLGraphicsPSO::SetStaticViewports(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter)
{
    stateMngr.SetViewportArray(0, numStaticViewports_, byteBufferIter.Next<GLViewport>(numStaticViewports_));
    stateMngr.SetDepthRangeArray(0, numStaticViewports_, byteBufferIter.Next<GLDepthRange>(numStaticViewports_));
}

void GLGraphicsPSO::SetStaticScissors(GLStateManager& stateMngr, ByteBufferIterator& byteBufferIter)
{
    stateMngr.SetScissorArray(0, numStaticScissors_, byteBufferIter.Next<GLScissor>(numStaticScissors_));
}


} // /namespace LLGL



// ================================================================================
