/*
 * GLProgramPipeline.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLProgramPipeline.h"
#include "GLShaderBindingLayout.h"
#include "GLSeparableShader.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/Exception.h"
#include <LLGL/Report.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


#if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

static GLuint GLCreateProgramPipeline()
{
    GLuint id = 0;
    #if LLGL_GLEXT_DIRECT_STATE_ACCESS
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glCreateProgramPipelines(1, &id);
    }
    else
    #endif // /LLGL_GLEXT_DIRECT_STATE_ACCESS
    {
        /* Generate new program pipeline and initialize to its default state via glBindProgramPipeline */
        glGenProgramPipelines(1, &id);
        GLStateManager::Get().BindProgramPipeline(id);
    }
    return id;
}

GLProgramPipeline::GLProgramPipeline(
    std::size_t             numShaders,
    Shader* const*          shaders,
    GLShader::Permutation   permutation)
:
    GLShaderPipeline { GLCreateProgramPipeline() }
{
    UseProgramStages(numShaders, reinterpret_cast<GLSeparableShader* const*>(shaders), permutation);
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
        #if defined(GL_VERSION_4_0) || defined(GL_ES_VERSION_3_2)
        case ShaderType::TessControl:       return GL_TESS_CONTROL_SHADER_BIT;
        case ShaderType::TessEvaluation:    return GL_TESS_EVALUATION_SHADER_BIT;
        #endif
        #if defined(GL_VERSION_3_2) || defined(GL_ES_VERSION_3_2)
        case ShaderType::Geometry:          return GL_GEOMETRY_SHADER_BIT;
        #endif
        case ShaderType::Fragment:          return GL_FRAGMENT_SHADER_BIT;
        #if defined(GL_VERSION_4_3) || defined(GL_ES_VERSION_3_1)
        case ShaderType::Compute:           return GL_COMPUTE_SHADER_BIT;
        #endif
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

void GLProgramPipeline::QueryInfoLogs(Report& report)
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

void GLProgramPipeline::UseProgramStages(
    std::size_t                 numShaders,
    GLSeparableShader* const*   shaders,
    GLShader::Permutation       permutation)
{
    /* Find last shader in pipeline that transforms gl_Position if such permutation is requested */
    const GLShader* shaderWithFlippedYPosition = nullptr;
    if (permutation == GLShader::PermutationFlippedYPosition)
        shaderWithFlippedYPosition = GLPipelineSignature::FindFinalGLPositionShader(numShaders, reinterpret_cast<const Shader* const*>(shaders));

    for_range(i, numShaders)
    {
        GLSeparableShader* separableShader = shaders[i];
        LLGL_ASSERT_PTR(separableShader);
        if (GLbitfield stage = ToGLShaderStageBit(separableShader->GetType()))
        {
            const GLShader::Permutation permutationForShader =
            (
                separableShader == shaderWithFlippedYPosition
                    ? GLShader::PermutationFlippedYPosition
                    : GLShader::PermutationDefault
            );
            glUseProgramStages(GetID(), stage, separableShader->GetID(permutationForShader));
            separableShaders_[i] = separableShader;
        }
    }

    BuildSignature(numShaders, reinterpret_cast<const Shader* const*>(shaders), permutation);
}

#else // LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

GLProgramPipeline::GLProgramPipeline(
    std::size_t             numShaders,
    Shader* const*          shaders,
    GLShader::Permutation   permutation)
:
    GLShaderPipeline { 0 }
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_separate_shader_objects");
}

void GLProgramPipeline::Bind(GLStateManager& stateMngr)
{
    // dummy
}

void GLProgramPipeline::BindResourceSlots(const GLShaderBindingLayout& bindingLayout)
{
    // dummy
}

void GLProgramPipeline::QueryInfoLogs(Report& report)
{
    // dummy
}

#endif // /LLGL_GLEXT_SEPARATE_SHADER_OBJECTS


} // /namespace LLGL



// ================================================================================
