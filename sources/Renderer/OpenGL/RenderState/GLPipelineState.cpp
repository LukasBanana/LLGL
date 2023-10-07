/*
 * GLPipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLPipelineState.h"
#include "GLStatePool.h"
#include "GLStateManager.h"
#include "GLPipelineCache.h"
#include "../GLTypes.h"
#include "../Shader/GLShaderProgram.h"
#include "../Ext/GLExtensions.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


GLPipelineState::GLPipelineState(
    bool                        isGraphicsPSO,
    const PipelineLayout*       pipelineLayout,
    PipelineCache*              pipelineCache,
    const ArrayView<Shader*>&   shaders)
:
    isGraphicsPSO_ { isGraphicsPSO }
{
    /* Get GL pipeline cache if specified */
    GLPipelineCache* pipelineCacheGL = (pipelineCache != nullptr ? LLGL_CAST(GLPipelineCache*, pipelineCache) : nullptr);

    for_range(permutationIndex, GLShader::PermutationCount)
    {
        const GLShader::Permutation permutation = static_cast<GLShader::Permutation>(permutationIndex);
        if (GLShader::HasAnyShaderPermutation(permutation, shaders))
        {
            /* Create shader pipeline for current permutation */
            shaderPipelines_[permutation] = GLStatePool::Get().CreateShaderPipeline(shaders.size(), shaders.data(), permutation, pipelineCacheGL);

            /* Query information log and stop linking shader pipelines if the default permutation has errors */
            if (permutation == GLShader::PermutationDefault)
            {
                shaderPipelines_[GLShader::PermutationDefault]->QueryInfoLogs(report_);
                if (report_.HasErrors())
                    break;
            }
        }
    }

    /* Create shader binding layout by binding descriptor */
    if (pipelineLayout != nullptr)
    {
        /* Ignore pipeline layout if there are no names specified, because no valid binding layout can be created then */
        pipelineLayout_ = LLGL_CAST(const GLPipelineLayout*, pipelineLayout);
        if (pipelineLayout_->HasNamedBindings())
        {
            shaderBindingLayout_ = GLStatePool::Get().CreateShaderBindingLayout(*pipelineLayout_);
            if (!shaderBindingLayout_->HasBindings())
                GLStatePool::Get().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
        }

        /* Build uniform table */
        for_range(permutationIndex, GLShader::PermutationCount)
        {
            const GLShader::Permutation permutation = static_cast<GLShader::Permutation>(permutationIndex);
            BuildUniformMap(permutation, pipelineLayout_->GetUniforms());
        }
    }
}

GLPipelineState::~GLPipelineState()
{
    for (GLShaderPipelineSPtr& shaderPipeline : shaderPipelines_)
        GLStatePool::Get().ReleaseShaderPipeline(std::move(shaderPipeline));
    GLStatePool::Get().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
}

const Report* GLPipelineState::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

void GLPipelineState::Bind(GLStateManager& stateMngr)
{
    /* Select shader pipeline permutation depending on what is needed for the current framebuffer */
    const GLShader::Permutation shaderPipelinePermutation =
    (
        stateMngr.GetBoundRenderTarget() != nullptr && shaderPipelines_[GLShader::PermutationFlippedYPosition].get() != nullptr
            ? GLShader::PermutationFlippedYPosition
            : GLShader::PermutationDefault
    );
    GLShaderPipeline* shaderPipeline = shaderPipelines_[shaderPipelinePermutation].get();

    /* Bind shader program and discard rasterizer if there is no fragment shader */
    LLGL_ASSERT(shaderPipeline != nullptr, "GL shader permutation [%d] not compiled", static_cast<int>(shaderPipelinePermutation));
    shaderPipeline->Bind(stateMngr);

    /* Update resource slots in shader program (if necessary) */
    if (shaderBindingLayout_)
        shaderPipeline->BindResourceSlots(*shaderBindingLayout_);

    /* Bind static samplers */
    if (pipelineLayout_ != nullptr)
        pipelineLayout_->BindStaticSamplers(stateMngr);
}


/*
 * ======= Private: =======
 */

