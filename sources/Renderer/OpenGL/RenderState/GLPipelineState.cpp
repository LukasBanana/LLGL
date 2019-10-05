/*
 * GLPipelineState.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLPipelineState.h"
#include "GLStatePool.h"
#include "GLStateManager.h"
#include "../Shader/GLShaderProgram.h"
#include "../../CheckedCast.h"


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

GLPipelineState::GLPipelineState(
    bool                    isGraphicsPSO,
    const PipelineLayout*   pipelineLayout,
    const ShaderProgram*    shaderProgram)
:
    isGraphicsPSO_ { isGraphicsPSO }
{
    /* Convert shader state */
    shaderProgram_ = LLGL_CAST(const GLShaderProgram*, shaderProgram);
    if (!shaderProgram_)
        throw std::invalid_argument("failed to create pipeline state due to missing shader program");

    /* Create shader binding layout by binding descriptor */
    if (pipelineLayout != nullptr)
    {
        /* Ignore pipeline layout if there are no names specified, because no valid binding layout can be created then */
        auto pipelineLayoutGL = LLGL_CAST(const GLPipelineLayout*, pipelineLayout);
        if (AnyNamesInPipelineLayout(*pipelineLayoutGL))
        {
            shaderBindingLayout_ = GLStatePool::Get().CreateShaderBindingLayout(*pipelineLayoutGL);
            if (!shaderBindingLayout_->HasBindings())
                GLStatePool::Get().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
        }
    }
}

GLPipelineState::~GLPipelineState()
{
    GLStatePool::Get().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
}

void GLPipelineState::Bind(GLStateManager& stateMngr)
{
    /* Bind shader program and discard rasterizer if there is no fragment shader */
    stateMngr.BindShaderProgram(shaderProgram_->GetID());

    /* Update resource slots in shader program (if necessary) */
    if (shaderBindingLayout_)
        shaderProgram_->BindResourceSlots(*shaderBindingLayout_);
}


} // /namespace LLGL



// ================================================================================
