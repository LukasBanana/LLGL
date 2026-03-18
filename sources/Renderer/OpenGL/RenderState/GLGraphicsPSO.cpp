/*
 * GLGraphicsPSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
#include <LLGL/Constants.h>
#include <LLGL/Utils/ForRange.h>
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

GLGraphicsPSO::GLGraphicsPSO(const GraphicsPipelineDescriptor& desc, const RenderingLimits& limits, PipelineCache* pipelineCache) :
    GLPipelineState { /*isGraphicsPSO:*/ true, desc.pipelineLayout, pipelineCache, GetShaderArrayFromDesc(desc) }
{
    /* Convert input-assembler state */
    drawMode_       = GLTypes::ToDrawMode(desc.primitiveTopology);
    primitiveMode_  = GLTypes::ToPrimitiveMode(desc.primitiveTopology);

    if (IsPrimitiveTopologyPatches(desc.primitiveTopology))
    {
        /* Store patch vertices and check limit */
        const std::uint32_t patchSize = GetPrimitiveTopologyPatchSize(desc.primitiveTopology);
        if (patchSize > limits.maxPatchVertices)
        {
            GetMutableReport().Errorf(
                "renderer does not support %u control points for patches (limit is %u)\n",
                patchSize, limits.maxPatchVertices
            );
            patchVertices_ = static_cast<GLint>(limits.maxPatchVertices);
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
    if (const RenderPass* renderPass = desc.renderPass)
    {
        auto* renderPassGL = LLGL_CAST(const GLRenderPass*, renderPass);
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
    SetStaticViewportsAndScissors(stateMngr);
}


/*
 * ======= Private: =======
 */

void GLGraphicsPSO::BuildStaticStateBuffer(const GraphicsPipelineDescriptor& desc)
{
    ByteBufferIterator byteBufferIter = staticStateBuffer_.Allocate(
        desc.viewports.size(), desc.scissors.size(),
        (sizeof(GLViewport) + sizeof(GLDepthRange)), sizeof(GLScissor),
        GetMutableReport()
    );

    /* Build <GLViewport> entries */
    for_range(i, staticStateBuffer_.GetNumViewports())
    {
        auto dst = byteBufferIter.Next<GLViewport>();
        {
            dst->x      = static_cast<GLfloat>(desc.viewports[i].x);
            dst->y      = static_cast<GLfloat>(desc.viewports[i].y);
            dst->width  = static_cast<GLfloat>(desc.viewports[i].width);
            dst->height = static_cast<GLfloat>(desc.viewports[i].height);
        }
    }

    /* Build <GLDepthRange> entries */
    for_range(i, staticStateBuffer_.GetNumViewports())
    {
        auto dst = byteBufferIter.Next<GLDepthRange>();
        {
            dst->minDepth = static_cast<GLclamp_t>(desc.viewports[i].minDepth);
            dst->maxDepth = static_cast<GLclamp_t>(desc.viewports[i].maxDepth);
        }
    }

    /* Build <GLScissor> entries */
    for_range(i, staticStateBuffer_.GetNumScissors())
    {
        auto dst = byteBufferIter.Next<GLScissor>();
        {
            dst->x      = static_cast<GLint>(desc.scissors[i].x);
            dst->y      = static_cast<GLint>(desc.scissors[i].y);
            dst->width  = static_cast<GLsizei>(desc.scissors[i].width);
            dst->height = static_cast<GLsizei>(desc.scissors[i].height);
        }
    }
}

void GLGraphicsPSO::SetStaticViewportsAndScissors(GLStateManager& stateMngr)
{
    if (staticStateBuffer_)
    {
        ByteBufferConstIterator byteBufferIter = staticStateBuffer_.GetBufferIterator();
        if (staticStateBuffer_.GetNumViewports() > 0)
        {
            const GLsizei numViewportsSizei = static_cast<GLsizei>(staticStateBuffer_.GetNumViewports());
            stateMngr.SetViewportArray(0, numViewportsSizei, byteBufferIter.Next<GLViewport>(staticStateBuffer_.GetNumViewports()));
            stateMngr.SetDepthRangeArray(0, numViewportsSizei, byteBufferIter.Next<GLDepthRange>(staticStateBuffer_.GetNumViewports()));
        }
        if (staticStateBuffer_.GetNumScissors() > 0)
        {
            const GLsizei numScissorsSizei = static_cast<GLsizei>(staticStateBuffer_.GetNumScissors());
            stateMngr.SetScissorArray(0, numScissorsSizei, byteBufferIter.Next<GLScissor>(staticStateBuffer_.GetNumScissors()));
        }
    }
}


} // /namespace LLGL



// ================================================================================
