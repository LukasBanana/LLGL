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
    bool                        isGraphicsPSO,
    const PipelineLayout*       pipelineLayout,
    const ArrayView<Shader*>&   shaders)
:
    isGraphicsPSO_ { isGraphicsPSO }
{
    /* Create shader pipeline */
    shaderPipeline_ = GLStatePool::Get().CreateShaderPipeline(shaders.size(), shaders.data());
    shaderPipeline_->QueryInfoLogs(report_);

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
    GLStatePool::Get().ReleaseShaderPipeline(std::move(shaderPipeline_));
    GLStatePool::Get().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
}

const Report* GLPipelineState::GetReport() const
{
    return (report_.HasErrors() || report_.GetText() != nullptr ? &report_ : nullptr);
}

void GLPipelineState::Bind(GLStateManager& stateMngr)
{
    /* Bind shader program and discard rasterizer if there is no fragment shader */
    shaderPipeline_->Bind(stateMngr);

    /* Update resource slots in shader program (if necessary) */
    if (shaderBindingLayout_)
        shaderPipeline_->BindResourceSlots(*shaderBindingLayout_);
}


} // /namespace LLGL



// ================================================================================
