/*
 * GLProgramPipeline.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLProgramPipeline.h"
#include "GLShaderBindingLayout.h"
#include "GLSeparableShader.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../../../Core/BasicReport.h"
#include <LLGL/Misc/ForRange.h>


namespace LLGL
{


GLProgramPipeline::GLProgramPipeline(std::size_t numShaders, Shader* const* shaders)
{
    GLuint id = 0;
    glCreateProgramPipelines(1, &id);
    SetID(id);
    UseProgramStages(numShaders, reinterpret_cast<GLSeparableShader* const*>(shaders));
}

GLProgramPipeline::~GLProgramPipeline()
{
    GLuint id = GetID();
    glDeleteProgramPipelines(1, &id);
    GLStateManager::Get().NotifyProgramPipelineRelease(this);
}

static GLbitfield ToGLShaderStageBit(ShaderType type)
{
    switch (type)
    {
        case ShaderType::Vertex:            return GL_VERTEX_SHADER_BIT;
        case ShaderType::TessControl:       return GL_TESS_CONTROL_SHADER_BIT;
        case ShaderType::TessEvaluation:    return GL_TESS_EVALUATION_SHADER_BIT;
        case ShaderType::Geometry:          return GL_GEOMETRY_SHADER_BIT;
        case ShaderType::Fragment:          return GL_FRAGMENT_SHADER_BIT;
        case ShaderType::Compute:           return GL_COMPUTE_SHADER_BIT;
        default:                            return 0;
    }
}

void GLProgramPipeline::Bind(GLStateManager& stateMngr)
{
    stateMngr.BindProgramPipeline(GetID());
}

void GLProgramPipeline::BindResourceSlots(const GLShaderBindingLayout& bindingLayout)
{
    for_range(i, static_cast<std::size_t>(GetSignature().GetNumShaders()))
        separableShaders_[i]->BindResourceSlots(bindingLayout);
}

void GLProgramPipeline::QueryInfoLogs(BasicReport& report)
{
    bool hasErrors = false;
    std::string log;

    for_range(i, GetSignature().GetNumShaders())
        separableShaders_[i]->QueryInfoLog(log, hasErrors);

    report.Reset(std::move(log), hasErrors);
}


/*
 * ======= Private: =======
 */

void GLProgramPipeline::UseProgramStages(std::size_t numShaders, GLSeparableShader* const* shaders)
{
    for (std::size_t i = 0; i < numShaders; ++i)
    {
        auto separableShader = shaders[i];
        if (auto stage = ToGLShaderStageBit(separableShader->GetType()))
        {
            glUseProgramStages(GetID(), stage, separableShader->GetID());
            separableShaders_[i] = separableShader;
        }
    }
    BuildSignature(numShaders, reinterpret_cast<const Shader* const*>(shaders));
}


} // /namespace LLGL



// ================================================================================
