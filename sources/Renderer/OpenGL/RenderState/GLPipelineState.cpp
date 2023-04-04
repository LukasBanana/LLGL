/*
 * GLPipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLPipelineState.h"
#include "GLStatePool.h"
#include "GLStateManager.h"
#include "../GLTypes.h"
#include "../Shader/GLShaderProgram.h"
#include "../Ext/GLExtensions.h"
#include "../../CheckedCast.h"
#include <LLGL/Misc/ForRange.h>


namespace LLGL
{


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
        pipelineLayout_ = LLGL_CAST(const GLPipelineLayout*, pipelineLayout);
        if (pipelineLayout_->HasNamedBindings())
        {
            shaderBindingLayout_ = GLStatePool::Get().CreateShaderBindingLayout(*pipelineLayout_);
            if (!shaderBindingLayout_->HasBindings())
                GLStatePool::Get().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
        }

        /* Build uniform table */
        BuildUniformMap(pipelineLayout_->GetUniforms());
    }
}

GLPipelineState::~GLPipelineState()
{
    GLStatePool::Get().ReleaseShaderPipeline(std::move(shaderPipeline_));
    GLStatePool::Get().ReleaseShaderBindingLayout(std::move(shaderBindingLayout_));
}

const Report* GLPipelineState::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

void GLPipelineState::Bind(GLStateManager& stateMngr)
{
    /* Bind shader program and discard rasterizer if there is no fragment shader */
    shaderPipeline_->Bind(stateMngr);

    /* Update resource slots in shader program (if necessary) */
    if (shaderBindingLayout_)
        shaderPipeline_->BindResourceSlots(*shaderBindingLayout_);

    /* Bind static samplers */
    if (pipelineLayout_ != nullptr)
        pipelineLayout_->BindStaticSamplers(stateMngr);
}


/*
 * ======= Private: =======
 */

//TODO: support separate shaders; each separable shader needs its own set of uniform locations
void GLPipelineState::BuildUniformMap(const std::vector<UniformDescriptor>& uniforms)
{
    if (!uniforms.empty())
    {
        const GLuint program = shaderPipeline_->GetID();

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
        case GL_DOUBLE:             return 1*2;
        case GL_DOUBLE_VEC2:        return 2*2;
        case GL_DOUBLE_VEC3:        return 3*2;
        case GL_DOUBLE_VEC4:        return 4*2;
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
        case GL_DOUBLE_MAT2:        return 2*2*2;
        case GL_DOUBLE_MAT2x3:      return 2*3*2;
        case GL_DOUBLE_MAT2x4:      return 2*4*2;
        case GL_DOUBLE_MAT3x2:      return 3*2*2;
        case GL_DOUBLE_MAT3:        return 3*3*2;
        case GL_DOUBLE_MAT3x4:      return 3*4*2;
        case GL_DOUBLE_MAT4x2:      return 4*2*2;
        case GL_DOUBLE_MAT4x3:      return 4*3*2;
        case GL_DOUBLE_MAT4:        return 4*4*2;

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