//TODO: support separate shaders; each separable shader needs its own set of uniform locations
void GLPipelineState::BuildUniformMap(GLShader::Permutation permutation, const std::vector<UniformDescriptor>& uniforms)
{
    if (shaderPipelines_[permutation].get() != nullptr && !uniforms.empty())
    {
        const GLuint program = shaderPipelines_[permutation]->GetID();

        /* Build uniform locations from input descriptors */
        uniformMap_.resize(uniforms.size());
        for_range(i, uniforms.size())
            BuildUniformLocation(program, uniformMap_[i], uniforms[i]);
    }
}

// Returns the size (in 32-bit words) for the specified GL uniform type
static GLuint GetUniformWordSize(GLenum type)
{
    switch (type)
    {
        /* ----- Scalars/Vectors ----- */
        case GL_FLOAT:              return 1;
        case GL_FLOAT_VEC2:         return 2;
        case GL_FLOAT_VEC3:         return 3;
        case GL_FLOAT_VEC4:         return 4;
        #ifdef LLGL_OPENGL
        case GL_DOUBLE:             return 1*2;
        case GL_DOUBLE_VEC2:        return 2*2;
        case GL_DOUBLE_VEC3:        return 3*2;
        case GL_DOUBLE_VEC4:        return 4*2;
        #endif // /LLGL_OPENGL
        case GL_INT:                return 1;
        case GL_INT_VEC2:           return 2;
        case GL_INT_VEC3:           return 3;
        case GL_INT_VEC4:           return 4;
        case GL_UNSIGNED_INT:       return 1;
        case GL_UNSIGNED_INT_VEC2:  return 2;
        case GL_UNSIGNED_INT_VEC3:  return 3;
        case GL_UNSIGNED_INT_VEC4:  return 4;
        case GL_BOOL:               return 1;
        case GL_BOOL_VEC2:          return 2;
        case GL_BOOL_VEC3:          return 3;
        case GL_BOOL_VEC4:          return 4;

        /* ----- Matrices ----- */
        case GL_FLOAT_MAT2:         return 2*2;
        case GL_FLOAT_MAT2x3:       return 2*3;
        case GL_FLOAT_MAT2x4:       return 2*4;
        case GL_FLOAT_MAT3x2:       return 3*2;
        case GL_FLOAT_MAT3:         return 3*3;
        case GL_FLOAT_MAT3x4:       return 3*4;
        case GL_FLOAT_MAT4x2:       return 4*2;
        case GL_FLOAT_MAT4x3:       return 4*3;
        case GL_FLOAT_MAT4:         return 4*4;
        #ifdef LLGL_OPENGL
        case GL_DOUBLE_MAT2:        return 2*2*2;
        case GL_DOUBLE_MAT2x3:      return 2*3*2;
        case GL_DOUBLE_MAT2x4:      return 2*4*2;
        case GL_DOUBLE_MAT3x2:      return 3*2*2;
        case GL_DOUBLE_MAT3:        return 3*3*2;
        case GL_DOUBLE_MAT3x4:      return 3*4*2;
        case GL_DOUBLE_MAT4x2:      return 4*2*2;
        case GL_DOUBLE_MAT4x3:      return 4*3*2;
        case GL_DOUBLE_MAT4:        return 4*4*2;
        #endif // /LLGL_OPENGL

        default:                    return 0;
    }
}

void GLPipelineState::BuildUniformLocation(GLuint program, GLUniformLocation& outUniform, const UniformDescriptor& inUniform)
{
    /* Find uniform location by name in shader pipeline */
    GLint location = glGetUniformLocation(program, inUniform.name.c_str());
    if (location == -1)
    {
        /* Write invalid uniform location */
        outUniform.type     = UniformType::Undefined;
        outUniform.location = -1;
        outUniform.count    = 0;
        outUniform.wordSize = 0;
    }
    else
    {
        /* Determine type of uniform */
        GLenum type = 0;
        GLint size = 0;
        glGetActiveUniform(program, static_cast<GLuint>(location), 0, nullptr, &size, &type, nullptr);

        /* Write output uniform */
        outUniform.type     = GLTypes::UnmapUniformType(type);
        outUniform.location = location;
        outUniform.count    = size;
        outUniform.wordSize = GetUniformWordSize(type);
    }
}


} // /namespace LLGL



// ================================================================================
